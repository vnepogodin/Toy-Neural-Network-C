#include <doctest_compatibility.h>
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

static constexpr std::string_view json = R"({"activation_function":1,"bias_h":{"rows":3,"columns":1,"data":[[-0.310796],[0.164406],[0.645323]]},"bias_o":{"rows":5,"columns":1,"data":[[0.912944],[0.197155],[-0.407593],[-0.380658],[-0.907513]]},"input_nodes":1,"hidden_nodes":3,"output_nodes":5,"weights_ih":{"rows":3,"columns":1,"data":[[0.765160],[-0.830610],[-0.554092]]},"weights_ho":{"rows":5,"columns":3,"data":[[0.804161,-0.485224,0.429943],[-0.475699,0.720266,-0.001433],[0.024648,0.841307,-0.784664],[-0.162800,0.275092,0.379352],[-0.567448,0.457605,-0.802463]]},"learning_rate":0.100000})";
static constexpr std::string_view vnjson[3]{
    R"({"activation_function":1,"bias_h":{"rows":4,"columns":1,"data":[[-0.633466],[0.710922],[0.611780],[0.599963]]},"bias_o":{"rows":1,"columns":1,"data":[[-0.518364]]},"input_nodes":2,"hidden_nodes":4,"output_nodes":1,"weights_ih":{"rows":4,"columns":2,"data":[[0.053919,0.314078],[0.684797,0.434933],[0.180186,0.034002],[-0.788184,0.551531]]},"weights_ho":{"rows":1,"columns":4,"data":[[-0.385283,0.221780,0.280472,-0.354132]]},"learning_rate":0.100000})",
    R"({"activation_function":1,"bias_h":{"rows":10,"columns":1,"data":[[-0.870251],[-0.560540],[0.960077],[0.835917],[0.578417],[-0.245731],[-0.827871],[0.046571],[0.814753],[-0.617377]]},"bias_o":{"rows":5,"columns":1,"data":[[0.282315],[-0.351763],[-0.672947],[-0.352585],[-0.273751]]},"input_nodes":2,"hidden_nodes":10,"output_nodes":5,"weights_ih":{"rows":10,"columns":2,"data":[[-0.853075,-0.187818],[-0.379609,-0.811567],[0.292244,-0.684672],[0.302438,0.859551],[0.108658,-0.369384],[0.370163,0.456599],[-0.245740,-0.843004],[0.445716,-0.129117],[-0.014729,0.800336],[-0.407517,-0.594954]]},"weights_ho":{"rows":5,"columns":10,"data":[[-0.607157,0.026271,-0.983965,-0.296402,-0.562994,0.630421,0.678352,0.327169,-0.643785,0.432182],[0.680215,-0.820070,-0.516226,0.689907,0.877224,-0.848790,-0.603512,0.135856,-0.919225,0.040454],[-0.251212,-0.248257,-0.193209,0.442758,-0.409590,0.317235,-0.191149,-0.077575,0.172540,-0.118574],[-0.805667,0.063235,0.632182,-0.855268,0.717966,-0.410448,0.995602,0.955374,0.541507,-0.087192],[0.349822,0.187802,0.562297,0.048902,0.098141,-0.669983,-0.583850,-0.005576,-0.885899,0.160088]]},"learning_rate":0.100000})",
    R"({"activation_function":1,"bias_h":{"rows":3,"columns":1,"data":[[-0.310796],[0.164406],[0.645323]]},"bias_o":{"rows":5,"columns":1,"data":[[0.912944],[0.197155],[-0.407593],[-0.380658],[-0.907513]]},"input_nodes":1,"hidden_nodes":3,"output_nodes":5,"weights_ih":{"rows":3,"columns":1,"data":[[0.765160],[-0.830610],[-0.554092]]},"weights_ho":{"rows":5,"columns":3,"data":[[0.804161,-0.485224,0.429943],[-0.475699,0.720266,-0.001433],[0.024648,0.841307,-0.784664],[-0.162800,0.275092,0.379352],[-0.567448,0.457605,-0.802463]]},"learning_rate":0.100000})"};

