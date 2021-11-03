#include <stdlib.h> /* free */
#include <string.h> /* strcmp */
#include <unity.h>
#include <vnepogodin/nn.h> /* NeuralNetwork */

static const char* json      = "{\"input_nodes\":1,\"hidden_nodes\":3,\"output_nodes\":5,\"weights_ih\":{\"rows\":3,\"columns\":1,\"data\":[[0.76516002416610718],[-0.83060997724533081],[-0.55409198999404907]]},\"weights_ho\":{\"rows\":5,\"columns\":3,\"data\":[[0.80416101217269897,-0.48522400856018066,0.42994299530982971],[-0.47569900751113892,0.72026598453521729,-0.0014329999685287476],[0.024647999554872513,0.84130698442459106,-0.78466397523880005],[-0.16279999911785126,0.27509200572967529,0.37935200333595276],[-0.56744801998138428,0.45760500431060791,-0.80246299505233765]]},\"bias_h\":{\"rows\":3,\"columns\":1,\"data\":[[-0.31079599261283875],[0.16440600156784058],[0.64532297849655151]]},\"bias_o\":{\"rows\":5,\"columns\":1,\"data\":[[0.91294401884078979],[0.19715499877929688],[-0.4075930118560791],[-0.38065800070762634],[-0.90751302242279053]]},\"learning_rate\":0.10000000149011612,\"activation_function\":1}";
static const char* vnjson[3] = {
    "{\"input_nodes\":2,\"hidden_nodes\":4,\"output_nodes\":1,\"weights_ih\":{\"rows\":4,\"columns\":2,\"data\":[[0.053918998688459396,0.31407800316810608],[0.68479698896408081,0.43493300676345825],[0.18018600344657898,0.034001998603343964],[-0.78818398714065552,0.55153101682662964]]},\"weights_ho\":{\"rows\":1,\"columns\":4,\"data\":[[-0.38528299331665039,0.22178000211715698,0.28047201037406921,-0.35413199663162231]]},\"bias_h\":{\"rows\":4,\"columns\":1,\"data\":[[-0.63346600532531738],[0.7109220027923584],[0.61177998781204224],[0.59996300935745239]]},\"bias_o\":{\"rows\":1,\"columns\":1,\"data\":[[-0.51836401224136353]]},\"learning_rate\":0.10000000149011612,\"activation_function\":1}",
    "{\"input_nodes\":2,\"hidden_nodes\":10,\"output_nodes\":5,\"weights_ih\":{\"rows\":10,\"columns\":2,\"data\":[[-0.85307502746582031,-0.1878180056810379],[-0.37960898876190186,-0.81156700849533081],[0.29224398732185364,-0.68467199802398682],[0.30243799090385437,0.85955101251602173],[0.10865800082683563,-0.36938399076461792],[0.37016299366950989,0.45659899711608887],[-0.24573999643325806,-0.84300398826599121],[0.44571599364280701,-0.12911699712276459],[-0.014728999696671963,0.80033600330352783],[-0.40751698613166809,-0.59495401382446289]]},\"weights_ho\":{\"rows\":5,\"columns\":10,\"data\":[[-0.60715699195861816,0.026271000504493713,-0.98396497964859009,-0.29640200734138489,-0.56299400329589844,0.630420982837677,0.6783519983291626,0.32716900110244751,-0.64378499984741211,0.43218201398849487],[0.68021500110626221,-0.82007002830505371,-0.51622599363327026,0.6899070143699646,0.87722402811050415,-0.84878998994827271,-0.6035119891166687,0.13585600256919861,-0.91922497749328613,0.040454000234603882],[-0.25121200084686279,-0.24825699627399445,-0.19320900738239288,0.4427579939365387,-0.40959000587463379,0.31723499298095703,-0.19114899635314941,-0.077574998140335083,0.17253999412059784,-0.11857400089502335],[-0.80566698312759399,0.063234999775886536,0.63218200206756592,-0.85526800155639648,0.71796602010726929,-0.41044801473617554,0.99560201168060303,0.95537400245666504,0.54150700569152832,-0.087191998958587646],[0.34982201457023621,0.18780200183391571,0.56229698657989502,0.048902001231908798,0.098140999674797058,-0.66998302936553955,-0.58385002613067627,-0.0055760000832378864,-0.88589900732040405,0.16008800268173218]]},\"bias_h\":{\"rows\":10,\"columns\":1,\"data\":[[-0.87025099992752075],[-0.56054002046585083],[0.96007698774337769],[0.83591699600219727],[0.57841700315475464],[-0.24573099613189697],[-0.82787102460861206],[0.046571001410484314],[0.81475299596786499],[-0.61737698316574097]]},\"bias_o\":{\"rows\":5,\"columns\":1,\"data\":[[0.28231498599052429],[-0.35176301002502441],[-0.6729469895362854],[-0.35258498787879944],[-0.27375099062919617]]},\"learning_rate\":0.10000000149011612,\"activation_function\":1}",
    "{\"input_nodes\":1,\"hidden_nodes\":3,\"output_nodes\":5,\"weights_ih\":{\"rows\":3,\"columns\":1,\"data\":[[0.76516002416610718],[-0.83060997724533081],[-0.55409198999404907]]},\"weights_ho\":{\"rows\":5,\"columns\":3,\"data\":[[0.80416101217269897,-0.48522400856018066,0.42994299530982971],[-0.47569900751113892,0.72026598453521729,-0.0014329999685287476],[0.024647999554872513,0.84130698442459106,-0.78466397523880005],[-0.16279999911785126,0.27509200572967529,0.37935200333595276],[-0.56744801998138428,0.45760500431060791,-0.80246299505233765]]},\"bias_h\":{\"rows\":3,\"columns\":1,\"data\":[[-0.31079599261283875],[0.16440600156784058],[0.64532297849655151]]},\"bias_o\":{\"rows\":5,\"columns\":1,\"data\":[[0.91294401884078979],[0.19715499877929688],[-0.4075930118560791],[-0.38065800070762634],[-0.90751302242279053]]},\"learning_rate\":0.10000000149011612,\"activation_function\":1}"};

