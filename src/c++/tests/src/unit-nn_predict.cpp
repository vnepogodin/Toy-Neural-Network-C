#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

TEST_CASE("predict")
{
    SECTION("basic behavior")
    {
        const NeuralNetwork nn(2, 4, 1);

        constexpr double input[2] { 1.F, 0.F };
        auto *output = nn.predict(input);

        const auto res = output[0];
        delete[] output;
        
        CHECK(res > 0.3);
    }
}
