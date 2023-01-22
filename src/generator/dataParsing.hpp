#pragma once

#include <boost/json.hpp>

#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "RotaModePool.hpp"

namespace rota {
/**
 * @brief parsing all used modes from config.json
 *
 * @param config pointer to config.json object
 * @param allPools empty map of modePools
 * @param allModes empty map of modes
 */
void parseModes(boost::json::object *config,
                std::map<std::string, RotaModePool *> *allPools,
                std::map<std::string, RotaMode *> *allModes);

/**
 * @brief requesting layers from url and parsing used layers
 *
 * @param url Layer API Url
 * @param maps map of used RotaMaps
 * @param layers empty map of used layers
 * @param modes map of used modes
 */
void parseLayers(std::string votesUrl, std::string teamsUrl, std::map<std::string, RotaMap *> *maps,
                 std::map<std::string, RotaLayer *> *layers,
                 std::map<std::string, RotaMode *> *modes,
                 std::map<std::string, RotaTeam *> *teams);

/**
 * @brief parsing all used maps from config to RotaMap Objects
 *
 * @param config config.json object
 * @param maps empty map of RotaMaps
 *
 */
void parseMaps(RotaConfig *config, std::map<std::string, RotaMap *> *maps);

/**
 * @brief maps every team on their containing layers
 *
 * @param layers map of all used layers
 * @param blueforTeams empty map mapping all bluefor factions on their
 * containing layers
 * @param opforTeams empty map mapping all opfor factions on their containing
 * layers
 */
void parseTeams(std::map<std::string, RotaLayer *> *layers,
                std::map<RotaTeam *, std::vector<RotaLayer *>> *blueforTeams,
                std::map<RotaTeam *, std::vector<RotaLayer *>> *opforTeams);
} // namespace rota
