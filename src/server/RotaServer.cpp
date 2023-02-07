
#include <boost/json/array.hpp>
//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <boost/json.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <exception>
#include <httplib.h>
#include <iostream>
#include <stdlib.h>

#include <GlobalConfig.hpp>
#include <vector>

#include "Generator.hpp"
#include "OptimizerConfig.hpp"
#include "OptimizerData.hpp"
#include "RotaMode.hpp"
#include "RotaOptimizer.hpp"
#include "RotaServer.hpp"

namespace json = boost::json;

int main(int ac, char **av) {

    std::cout << "Start Server" << std::endl;
    std::cout << "Rota Version " << ROTA_VERSION_MAJOR << "."
              << ROTA_VERSION_MINOR << std::endl;

    // need static generator object to check if a layer exists

    // basic Server
    // httplib::SSLServer svr(CERT_PATH, PRIVATE_KEY_PATH);
    httplib::Server svr;
    svr.Get("/getRota",
            [](const httplib::Request &req, httplib::Response &res) {
                handleGetRota(req, res);
            });

    svr.Get("/GetProposal",
            [](const httplib::Request &req, httplib::Response &res) {
                handleGetProposal(req, res);
            });

    svr.listen("172.30.226.156", 1337); // ip der linux sub machine
}

void handleGetRota(const httplib::Request &req, httplib::Response &res) {

    rota::Generator *gen = initialize();

    json::object retObj;

    if (req.has_param("pastRota")) {
        bool error = false;
        std::cout << req.get_param_value("pastRota") << std::endl;

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
                gen->generateRota();
                json::array ret;
                for (rota::RotaLayer *layer : *gen->getRota()) {
                    ret.push_back(json::string(layer->getName()));
                }

                retObj["status"] = 200;
                retObj["msg"] = json::serialize(ret);
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
                // delete current generator
                delete gen;
                // init new generator
                gen = initialize();

                json::array ret;

                for (int i = 0; i < count; i++) {
                    json::array rota;
                    gen->generateRota();
                    for (rota::RotaLayer *layer : *gen->getRota()) {
                        rota.push_back(json::string(layer->getName()));
                    }
                    ret.push_back(rota);
                }

                retObj["status"] = 200;
                retObj["msg"] = json::serialize(ret);

            } else {
                retObj["status"] = 418;
                retObj["msg"] = "number smaller then 0";
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
    delete gen;
}

void handleGetProposal(const httplib::Request &req, httplib::Response &res) {
    rota::Generator *gen = initialize();
    json::object retObj;

    if (req.has_param("pastRota") && req.has_param("count")) {

        std::vector<std::string> pastRota;
        bool error = false;

        try {
            int count = std::stoi(req.get_param_value("pastRota"));
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
                    retObj["msg"] = json::serialize(r);
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
    delete gen;
}

rota::Generator *initialize() {
    std::string path = std::string(CONFIG_PATH) + "config.json";
    rota::RotaConfig conf(path);
    rota::Generator *gen = new rota::Generator(&conf);

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
            new std::thread(&runOpt, *x.second, &conf, dataOut[x.first]);
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