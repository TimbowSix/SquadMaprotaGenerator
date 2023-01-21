
#include <boost/json/array.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <boost/json.hpp>
#include <httplib.h>
#include <iostream>

#include <GlobalConfig.hpp>

#include "RotaServer.hpp"

int main(int ac, char **av) {

    namespace json = boost::json;

    std::cout << "Start Server" << std::endl;
    std::cout << "Rota Version " << ROTA_VERSION_MAJOR << "."
              << ROTA_VERSION_MINOR << std::endl;

    // need static generator object to check if a layer exists

    // basic Server
    // httplib::SSLServer svr(CERT_PATH, PRIVATE_KEY_PATH);
    httplib::Server svr;
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

    svr.listen("172.30.249.128", 1337); // ip der linux sub machine
}