#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

TEST_CASE("train")
{
    SECTION("basic behavior")
    {
        NeuralNetwork nn(5, 25, 1);

        constexpr double input[5]{1.F, 0.F, 0.F, 0.F, 0.F};
        auto *output = nn.predict(input);

        nn.train(input, output);

        CHECK(output[0] > 0.3);

        delete[] output;
    }
}
