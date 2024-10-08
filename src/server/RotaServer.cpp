
#include <boost/json/array.hpp>
//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <boost/json.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <chrono>
#include <exception>
#include <httplib.h>
#include <iostream>

#include <GlobalConfig.hpp>
#include <fstream>
#include <map>
#include <thread>
#include <vector>

#include "Generator.hpp"
#include "OptimizerConfig.hpp"
#include "OptimizerData.hpp"
#include "RotaConfig.hpp"
#include "RotaMode.hpp"
#include "RotaOptimizer.hpp"
#include "RotaServer.hpp"

#define TIMEOUT_WAIT_FOR_OTHER_THREAD 1000 // in sec

namespace json = boost::json;

std::string path = std::string(CONFIG_PATH) + "config.json";
rota::RotaConfig *conf;
rota::Generator *gen;
std::mutex gen_mutex;
std::thread *serverThread;
bool running = true;

int main(int ac, char **av) {
    std::string host = "0.0.0.0";
    int port = 1330;
    if (ac == 3) {
        try {
            host = std::string(av[1]);
            port = atoi(av[2]);
        } catch (std::exception &e) {
            std::cerr << "invalide arguments" << std::endl;
            return 1;
        }
    }

    std::cout << "Start Server on " << host << ":" << port << std::endl;
    std::cout << "Rota Version " << ROTA_VERSION_MAJOR << "."
              << ROTA_VERSION_MINOR << "." << ROTA_VERSION_PATCH << std::endl;

    try {
        std::cout << "Start Interface" << std::endl;
        serverThread = new std::thread(&handleInterface, &host, port);

        gen_mutex.lock();
        std::cout << "Init generator and run optimizer" << std::endl;
        gen = initialize();
        std::cout << "Ready" << std::endl;
        gen_mutex.unlock();

        while (running) {
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(60s);
            }
        }
        return 0;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

void handleInterface(std::string *host, int port) {
    // basic Server
    // httplib::SSLServer svr(CERT_PATH, PRIVATE_KEY_PATH);
    try {
        httplib::Server svr;
        svr.Post("/rota",
                 [](const httplib::Request &req, httplib::Response &res) {
                     if (gen_mutex.try_lock()) {
                         handleGetRota(req, res);
                         gen_mutex.unlock();
                     } else {
                         // resource occupied
                         json::object retObj;
                         retObj["error"] = "locked";
                         res.status = 423;
                         res.set_content(json::serialize(retObj), "text/json");
                     }
                 });

        svr.Post("/rota/proposal",
                 [](const httplib::Request &req, httplib::Response &res) {
                     if (gen_mutex.try_lock()) {
                         handleGetProposal(req, res);
                         gen_mutex.unlock();
                     } else {
                         // resource occupied
                         json::object retObj;
                         retObj["error"] = "locked";
                         res.status = 423;
                         res.set_content(json::serialize(retObj), "text/json");
                     }
                 });

        svr.listen(*host, port); // ip der linux sub machine
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void handleGetRota(const httplib::Request &req, httplib::Response &res) {

    json::object retObj;
    retObj["currSeed"] = gen->getSeed();

    bool error = false;

    try {
        json::object reqObj = json::parse(req.body).as_object();

        if (reqObj.contains("pastRota") && !reqObj.contains("rotaCount")) {

            std::vector<std::string> pastRota;

            json::array pastLayers = reqObj.at("pastRota").as_array();

            // check if all layer are available in generator
            for (auto s : pastLayers) {
                if (gen->getLayerMap()->count((std::string)s.as_string())) {
                    pastRota.push_back((std::string)s.as_string());
                } else {
                    error = true;
                    res.status = 400;

                    retObj["error"] = "cannot find " + json::serialize(s);
                    break;
                }
            }

            if (!error) {
                gen->reset(&pastRota);
                gen->generateRota(false, conf->get_number_of_layers() -
                                             pastRota.size());
                json::array ret;
                for (rota::RotaLayer *layer : *gen->getRota()) {
                    ret.push_back(json::string(layer->getName()));
                }

                retObj["data"] = ret;
            }

            pastRota.clear();

        } else if (reqObj.contains("rotaCount") &&
                   !reqObj.contains("pastRota")) {
            try {

                int count = reqObj.at("rotaCount").as_int64();
                if (count > 0) {
                    // init new generator
                    gen = initialize();

                    retObj["currSeed"] = gen->getSeed();

                    json::array ret;

                    for (int i = 0; i < count; i++) {
                        json::array rota;
                        gen->generateRota();
                        for (rota::RotaLayer *layer : *gen->getRota()) {
                            rota.push_back(json::value(layer->getName()));
                        }
                        ret.push_back(rota);
                        gen->reset();
                    }

                    retObj["data"] = ret;

                } else {
                    retObj["error"] = "invalide arguments";
                    res.status = 400;
                }
            } catch (std::exception &e) {
                retObj["error"] = e.what();
                res.status = 400;
            }
        } else {
            retObj["error"] = "unknow parameter or too many";
            res.status = 400;
        }

    } catch (std::exception &e) {
        retObj["error"] = e.what();
        res.status = 400;
    }

    res.set_content(json::serialize(retObj), "text/json");
}

void handleGetProposal(const httplib::Request &req, httplib::Response &res) {

    json::object retObj;
    // retObj["lastSeed"] = gen->getSeed();

    try {
        json::object reqObj = json::parse(req.body).as_object();

        if (reqObj.contains("pastRota") && reqObj.contains("count")) {

            std::vector<std::string> pastRota;
            bool error = false;

            int count = reqObj.at("count").as_int64();
            if (count > 0) {
                json::array pastLayers = reqObj.at("pastRota").as_array();

                // check if all layer are available in generator
                for (auto s : pastLayers) {
                    if (gen->getLayerMap()->count((std::string)s.as_string())) {
                        pastRota.push_back((std::string)s.as_string());
                    } else {
                        error = true;
                        res.status = 400;
                        retObj["error"] = "cannot find " + json::serialize(s);
                        break;
                    }
                }

                if (!error) {
                    std::vector<rota::RotaLayer *> offer;
                    gen->reset(&pastRota);

                    retObj["currSeed"] = gen->getSeed();

                    gen->generateOffer(&offer, count);

                    json::array r;
                    for (rota::RotaLayer *layer : offer) {
                        r.push_back(json::string(layer->getName()));
                    }

                    retObj["data"] = r;
                }

            } else {
                retObj["error"] = "invalide arguments";
                res.status = 400;
            }

        } else {
            retObj["error"] = "unknow parameter";
            res.status = 400;
        }

    } catch (std::exception &e) {
        retObj["error"] = e.what();
        res.status = 400;
    }
    res.set_content(json::serialize(retObj), "text/json");
}

rota::Generator *initialize() {
    // delete old generator and config
    delete gen;
    delete conf;

    conf = new rota::RotaConfig((path));
    rota::Generator *gen = new rota::Generator(conf);

    // run optimizer
    std::map<rota::RotaMode *, OptDataIn *> dataIn;
    std::map<rota::RotaMode *, OptDataOut *> dataOut;
    std::vector<std::thread *> threads;

    for (auto const &x : *gen->getModes()) {
        if (x.second->modePool != nullptr && x.second->probability > 0.0) {
            dataIn[x.second] = new OptDataIn;
            dataOut[x.second] = new OptDataOut;
        }
    }

    for (auto const &x : dataIn) {
        gen->packOptData(x.second, x.first);
        std::thread *t = new std::thread(&runOpt, *x.second, conf,
                                         dataOut[x.first], x.first->name);
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

void runOpt(OptDataIn dataIn, rota::RotaConfig *conf, OptDataOut *dataOut,
            std::string modeName) {
    optimizer::OptimizerConfig optConfig(
        conf->get_biom_spacing(), dataIn.clusters, dataIn.mapDist, modeName);
    optimizer::RotaOptimizer opt(optConfig);
    dataOut->mapWeights = opt.Run();
}