static tnn_really_inline auto isEqual(const std::pmr::vector<NeuralNetwork>& vnn) -> bool {
    bool res{1};
    for (const auto& nn : vnn) {
        const auto& dump = nn.dumps();
        if ((vnjson[0] == dump) ||
            (vnjson[1] == dump) ||
            (vnjson[2] == dump))
            { continue; }
        else { res = 0; break; }
    }
    return res;
}

TEST_CASE("deserialize")
{
    SECTION("parsing from memory")
    {
        SECTION("only 1 json_doc")
        {
            const auto& nn = NeuralNetwork::parse(json);
            CHECK(json == nn.dumps());
        }
        SECTION("many json_docs (njson)")
        {
            constexpr std::string_view njson = R"(
    {"activation_function":1,"bias_h":{"rows":4,"columns":1,"data":[[-0.633466],[0.710922],[0.611780],[0.599963]]},"bias_o":{"rows":1,"columns":1,"data":[[-0.518364]]},"input_nodes":2,"hidden_nodes":4,"output_nodes":1,"weights_ih":{"rows":4,"columns":2,"data":[[0.053919,0.314078],[0.684797,0.434933],[0.180186,0.034002],[-0.788184,0.551531]]},"weights_ho":{"rows":1,"columns":4,"data":[[-0.385283,0.221780,0.280472,-0.354132]]},"learning_rate":0.100000}
    {"activation_function":1,"bias_h":{"rows":10,"columns":1,"data":[[-0.870251],[-0.560540],[0.960077],[0.835917],[0.578417],[-0.245731],[-0.827871],[0.046571],[0.814753],[-0.617377]]},"bias_o":{"rows":5,"columns":1,"data":[[0.282315],[-0.351763],[-0.672947],[-0.352585],[-0.273751]]},"input_nodes":2,"hidden_nodes":10,"output_nodes":5,"weights_ih":{"rows":10,"columns":2,"data":[[-0.853075,-0.187818],[-0.379609,-0.811567],[0.292244,-0.684672],[0.302438,0.859551],[0.108658,-0.369384],[0.370163,0.456599],[-0.245740,-0.843004],[0.445716,-0.129117],[-0.014729,0.800336],[-0.407517,-0.594954]]},"weights_ho":{"rows":5,"columns":10,"data":[[-0.607157,0.026271,-0.983965,-0.296402,-0.562994,0.630421,0.678352,0.327169,-0.643785,0.432182],[0.680215,-0.820070,-0.516226,0.689907,0.877224,-0.848790,-0.603512,0.135856,-0.919225,0.040454],[-0.251212,-0.248257,-0.193209,0.442758,-0.409590,0.317235,-0.191149,-0.077575,0.172540,-0.118574],[-0.805667,0.063235,0.632182,-0.855268,0.717966,-0.410448,0.995602,0.955374,0.541507,-0.087192],[0.349822,0.187802,0.562297,0.048902,0.098141,-0.669983,-0.583850,-0.005576,-0.885899,0.160088]]},"learning_rate":0.100000}
    {"activation_function":1,"bias_h":{"rows":3,"columns":1,"data":[[-0.310796],[0.164406],[0.645323]]},"bias_o":{"rows":5,"columns":1,"data":[[0.912944],[0.197155],[-0.407593],[-0.380658],[-0.907513]]},"input_nodes":1,"hidden_nodes":3,"output_nodes":5,"weights_ih":{"rows":3,"columns":1,"data":[[0.765160],[-0.830610],[-0.554092]]},"weights_ho":{"rows":5,"columns":3,"data":[[0.804161,-0.485224,0.429943],[-0.475699,0.720266,-0.001433],[0.024648,0.841307,-0.784664],[-0.162800,0.275092,0.379352],[-0.567448,0.457605,-0.802463]]},"learning_rate":0.100000}
            )";
            const auto& vnn = NeuralNetwork::parse_many(njson);
            CHECK(isEqual(vnn));
        }
    }

    SECTION("parsing from storage")
    {
        SECTION("only 1 json_doc")
        {
            const auto& nn = NeuralNetwork::load(NN_TEST_DIR "test-load.json");
            CHECK(json == nn.dumps());
        }
        SECTION("many json_docs (njson)")
        {
            const auto& vnn = NeuralNetwork::load_many(NN_TEST_DIR "test-load_many.json");
            CHECK(isEqual(vnn));
        }
    }
}
