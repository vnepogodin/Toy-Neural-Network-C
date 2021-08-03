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
    const NeuralNetwork nn(2, 4, 1);
    constexpr double input[2]{1.F, 0.F};

    for (auto _ : state) {
        const auto& output = nn.predict(input);

        state.PauseTiming();
        delete[] output;
        state.ResumeTiming();
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * long(1 << state.range(0)));
}
BENCHMARK(Predict)->Range(1, 26)->ReportAggregatesOnly(1);

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
    const auto& _str = "{\"activation_function\":1,\"bias_h\":{\"columns\":1,\"data\":[[0.9767036437988281],[0.035740338265895844],[0.6651158332824707],[0.18232037127017975]],\"rows\":4},\"bias_o\":{\"columns\":1,\"data\":[[0.2508050501346588]],\"rows\":1},\"hidden_nodes\":4,\"input_nodes\":2,\"learning_rate\":0.10000000149011612,\"output_nodes\":1,\"weights_ho\":{\"columns\":4,\"data\":[[0.16082069277763367,0.4601464867591858,0.26322486996650696,0.615668535232544]],\"rows\":1},\"weights_ih\":{\"columns\":2,\"data\":[[0.22036688029766083,0.35037320852279663],[0.8787314295768738,0.3239816725254059],[0.7053817510604858,0.045548245310783386],[0.6740343570709229,0.49260538816452026]],\"rows\":4}}"_padded;

    simdjson::dom::parser p;
    simdjson::dom::element obj = p.parse(_str);

    for (auto _ : state) {
        const auto& nn = NeuralNetwork::parse(obj);

        benchmark::DoNotOptimize(nn);
    }
    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * long(1 << state.range(0)));
}
BENCHMARK(Deserialize)->Range(1, 26)->ReportAggregatesOnly(1);

BENCHMARK_MAIN();
