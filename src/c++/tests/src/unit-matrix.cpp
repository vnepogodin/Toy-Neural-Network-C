#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class Matrix

using vnepogodin::Matrix;

/* clang-format off */
TEST_CASE("matrix")
{
    SECTION("add scalar to matrix")
    {
        auto m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}};
        m += 1;
        const auto& test = Matrix{
            {2.0, 3.0, 4.0},
            {5.0, 6.0, 7.0},
            {8.0, 9.0, 10.0}};
        CHECK(m == test);
    }

    SECTION("add matrix to other matrix")
    {
        auto m = Matrix{
            {1.0, 2.0},
            {3.0, 4.0}};
        const auto& n = Matrix{
            {10.0, 11.0},
            {12.0, 13.0}};

        m += n;
        const auto& test = Matrix{
            {11.0, 13.0},
            {15.0, 17.0}};
        CHECK(m == test);
    }

    SECTION("subtract matrix from other matrix")
    {
        const auto& m = Matrix{
            {10.0, 11.0},
            {12.0, 13.0}};

        const auto& n = Matrix{
            {1.0, 2.0},
            {3.0, 4.0}};

        const auto& mMinusN = Matrix::subtract(m, n);
        const auto& test    = Matrix{
            {9.0, 9.0},
            {9.0, 9.0}};
        CHECK(mMinusN == test);
    }

    SECTION("matrix product")
    {
        const auto& m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0}};

        const auto& n = Matrix{
            {7.0, 8.0},
            {9.0, 10.0},
            {11.0, 12.0}};

        const auto& mn   = Matrix::multiply(m, n);
        const auto& test = Matrix{
            {58.0, 64.0},
            {139.0, 154.0}};
        CHECK(mn == test);
    }

    SECTION("hadamard product")
    {
        auto m = Matrix{
            {1.0, 2.0},
            {3.0, 4.0},
            {5.0, 6.0}};

        const auto& n = Matrix{
            {7.0, 8.0},
            {9.0, 10.0},
            {11.0, 12.0}};

        m *= n;
        const auto& test = Matrix{
            {7.0, 16.0},
            {27.0, 40.0},
            {55.0, 72.0}};
        CHECK(m == test);
    }

    SECTION("scalar product")
    {
        auto m = Matrix{
            {1.0, 2.0},
            {3.0, 4.0},
            {5.0, 6.0}};

        m *= 7;
        const auto& test = Matrix{
            {7.0, 14.0},
            {21.0, 28.0},
            {35.0, 42.0}};
        CHECK(m == test);
    }

    SECTION("transpose matrix - (1, 1)")
    {
        const auto& m    = Matrix{{1.0}};
        const auto& mt   = Matrix::transpose(m);
        const auto& test = Matrix{{1.0}};
        CHECK(mt == test);
    }

    SECTION("transpose matrix - (2, 3) to (3, 2)")
    {
        const auto& m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0}};
        const auto& mt = Matrix::transpose(m);

        const auto& test = Matrix{
            {1.0, 4.0},
            {2.0, 5.0},
            {3.0, 6.0}};
        CHECK(mt == test);
    }

    SECTION("transpose matrix - (3, 2) to (2, 3)")
    {
        const auto& m = Matrix{
            {1.0, 2.0},
            {3.0, 4.0},
            {5.0, 6.0}};
        const auto& mt = Matrix::transpose(m);

        const auto& test = Matrix{
            {1.0, 3.0, 5.0},
            {2.0, 4.0, 6.0}};
        CHECK(mt == test);
    }

    SECTION("transpose matrix - (1, 5) to (5, 1)")
    {
        const auto& m = Matrix{
            {1.0, 2.0, 3.0, 4.0, 5.0}};
        const auto& mt = Matrix::transpose(m);

        const auto& test = Matrix{
            {1.0}, {2.0}, {3.0}, {4.0}, {5.0}};
        CHECK(mt == test);
    }

    SECTION("transpose matrix - (5, 1) to (1, 5)")
    {
        const auto& m = Matrix{
            {1.0}, {2.0}, {3.0}, {4.0}, {5.0}};
        const auto& mt = Matrix::transpose(m);

        const auto& test = Matrix{
            {1.0, 2.0, 3.0, 4.0, 5.0}};
        CHECK(mt == test);
    }

    SECTION("mapping with static map")
    {
        const auto& m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}};
        const auto& mt = Matrix::map(m, [=](double elem) {
            return elem * 10;
        });

        const auto& test = Matrix{
            {10.0, 20.0, 30.0},
            {40.0, 50.0, 60.0},
            {70.0, 80.0, 90.0}};
        CHECK(mt == test);
    }

    SECTION("mapping with instance map")
    {
        auto m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}};
        m.map([=](double elem) {
            return elem * 10;
        });

        const auto& test = Matrix{
            {10.0, 20.0, 30.0},
            {40.0, 50.0, 60.0},
            {70.0, 80.0, 90.0}};
        CHECK(m == test);
    }

    SECTION("error handling of addition when columns and rows of A don\'t match columns and rows of B.")
    {
        // This can be an ofstream as well or any other ostream
        std::stringstream buffer;

        // Save cerr's buffer here
        std::streambuf* sbuf = std::cerr.rdbuf();

        // Redirect cerr to our stringstream buffer or any other ostream
        std::cerr.rdbuf(buffer.rdbuf());

        Matrix m1(1, 2);
        const Matrix m2(3, 4);
        m1 += m2;

        // When done redirect cerr to its old self
        std::cerr.rdbuf(sbuf);

        CHECK(buffer.str() == std::string("Columns and Rows of A must match Columns and Rows of B.\n"));
    }

    SECTION("error handling of static subtraction when columns and rows of A don\'t match columns and rows of B.")
    {
        // This can be an ofstream as well or any other ostream
        std::stringstream buffer;

        // Save cerr's buffer here
        std::streambuf* sbuf = std::cerr.rdbuf();

        // Redirect cerr to our stringstream buffer or any other ostream
        std::cerr.rdbuf(buffer.rdbuf());

        const Matrix m1(1, 2);
        const Matrix m2(3, 4);
        Matrix::subtract(m1, m2);

        // When done redirect cerr to its old self
        std::cerr.rdbuf(sbuf);

        CHECK(buffer.str() == std::string("Columns and Rows of A must match Columns and Rows of B.\n"));
    }

    SECTION("error handling of hadamard product when columns and rows of A don\'t match columns and rows of B.")
    {
        // This can be an ofstream as well or any other ostream
        std::stringstream buffer;

        // Save cerr's buffer here
        std::streambuf* sbuf = std::cerr.rdbuf();

        // Redirect cerr to our stringstream buffer or any other ostream
        std::cerr.rdbuf(buffer.rdbuf());

        Matrix m1(1, 2);
        const Matrix m2(3, 4);
        m1 *= m2;

        // When done redirect cerr to its old self
        std::cerr.rdbuf(sbuf);

        CHECK(buffer.str() == std::string("Columns and Rows of A must match Columns and Rows of B.\n"));
    }

    SECTION("error handling of matrix product when columns of A don\'t match rows of B.")
    {
        // This can be an ofstream as well or any other ostream
        std::stringstream buffer;

        // Save cerr's buffer here
        std::streambuf* sbuf = std::cerr.rdbuf();

        // Redirect cerr to our stringstream buffer or any other ostream
        std::cerr.rdbuf(buffer.rdbuf());

        const Matrix m1(1, 2);
        const Matrix m2(3, 4);
        Matrix::multiply(m1, m2);

        // When done redirect cerr to its old self
        std::cerr.rdbuf(sbuf);

        CHECK(buffer.str() == std::string("Columns of A must match rows of B.\n"));
    }

    SECTION("matrix from array")
    {
        static constexpr std::array<double, 3> array{1, 2, 3};
        const auto& m = Matrix::fromArray(array.data(), array.size());

        const auto& test = Matrix{
            {1.0}, {2.0}, {3.0}};
        CHECK(m == test);
    }

    SECTION("matrix to array")
    {
        const auto& m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}};

        static constexpr int array_size = 9;
        std::unique_ptr<double[]> array(m.toArray());
        const auto& is_equal = (m.size() == array_size) && (std::equal(m.begin(), m.end(), array.get()));
        CHECK(is_equal);
    }

    SECTION("chanining matrix methods")
    {
        auto m = Matrix{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}};

        m.map([=](double elem) {
            return elem - 1;
        });
        m *= 10;
        m += 6;

        // This can be an ofstream as well or any other ostream
        std::stringstream buffer;

        // Save clog's buffer here
        std::streambuf* sbuf = std::clog.rdbuf();

        // Redirect clog to our stringstream buffer or any other ostream
        std::clog.rdbuf(buffer.rdbuf());

        std::clog << m << '\n';

        // When done redirect clog to its old self
        std::clog.rdbuf(sbuf);

        const auto& test = Matrix{
            {6.0, 16.0, 26.0},
            {36.0, 46.0, 56.0},
            {66.0, 76.0, 86.0}};
        CHECK(m == test);
    }

    SECTION("matrix (de)serialization")
    {
        Matrix m(5, 5);
        m.randomize();

        const auto& first = m.dumps();

        const auto& copy = Matrix::parse(first);
        const auto& second = copy.dumps();
        CHECK(first == second);
    }

    SECTION("matrix copy")
    {
        Matrix m(5, 5);
        m.randomize();

        const auto& n = Matrix::copy(m);
        CHECK(m == n);
    }
    /* clang-format on */

    /*

// TODO
test('instance map with row and column params', () => {
  let m = new Matrix(3, 3);
  m.data[0] = [1, 2, 3];
  m.data[1] = [4, 5, 6];
  m.data[2] = [7, 8, 9];

  m.map((e, row, col) => e*100 + row*10 + col);

  expect(m).toEqual({
    rows: 3,
    cols: 3,
    data: [
      [100, 201, 302],
      [410, 511, 612],
      [720, 821, 922]
    ]
  });
});

test('static map with row and column params', () => {
  let m = new Matrix(3, 3);
  m.data[0] = [1, 2, 3];
  m.data[1] = [4, 5, 6];
  m.data[2] = [7, 8, 9];

  let mapped = Matrix.map(m, (e, row, col) => e*100 + row*10 + col);

  expect(mapped).toEqual({
    rows: 3,
    cols: 3,
    data: [
      [100, 201, 302],
      [410, 511, 612],
      [720, 821, 922]
    ]
  });
});

     */
}
