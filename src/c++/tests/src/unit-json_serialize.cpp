#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

TEST_CASE("serialize")
{
    SECTION("basic behavior")
    {
        const NeuralNetwork nn(2, 4, 1);
        const auto& first = nn.dumps();

        const auto& copy = NeuralNetwork::parse(first);
        const auto& second = copy.dumps();
        CHECK(first == second);
    }
}
