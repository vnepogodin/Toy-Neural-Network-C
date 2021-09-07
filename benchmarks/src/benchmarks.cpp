#include "benchmark/benchmark.h"

#include <vnepogodin/nn.hpp>

using vnepogodin::Matrix;
using vnepogodin::NeuralNetwork;

static void Init(benchmark::State& state) {
    const int bytes = 1 << state.range(0);
    for (auto _ : state) {
        const Matrix m(state.range(0) / 2, state.range(0));

        benchmark::DoNotOptimize(m.data());
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * long(bytes));
}
BENCHMARK(Init)->DenseRange(13, 26)->ReportAggregatesOnly(1);

static void Copy(benchmark::State& state) {
    const int bytes = 1 << state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        const Matrix m(state.range(0) / 2, state.range(0));
        state.ResumeTiming();

        const Matrix&& copy = Matrix::copy(m);

        benchmark::DoNotOptimize(copy.data());
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * long(bytes));
}
BENCHMARK(Copy)->DenseRange(13, 26)->ReportAggregatesOnly(1);

static void Predict(benchmark::State& state) {
    constexpr double input[2]{1.F, 0.F};

    const int bytes = 1 << state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        const NeuralNetwork nn(state.range(0) / 2, state.range(0), state.range(0) / 4);
        state.ResumeTiming();

        const auto& output = nn.predict(input);

        state.PauseTiming();
        delete[] output;
        state.ResumeTiming();
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * long(bytes));
}
BENCHMARK(Predict)->DenseRange(13, 26)->ReportAggregatesOnly(1);

static void Train(benchmark::State& state) {
    const NeuralNetwork nn(2, 4, 1);
    constexpr double input[2]{1.F, 0.F};
    const auto& output = nn.predict(input);

    for (auto _ : state) {
        state.PauseTiming();
        NeuralNetwork&& copy = NeuralNetwork::copy(nn);
        state.ResumeTiming();

        copy.train(input, output);
    }

    delete[] output;

    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * long(1 << state.range(0)));
}
BENCHMARK(Train)->Range(1, 26)->ReportAggregatesOnly(1);

static void Serialize(benchmark::State& state) {
    const NeuralNetwork nn(2, 4, 1);

    for (auto _ : state) {
        const auto& json = nn.dumps();

        benchmark::DoNotOptimize(json);
    }
    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * long(1 << state.range(0)));
}
BENCHMARK(Serialize)->Range(1, 26)->ReportAggregatesOnly(1);

static void Deserialize(benchmark::State& state) {
    const int bytes = 1 << state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        const NeuralNetwork nn(state.range(0) / 2, state.range(0), state.range(0) / 4);
        const auto& first = nn.dumps();
        state.ResumeTiming();

        const auto& copy = NeuralNetwork::parse(first);

        benchmark::DoNotOptimize(copy);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * long(bytes));
}
BENCHMARK(Deserialize)->DenseRange(13, 26)->ReportAggregatesOnly(1);

BENCHMARK_MAIN();
