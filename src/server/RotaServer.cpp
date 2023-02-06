
#include <boost/json/array.hpp>
//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <boost/json.hpp>
#include <httplib.h>
#include <iostream>

#include <GlobalConfig.hpp>
#include <vector>

#include "Generator.hpp"
#include "OptimizerConfig.hpp"
#include "OptimizerData.hpp"
#include "RotaMode.hpp"
#include "RotaOptimizer.hpp"
#include "RotaServer.hpp"

int main(int ac, char **av) {

    namespace json = boost::json;

    std::cout << "Start Server" << std::endl;
    std::cout << "Rota Version " << ROTA_VERSION_MAJOR << "."
              << ROTA_VERSION_MINOR << std::endl;

    rota::Generator *gen = initialize();

    // need static generator object to check if a layer exists

    // basic Server
    // httplib::SSLServer svr(CERT_PATH, PRIVATE_KEY_PATH);
    /*httplib::Server svr;
    svr.Get(
        "/getRota", [](const httplib::Request &req, httplib::Response &res) {
            if (req.has_param("pastRota")) {

                std::cout << req.get_param_value("pastRota") << std::endl;
                try {
                    json::array pastLayers =
                        json::parse(req.get_param_value("pastRota")).as_array();

                    res.set_content("ok", "text/plain");
                } catch (std::exception &e) {
                    res.status = 418;
                    res.set_content("json parse error", "text/plain");
                }
                // TODO check if all past layer are available to the generator
                // object
                // TODO hier fehlt das generator object
            } else {
                res.set_content("without data, WO ROTA", "text/plain");
                // TODO hier fehlt mir das Rota generator object
            }
        });

    svr.listen("172.30.249.128", 1337); // ip der linux sub machine*/
}

rota::Generator *initialize() {
    std::string path = std::string(CONFIG_PATH) + "config.json";
    rota::RotaConfig conf(path);
    rota::Generator *gen = new rota::Generator(&conf);
    time_t t = time(nullptr);
    std::tm lastOptRun = *std::localtime(&t);

    std::cout << lastOptRun.tm_hour << std::endl;

    // run optimizer
    std::map<rota::RotaMode *, OptDataIn *> dataIn;
    std::map<rota::RotaMode *, OptDataOut *> dataOut;
    std::vector<std::thread *> threads;

    for (auto const &x : *gen->getModes()) {
        dataIn[x.second] = new OptDataIn;
        dataOut[x.second] = new OptDataOut;
    }

    for (auto const &x : dataIn) {
        gen->packOptData(x.second, x.first);
        std::thread *t =
            new std::thread(&runOpt, x.second, &conf, dataOut[x.first]);
        threads.push_back(t);
    }

    for (std::thread *t : threads) {
        t->join();
    }

    for (auto const &x : dataOut) {
        gen->setMapWeights(x.second, x.first);
    }

    // aufräumen
    for (std::thread *t : threads) {
        delete t;
    }

    for (auto const &x : dataIn) {
        delete x.second;
    }

    for (auto const &x : dataOut) {
        delete x.second;
    }

    return gen;
}

void runOpt(OptDataIn *dataIn, rota::RotaConfig *conf, OptDataOut *dataOut) {
    optimizer::OptimizerConfig optConfig(conf->get_biom_spacing(),
                                         dataIn->clusters, dataIn->mapDist);
    optimizer::RotaOptimizer opt(optConfig);
    dataOut->mapWeights = opt.Run();
}