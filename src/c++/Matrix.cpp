// Matrix lib
#include <vnepogodin/Matrix.hpp>

#include <algorithm>  // std::fill
#include <atomic>     // std::atomic<std::uint32_t>, std::memory_order_release
#include <iostream>   // std::cerr
#include <random>     // std::mt19937, std::uniform_real_distribution, std::random_device

#include <vnepogodin/helper.hpp>                     // for tnn_unlikely
#include <vnepogodin/third_party/json/simdjson.hpp>  // for simdjson_result

#ifdef NN_ENABLE_SIMD
#include <Vc/Vc>
#endif

#ifdef NN_ENABLE_OPENCL
#define CL_HPP_TARGET_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/opencl.hpp>
#endif
#endif

namespace {
/**
 * PTR_START(end):
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end)               \
    std::atomic<std::uint32_t> i(0); \
    while (i < (end))

/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END \
    i.fetch_add(1, std::memory_order_release);

class random_in_range {
 public:
    tnn_really_inline random_in_range()
      : rng(std::random_device()()) { }

    virtual ~random_in_range() = default;

    tnn_really_inline auto get() noexcept -> double {
        std::uniform_real_distribution<double> _realDistribution(start, end);

        return _realDistribution(rng);
    }

    // Delete.
    random_in_range(random_in_range&&)      = delete;
    random_in_range(const random_in_range&) = delete;
    auto operator=(random_in_range&&) -> random_in_range& = delete;
    auto operator=(const random_in_range&) -> random_in_range& = delete;

 private:
    std::mt19937 rng;
    static constexpr double start = 0.0;
    static constexpr double end   = 2.0;
};

#ifdef NN_ENABLE_OPENCL
static auto get_platforms() noexcept -> std::vector<cl::Platform> {
    std::vector<cl::Platform> platforms_;
    cl::Platform::get(&platforms_);
    return platforms_;
}

static auto get_context(const std::vector<cl::Platform>& platforms) noexcept -> cl::Context {
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platforms[0])(),
        0};
    cl::Context context_(CL_DEVICE_TYPE_GPU, properties);
    return context_;
}

static const std::vector<cl::Platform> s_platforms = get_platforms();
static const cl::Context s_context                 = get_context(s_platforms);
static const std::vector<cl::Device>& s_devices    = s_context.getInfo<CL_CONTEXT_DEVICES>();
#endif
};  // namespace

namespace vnepogodin {
#ifdef NN_ENABLE_OPENCL
auto Matrix::transpose_cl(const Matrix& m) noexcept(false) -> Matrix {
    cl_int err = CL_SUCCESS;

    cl::CommandQueue command_queue(s_context, s_devices[0], 0, &err);

    // size of memory required to store the matrix
    const size_t& mem_size = m.size() * sizeof(Matrix::value_type);

    // Create memory buffers on the device for each vector
    cl::Buffer matrix_mem_obj(s_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        mem_size, (void*)m.data(), &err);
    cl::Buffer out_mem_obj(s_context, CL_MEM_WRITE_ONLY,
        mem_size, nullptr, &err);

    // Create a program from the kernel source
    static constexpr std::string_view kernel_source = R"(
__kernel void transpose(__global double* odata, __global const double* idata, int width, int height) {
    const int i = get_global_id(0);
    const int j = get_global_id(1);

    if ((i < width) && (j < height)) {
        const unsigned int index_in  = i + width * j;
        const unsigned int index_out = j + height * i;
        odata[index_out]             = idata[index_in];
    }
})";

    cl::Program program_(s_context, kernel_source.data(), false, &err);

    // Build the program
    err = program_.build(s_devices);

    cl::Kernel kernel(program_, "transpose", &err);

    // Set the arguments of the kernel
    kernel.setArg(0, out_mem_obj);
    kernel.setArg(1, matrix_mem_obj);
    kernel.setArg(2, m.columns);
    kernel.setArg(3, m.rows);

    // Execute the OpenCL kernel on the list
    cl::Event event;
    command_queue.enqueueNDRangeKernel(kernel, cl::NullRange,
        cl::NDRange(m.columns, m.rows), cl::NDRange(m.columns), nullptr, &event);

    event.wait();

    // Read the memory buffer on the device to the local variable
    Matrix t(m.columns, m.rows);

    command_queue.enqueueReadBuffer(out_mem_obj, CL_TRUE, 0,
        mem_size, (void*)t.data());

    return t;
}

auto Matrix::multiply_cl(const Matrix& a, const Matrix& b) noexcept(false) -> Matrix {
    Matrix t(a.rows, b.columns);

    cl_int err = CL_SUCCESS;
    cl::CommandQueue command_queue(s_context, s_devices[0], 0, &err);

    // size of memory required to store the matrix
    const size_t& a_size   = a.size() * sizeof(Matrix::value_type);
    const size_t& b_size   = b.size() * sizeof(Matrix::value_type);
    const size_t& mem_size = t.size() * sizeof(Matrix::value_type);

    // Create memory buffers on the device for each vector
    cl::Buffer matrix_a_mem_obj(s_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        a_size, (void*)a.data(), &err);
    cl::Buffer matrix_b_mem_obj(s_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        b_size, (void*)b.data(), &err);
    cl::Buffer out_mem_obj(s_context, CL_MEM_WRITE_ONLY,
        mem_size, nullptr, &err);

    // Create a program from the kernel source
    static constexpr std::string_view kernel_source = R"(
#define AS(i, j, size) As[j + i * size]
#define BS(i, j, size) Bs[j + i * size]

__kernel void
multiply(__global double* C, __global double* A, __global double* B,
         __local double* As, __local double* Bs, const int uiWA, const int uiWB, const int block_size) {
    const int bx = get_group_id(0);
    const int by = get_group_id(1);

    const int tx = get_local_id(0);
    const int ty = get_local_id(1);

    const int aBegin = uiWA * block_size * by;
    const int aEnd   = aBegin + uiWA - 1;
    const int bBegin = block_size * bx;

    int aStep  = block_size;
    int bStep  = block_size * uiWB;
    double Csub = 0.0;

    // Loop over all the sub-matrices of A and B
    // required to compute the block sub-matrix
    for (int a = aBegin, b = bBegin; a <= aEnd; a += aStep, b += bStep) {
        AS(ty, tx, block_size) = A[a + uiWA * ty + tx];
        BS(ty, tx, block_size) = B[b + uiWB * ty + tx];

        barrier(CLK_LOCAL_MEM_FENCE);

        #pragma unroll
        for (int k = 0; k < block_size; ++k)
            Csub += AS(ty, k, block_size) * BS(k, tx, block_size);

        barrier(CLK_LOCAL_MEM_FENCE);
    }
    C[get_global_id(1) * get_global_size(0) + get_global_id(0)] = Csub;
})";

    cl::Program program_(s_context, kernel_source.data(), false, &err);

    // Build the program
    err = program_.build(s_devices);

    cl::Kernel kernel(program_, "multiply", &err);

    // Set the arguments of the kernel
    kernel.setArg(0, out_mem_obj);
    kernel.setArg(1, matrix_a_mem_obj);
    kernel.setArg(2, matrix_b_mem_obj);
    kernel.setArg(3, mem_size, nullptr);
    kernel.setArg(4, mem_size, nullptr);
    kernel.setArg(5, a.columns);
    kernel.setArg(6, b.columns);
    kernel.setArg(7, a.rows);

    // Multiplication - non-blocking execution:  launch and push to device(s)
    cl::Event GPUExecution;
    command_queue.enqueueNDRangeKernel(kernel, cl::NullRange,
        cl::NDRange(b.columns, t.size()), cl::NDRange(b.columns, b.columns), nullptr, &GPUExecution);
    command_queue.flush();

    // sync all queues to host
    command_queue.finish();

    // Non-blocking copy of result from device to host
    cl::Event GPUDone;
    command_queue.enqueueReadBuffer(out_mem_obj, CL_FALSE, 0,
        mem_size, (void*)t.data(), nullptr, &GPUDone);

    // CPU sync with GPU
    GPUDone.wait();

    return t;
}
#endif

// Operators
auto Matrix::operator+=(const Matrix& mat_a) noexcept -> Matrix& {
    if ((this->rows != mat_a.rows) || (this->columns != mat_a.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : *this) {
        iter += mat_a[i.load(std::memory_order_consume)];
        PTR_END
    }

    return *this;
}

auto Matrix::operator*=(const Matrix& mat) noexcept -> Matrix& {
    if (tnn_unlikely((this->rows != mat.rows) || (this->columns != mat.columns))) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    // hadamard product
    // @see https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
    //
    std::atomic<std::uint32_t> i(0);
    for (auto& iter : *this) {
        iter *= mat[i.load(std::memory_order_consume)];
        PTR_END
    }

    return *this;
}

// Non member operator
auto operator<<(std::ostream& stream, const Matrix& mat) noexcept -> std::ostream& {
    std::atomic<std::uint32_t> counter(0);
    for (const auto& i : mat) {
        stream << i << ' ';

        counter.fetch_add(1, std::memory_order_release);
        if (counter == mat.columns) {
            counter = 0;

            if ((&i + 1) != mat.end())
                stream << "\n";
        }
    }

    return stream;
}

// Functions
auto Matrix::toArray() const noexcept -> pointer {
    auto* tmp = new double[len];

    std::atomic<std::uint32_t> i(0);
    for (const auto& iter : *this) {
        tmp[i.load(std::memory_order_consume)] = iter;
        PTR_END
    }
    return tmp;
}

void Matrix::randomize() noexcept {
    random_in_range r;
    std::generate(begin(), end(), [&]() { return r.get() - 1; });
}

// Serialize to JSON
// TODO: Refactor
//
auto Matrix::dumps() const noexcept -> std::string {
    static constexpr int resLen = 125;
    /* clang-format off */
    std::string _str = std::string("{")
                     + std::string("\"rows\":") + std::to_string(rows)
                     + std::string(",\"columns\":") + std::to_string(columns)
                     + std::string(",\"data\":[");
    /* clang-format on */
    _str.reserve(resLen);