json_object* object1 = NULL;
json_object* object2 = NULL;

NeuralNetwork* nn   = NULL;
NeuralNetwork* copy = NULL;

NeuralNetwork** vnn = NULL;
int vector_len      = 0;

unsigned char isEqual(const NeuralNetwork** vec, const int size) {
    unsigned char res = 1;
    for (int i = 0; i < size; ++i) {
        json_object* j   = neural_network_serialize(vec[i]);
        const char* dump = json_object_to_json_string_ext(j, 0);
        res              = (strcmp(vnjson[i], dump) == 0);

        if (res == 0) {
            fprintf(stderr, "\nExpected:\n%s\n", vnjson[i]);
            fprintf(stderr, "\nActual:\n%s\n", dump);
            json_object_put(j);
            break;
        }
        json_object_put(j);
    }
    return res;
}

void setUp(void) { }
void tearDown(void) {
    /* clang-format off */
    if (object1) { json_object_put(object1); object1 = NULL; }
    if (object2) { json_object_put(object2); object2 = NULL; }

    if (nn) { neural_network_free(nn); nn = NULL; }
    if (copy) { neural_network_free(copy); copy = NULL; }

    if (vnn) {
        for (int i = 0; i < vector_len; ++i) {
            neural_network_free(vnn[i]);
            vnn[i] = NULL;
        }
        free(vnn);
        vector_len = 0;
        vnn = NULL;
    }
    /* clang-format on */
}

void test_deser_mem_one_jsondoc(void) {
    object1 = json_tokener_parse(json);
    nn      = neural_network_deserialize(object1);

    object2          = neural_network_serialize(nn);
    const char* dump = json_object_to_json_string_ext(object2, 0);

    TEST_ASSERT_EQUAL_STRING(json, dump);
}

void test_deser_file_one_jsondoc(void) {
    nn = neural_network_deserialize_file(NN_TEST_DIR "test-load.json");

    object1          = neural_network_serialize(nn);
    const char* dump = json_object_to_json_string_ext(object1, 0);

    TEST_ASSERT_EQUAL_STRING(json, dump);
}

