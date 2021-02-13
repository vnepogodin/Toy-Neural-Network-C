#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

TEST_CASE("constructor 1")
{
    SECTION("basic behavior")
    {
        const std::unique_ptr<NeuralNetwork> nn = std::make_unique<NeuralNetwork>(2, 4, 1);

        CHECK(nn != nullptr);
    }
}
