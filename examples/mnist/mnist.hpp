#ifndef MNIST_HPP_
#define MNIST_HPP_

#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

using mnist_array = std::variant<std::vector<std::uint8_t>, std::vector<std::vector<std::uint8_t>>>;

enum class label_data { train_images,
    train_labels,
    test_images,
    test_labels };

auto load_mnist() noexcept -> std::unordered_map<label_data, mnist_array>;

#endif  // MNIST_HPP_
