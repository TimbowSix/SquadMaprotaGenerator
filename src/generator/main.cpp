#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <exception>
#include <boost/json.hpp>

#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "utils.hpp"


int main(void)
{
    std::cout << "Version " << ROTA_VERSION_MAJOR << "." << ROTA_VERSION_MINOR << std::endl;

    /*
    const std::filesystem::path configFile{"../../../config.json"};

    if (std::filesystem::exists(configFile))
    {
        std::ifstream ifs(configFile);
        std::string data(std::istreambuf_iterator<char>{ifs}, {});
        std::cout << "data" << std::endl;
        //std::cout << data << std::endl;

        try
        {
            boost::json::object pData =  boost::json::parse(data).get_object();
            std::cout << pData["mode_distribution"] << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    else
    {
        std::cout << "no config file" << std::endl;
    }*/


    std::map<std::string, rota::RotaLayer*> layers;
    std::map<std::string, rota::RotaMode*> modes;
    std::map<std::string, rota::RotaTeam*> teams;

    int ret = rota::getLayers("https://api.welovesquad.com", "/votes", &layers);
    rota::injectLayerInfo("https://api.welovesquad.com", "/layers", &layers, &modes, &teams);
    /*
    std::cout << ret << std::endl;

    for(auto const& [key, val]: layers){
        std::cout << val->getName() << " " << val->getVotes() << std::endl;
    }

    for(auto const& [key, val]: modes){
        std::cout << val->name << std::endl;
    }

    for(auto const& [key, val]: teams){
        std::cout << val->getName() << std::endl;
    }*/


    return 0;
}