#ifndef HEADER_HPP_
#define HEADER_HPP_

/* clang-format off */
enum { widthI = 400, heightI = 200 };
/* clang-format on */

// Includes
#include "mnist.hpp"
#include <vnepogodin/nn.hpp>

// Constants
static constexpr std::uint8_t red = 235, green = 92, blue = 83;
static constexpr std::uint32_t nn_inputs_size  = 784;
static constexpr std::uint32_t nn_hidden_size  = 64;
static constexpr std::uint32_t nn_outputs_size = 10;
static constexpr std::uint32_t refresh_rate    = 500;

// Variables
static NeuralNetwork nn(nn_inputs_size, nn_hidden_size, nn_outputs_size);
static auto mnist = load_mnist();
static std::string percent_element{"0"};
static std::string user_guess_element{"_"};
static bool user_has_drawing = false;
static bool running          = true;
static size_t train_index    = 0;
static std::mutex m{};

// testing variables
static size_t test_index    = 0;
static size_t total_tests   = 0;
static size_t total_correct = 0;

#endif  // HEADER_HPP_