void test_deser_mem_many_jsondoc(void) {
    static const char* njson = "[{\"activation_function\":1,\"bias_h\":{\"rows\":4,\"columns\":1,\"data\":[[-0.633466],[0.710922],[0.611780],[0.599963]]},\"bias_o\":{\"rows\":1,\"columns\":1,\"data\":[[-0.518364]]},\"input_nodes\":2,\"hidden_nodes\":4,\"output_nodes\":1,\"weights_ih\":{\"rows\":4,\"columns\":2,\"data\":[[0.053919,0.314078],[0.684797,0.434933],[0.180186,0.034002],[-0.788184,0.551531]]},\"weights_ho\":{\"rows\":1,\"columns\":4,\"data\":[[-0.385283,0.221780,0.280472,-0.354132]]},\"learning_rate\":0.100000},\n"
                               "{\"activation_function\":1,\"bias_h\":{\"rows\":10,\"columns\":1,\"data\":[[-0.870251],[-0.560540],[0.960077],[0.835917],[0.578417],[-0.245731],[-0.827871],[0.046571],[0.814753],[-0.617377]]},\"bias_o\":{\"rows\":5,\"columns\":1,\"data\":[[0.282315],[-0.351763],[-0.672947],[-0.352585],[-0.273751]]},\"input_nodes\":2,\"hidden_nodes\":10,\"output_nodes\":5,\"weights_ih\":{\"rows\":10,\"columns\":2,\"data\":[[-0.853075,-0.187818],[-0.379609,-0.811567],[0.292244,-0.684672],[0.302438,0.859551],[0.108658,-0.369384],[0.370163,0.456599],[-0.245740,-0.843004],[0.445716,-0.129117],[-0.014729,0.800336],[-0.407517,-0.594954]]},\"weights_ho\":{\"rows\":5,\"columns\":10,\"data\":[[-0.607157,0.026271,-0.983965,-0.296402,-0.562994,0.630421,0.678352,0.327169,-0.643785,0.432182],[0.680215,-0.820070,-0.516226,0.689907,0.877224,-0.848790,-0.603512,0.135856,-0.919225,0.040454],[-0.251212,-0.248257,-0.193209,0.442758,-0.409590,0.317235,-0.191149,-0.077575,0.172540,-0.118574],[-0.805667,0.063235,0.632182,-0.855268,0.717966,-0.410448,0.995602,0.955374,0.541507,-0.087192],[0.349822,0.187802,0.562297,0.048902,0.098141,-0.669983,-0.583850,-0.005576,-0.885899,0.160088]]},\"learning_rate\":0.100000},\n"
                               "{\"activation_function\":1,\"bias_h\":{\"rows\":3,\"columns\":1,\"data\":[[-0.310796],[0.164406],[0.645323]]},\"bias_o\":{\"rows\":5,\"columns\":1,\"data\":[[0.912944],[0.197155],[-0.407593],[-0.380658],[-0.907513]]},\"input_nodes\":1,\"hidden_nodes\":3,\"output_nodes\":5,\"weights_ih\":{\"rows\":3,\"columns\":1,\"data\":[[0.765160],[-0.830610],[-0.554092]]},\"weights_ho\":{\"rows\":5,\"columns\":3,\"data\":[[0.804161,-0.485224,0.429943],[-0.475699,0.720266,-0.001433],[0.024648,0.841307,-0.784664],[-0.162800,0.275092,0.379352],[-0.567448,0.457605,-0.802463]]},\"learning_rate\":0.100000}]";
    object1 = json_tokener_parse(njson);
    vnn = neural_network_deserialize_many(object1, &vector_len);
    TEST_ASSERT_EQUAL(1, isEqual((const NeuralNetwork**)vnn, vector_len));
}

void test_deser_file_many_jsondoc(void) {
    vnn = neural_network_deserialize_file_many(NN_TEST_DIR "test-load_many.json", &vector_len);
    TEST_ASSERT_EQUAL(1, isEqual((const NeuralNetwork**)vnn, vector_len));
}


int main(void) {
    UnityBegin("unit-json_deserialize.c");
    RUN_TEST(test_deser_mem_one_jsondoc);
    RUN_TEST(test_deser_file_one_jsondoc);
    RUN_TEST(test_deser_mem_many_jsondoc);
    RUN_TEST(test_deser_file_many_jsondoc);
    return UNITY_END();
}
