#include "mnist.hpp"
#include "data_view/data_view.h"

#include <filesystem>
#include <fstream>

namespace fs     = std::filesystem;
using Bytes      = std::vector<std::uint8_t>;
using BytesArray = std::vector<Bytes>;

enum class dataset_types { image,
    label };

static inline auto readFile(const std::string_view& file) noexcept -> Bytes {
    // Open the stream to 'lock' the file.
    std::ifstream f(file.data(), std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto& sz = fs::file_size(file.data());

    // Create a buffer.
    Bytes buffer(sz, 0);

    // Read the whole file into the buffer.
    f.read(reinterpret_cast<char*>(buffer.data()), sz);
    return buffer;
}

static auto createArray(const Bytes& buffer, const uint32_t& headerCount) -> std::vector<std::uint32_t> {
    data_view::ConstDataView headerView(reinterpret_cast<const char*>(buffer.data()), 4 * headerCount);
    std::vector<std::uint32_t> res(headerCount, 0);
    for (int i = 0; i < res.size(); ++i) {
        std::uint32_t byte;
        headerView.Read(4 * i, &byte, false);
        res[i] = byte;
    }

    return res;
}

static auto loadFile(const std::string_view& file) noexcept(false) -> mnist_array {
    const auto& buffer   = readFile(file);
    uint32_t headerCount = 4;
    const auto& headers  = createArray(buffer, headerCount);
    // Get file type from the magic number
    dataset_types type;
    std::uint8_t dataLength;
    if (headers[0] == 2049) {
        type        = dataset_types::label;
        dataLength  = 1;
        headerCount = 2;
    } else if (headers[0] == 2051) {
        type       = dataset_types::image;
        dataLength = headers[2] * headers[3];
    } else {
        throw std::runtime_error("Unknown file type " + std::to_string(headers[0]));
    }

    const auto& data_offset = headerCount * 4;
    Bytes data(buffer.size() - data_offset, 0);
    for (size_t i = data_offset; i < buffer.size(); ++i) {
        data[i] = buffer[i];
    }

    if (type == dataset_types::image) {
        BytesArray dataArr;
        for (int i = 0; i < headers[1]; i++) {
            Bytes temp;
            const auto& first = data.begin() + (dataLength * i);
            const auto& last  = data.begin() + (dataLength * (i + 1));
            temp.assign(first, last);
            dataArr.emplace_back(temp);
        }

        dataArr.shrink_to_fit();
        return dataArr;
    }
    return data;
}

auto load_mnist() noexcept -> std::unordered_map<label_data, mnist_array> {
    static const std::unordered_map<label_data, std::string_view> files = {
        {label_data::train_images, "train-images-idx3-ubyte"},
        {label_data::train_labels, "train-labels-idx1-ubyte"},
        {label_data::test_images, "t10k-images-idx3-ubyte"},
        {label_data::test_labels, "t10k-labels-idx1-ubyte"},
    };

    std::unordered_map<label_data, mnist_array> mnist;
    for (const auto& [label, file] : files) {
        mnist[label] = loadFile(file);
    }

    return mnist;
}