    std::string temp_arr = "[";
    temp_arr.reserve(3 + (this->columns * 11));

    std::uint32_t counter = 0;
    for (const auto& i : *this) {
        temp_arr += std::to_string(i);

        ++counter;
        if (tnn_likely(counter != this->columns)) {
            temp_arr += ',';
        } else {
            counter = 0;
            _str += temp_arr + ']';

            temp_arr = '[';
            if (tnn_likely((&i + 1) != this->end()))
                _str += ',';
        }
    }
    _str += "]}";

    _str.shrink_to_fit();
    return _str;
}

// Static functions
auto Matrix::fromArray(const_pointer arr, const std::uint32_t& len) noexcept -> Matrix {
    Matrix t(len, 1);

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
        iter = arr[i.load(std::memory_order_consume)];
        PTR_END
    }

    return t;
}

// @see https://en.wikipedia.org/wiki/Transpose
auto Matrix::transpose(const Matrix& m) noexcept -> Matrix {
#ifdef NN_ENABLE_OPENCL
    return Matrix::transpose_cl(m);
#else
    Matrix t(m.columns, m.rows);

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows) {
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            /* clang-format off */
            t[counter.load(std::memory_order_consume)] =
                m[j.load(std::memory_order_consume) * t.rows + i.load(std::memory_order_consume)];

            /* clang-format on */
            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
        PTR_END
    }

    return t;
