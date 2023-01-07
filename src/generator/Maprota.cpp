#include "Maprota.hpp"
#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include <vector>
#include <boost/json.hpp>
#include "dataParsing.hpp"

namespace rota
{
    Maprota::Maprota(boost::json::object *config){
        this->config = config;
        parseModes(this->config, &this->modePools, &this->modes);
    }

    RotaMode* Maprota::chooseMode(bool useLatestModes=true, RotaModePool *customPool=nullptr){

    }

} // namespace rota

