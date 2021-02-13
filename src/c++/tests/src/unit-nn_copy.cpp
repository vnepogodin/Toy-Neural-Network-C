#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

TEST_CASE("copy")
{
    SECTION("basic behavior")
    {
        const NeuralNetwork first(2, 4, 1);
        [[maybe_unused]] const NeuralNetwork second = NeuralNetwork::copy(first);
        //CHECK(first == second);
    }
}