#endif
}

auto Matrix::multiply(const Matrix& a, const Matrix& b) noexcept -> Matrix {
    // Matrix product
    if (tnn_unlikely(a.columns != b.rows)) {
        std::cerr << "Columns of A must match rows of B.\n";
        return Matrix();
    }

#ifdef NN_ENABLE_OPENCL
    return Matrix::multiply_cl(a, b);
#else

    // Dot product of values in column
    Matrix t(a.rows, b.columns);

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows) {
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            std::atomic<std::uint32_t> k(0);
#ifdef NN_ENABLE_SIMD
            Vc::double_v vresult;
#else
            double sum = 0.0;
#endif
            while (k < a.columns) {
#ifdef NN_ENABLE_SIMD
                const Vc::double_v& va = a[i.load(std::memory_order_consume) * a.columns + k.load(std::memory_order_consume)];
                const Vc::double_v& vb = b[k.load(std::memory_order_consume) * b.columns + j.load(std::memory_order_consume)];

                vresult += va * vb;
#else
                /* clang-format off */
                sum += a[i.load(std::memory_order_consume) * a.columns + k.load(std::memory_order_consume)]
                     * b[k.load(std::memory_order_consume) * b.columns + j.load(std::memory_order_consume)];

                /* clang-format on */
#endif

                k.fetch_add(1, std::memory_order_release);
            }
#ifdef NN_ENABLE_SIMD
            t[counter.load(std::memory_order_consume)] = vresult[0];
#else
            t[counter.load(std::memory_order_consume)] = sum;
#endif

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
        PTR_END
    }
    return t;
#endif
}

auto Matrix::subtract(const Matrix& a, const Matrix& b) noexcept -> Matrix {
    if (tnn_unlikely((a.rows != b.rows) || (a.columns != b.columns))) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return Matrix();
    }

    Matrix t(a.rows, b.columns);
    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
#ifdef NN_ENABLE_SIMD
        const Vc::double_v& va = a[i.load(std::memory_order_consume)];
        const Vc::double_v& vb = b[i.load(std::memory_order_consume)];

        const Vc::double_v& vresult = va - vb;
        iter                        = vresult[0];
#else
        /* clang-format off */
        iter = a[i.load(std::memory_order_consume)]
             - b[i.load(std::memory_order_consume)];

        /* clang-format on */
#endif
        PTR_END
    }
    return t;
}

auto Matrix::map(const Matrix& m, const function_t& func) noexcept -> Matrix {
    Matrix t = Matrix::copy(m);

    t.map(func);
    return t;
}

// Deserialize from JSON
//
auto Matrix::parse(const simdjson::dom::object& obj) noexcept -> Matrix {
    const std::uint64_t& rows = obj["rows"];
    const std::uint64_t& cols = obj["columns"];

    Matrix m(rows, cols);
    auto* ptr = m.begin();

    const auto& data = obj["data"];

    std::uint32_t counter = 0;
    PTR_START(m.rows) {
        const auto& buf_s = '/' + std::to_string(i.load(std::memory_order_consume)) + '/' + std::to_string(counter);
        *ptr              = data.at_pointer(buf_s);
        ++ptr;
        counter++;

        if (tnn_unlikely(counter == m.columns)) {
            counter = 0;

            i.fetch_add(1, std::memory_order_release);
        }
    }

    return m;
}
};  // namespace vnepogodin
