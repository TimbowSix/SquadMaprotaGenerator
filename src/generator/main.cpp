#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <exception>
#include <boost/json.hpp>

#include "RotaConfig.hpp"

int main(void)
{
    std::cout << "Version " << ROTA_VERSION_MAJOR << "." << ROTA_VERSION_MINOR << std::endl;

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
    }

    return 0;
}