
#include <boost/json/array.hpp>
//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <boost/json.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <exception>
#include <httplib.h>
#include <iostream>

#include <GlobalConfig.hpp>
#include <fstream>
#include <map>
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

int main(int ac, char **av) {

    std::cout << "Start Server" << std::endl;
    std::cout << "Rota Version " << ROTA_VERSION_MAJOR << "."
              << ROTA_VERSION_MINOR << std::endl;

    std::cout << "Init generator and run optimizer" << std::endl;
    gen = initialize();
    gen->generateRota();
    std::cout << "Ready" << std::endl;

    std::ofstream file;
    file.open("rota.dat");
    for (rota::RotaLayer *layer : *gen->getRota()) {
        file << layer->getName() << "\n";
    }
    file.close();
    return 0;
    // basic Server
    // httplib::SSLServer svr(CERT_PATH, PRIVATE_KEY_PATH);
    httplib::Server svr;
    svr.Get("/getRota",
            [](const httplib::Request &req, httplib::Response &res) {
                gen_mutex.lock();
                handleGetRota(req, res);
                gen_mutex.unlock();
            });

    svr.Get("/getProposal",
            [](const httplib::Request &req, httplib::Response &res) {
                gen_mutex.lock();
                handleGetProposal(req, res);
                gen_mutex.unlock();
            });

    svr.listen("172.29.43.69", 1330); // ip der linux sub machine
    return 0;
}

void handleGetRota(const httplib::Request &req, httplib::Response &res) {

    json::object retObj;

    if (req.has_param("pastRota")) {
        bool error = false;

        std::vector<std::string> pastRota;
        try {
            json::array pastLayers =
                json::parse(req.get_param_value("pastRota")).as_array();

            // check if all layer are available in generator
            for (auto s : pastLayers) {
                if (gen->getLayerMap()->count((std::string)s.as_string())) {
                    pastRota.push_back((std::string)s.as_string());
                } else {
                    error = true;
                    res.status = 418;

                    retObj["status"] = 418;
                    retObj["msg"] = "cannot find " + json::serialize(s);
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

                retObj["status"] = 200;
                retObj["msg"] = ret;
            }

        } catch (std::exception &e) {
            error = true;

            retObj["status"] = 418;
            retObj["msg"] = e.what();
            res.status = 418;
        }
        pastRota.clear();

    } else if (req.has_param("rotaCount")) {
        try {
            int count = std::stoi(req.get_param_value("rotaCount"));
            if (count > 0) {
                // init new generator
                gen = initialize();

                json::array ret;

                for (int i = 0; i < count; i++) {
                    json::array rota;
                    gen->generateRota();
                    for (rota::RotaLayer *layer : *gen->getRota()) {
                        rota.push_back(json::value(layer->getName()));
                    }
                    ret.push_back(rota);
                }

                retObj["status"] = 200;
                retObj["msg"] = ret;

            } else {
                retObj["status"] = 418;
                retObj["msg"] = "invalide arguments";
                res.status = 418;
            }
        } catch (std::exception &e) {
            retObj["status"] = 418;
            retObj["msg"] = e.what();
            res.status = 418;
        }
    } else {
        retObj["status"] = 418;
        retObj["msg"] = "unknow parameter";
        res.status = 418;
    }

    res.set_content(json::serialize(retObj), "text/json");
}

void handleGetProposal(const httplib::Request &req, httplib::Response &res) {

    json::object retObj;

    if (req.has_param("pastRota") && req.has_param("count")) {

        std::vector<std::string> pastRota;
        bool error = false;

        try {
            int count = std::stoi(req.get_param_value("count"));
            if (count > 0) {
                json::array pastLayers =
                    json::parse(req.get_param_value("pastRota")).as_array();

                // check if all layer are available in generator
                for (auto s : pastLayers) {
                    if (gen->getLayerMap()->count((std::string)s.as_string())) {
                        pastRota.push_back((std::string)s.as_string());
                    } else {
                        error = true;
                        res.status = 418;
                        retObj["status"] = 418;
                        retObj["msg"] = "cannot find " + json::serialize(s);
                        break;
                    }
                }

                if (!error) {
                    std::vector<rota::RotaLayer *> offer;
                    gen->reset(&pastRota);
                    gen->generateOffer(&offer, count);

                    json::array r;
                    for (rota::RotaLayer *layer : offer) {
                        r.push_back(json::string(layer->getName()));
                    }

                    retObj["status"] = 200;
                    retObj["msg"] = r;
                }

            } else {
                retObj["status"] = 418;
                retObj["msg"] = "invalide arguments";
                res.status = 418;
            }
        } catch (std::exception &e) {
            retObj["status"] = 418;
            retObj["msg"] = e.what();
            res.status = 418;
        }
    } else {
        retObj["status"] = 418;
        retObj["msg"] = "unknow parameter";
        res.status = 418;
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
        std::thread *t =
            new std::thread(&runOpt, *x.second, conf, dataOut[x.first]);
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

void runOpt(OptDataIn dataIn, rota::RotaConfig *conf, OptDataOut *dataOut) {
    optimizer::OptimizerConfig optConfig(conf->get_biom_spacing(),
                                         dataIn.clusters, dataIn.mapDist);
    optimizer::RotaOptimizer opt(optConfig);
    dataOut->mapWeights = opt.Run();
}