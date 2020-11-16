#include "../../../include/c++/nn.hpp"  // class NeuralNetwork

using namespace simdjson::dom;

auto main() -> int32_t {
    const auto& _str = "{\"activation_function\":1,\"bias_h\":{\"columns\":1,\"data\":[[0.9767036437988281],[0.035740338265895844],[0.6651158332824707],[0.18232037127017975]],\"rows\":4},\"bias_o\":{\"columns\":1,\"data\":[[0.2508050501346588]],\"rows\":1},\"hidden_nodes\":4,\"input_nodes\":2,\"learning_rate\":0.10000000149011612,\"output_nodes\":1,\"weights_ho\":{\"columns\":4,\"data\":[[0.16082069277763367,0.4601464867591858,0.26322486996650696,0.615668535232544]],\"rows\":1},\"weights_ih\":{\"columns\":2,\"data\":[[0.22036688029766083,0.35037320852279663],[0.8787314295768738,0.3239816725254059],[0.7053817510604858,0.045548245310783386],[0.6740343570709229,0.49260538816452026]],\"rows\":4}}"_padded;
    parser p;
    object obj = p.parse(_str);

    const auto nn = NeuralNetwork::deserialize(obj);
}
