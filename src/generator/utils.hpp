/**
 * @file utils
 * @brief Utilities with are used bei die Squad MapRota generator
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */

#pragma once
#include <exception>
#include <map>
#include <vector>

#include "RotaLayer.hpp"

namespace rota {
/**
 * @brief chooses an index randomly with probability given by the values
 *
 * Note: the sum of all weight have to be 1
 *
 * @param weights list of weights
 * @returns choosen index
 */
int weightedChoice(std::vector<float> *weights);

/**
 * @brief chooses random index for any container of given length
 *
 * @param length length of container to choose from. Has to be >= 1
 * @returns choosen index
 */
int choice(int length);

/**
 * @brief normalizes all values in list
 *
 * sum can be used, by not using it sum = NULL
 *
 * @param arr list of values to normalize
 * @param sum sum of values in the vector
 */
void normalize(std::vector<float> *arr, float *sum);

/**
 * @brief normalizes all values in list
 *
 * @param arr list of values to normalize
 */
void normalize(std::vector<float> *arr);

/**
 * @brief sigmoid function
 */
float sigmoid(float x, float slope, float shift);

/**
 * @brief Gets all Layers from an Endpoint via GET req. and creates a map of
 * them
 *
 * The endpoint returns a json string. The format may change in the future for
 * now it is:
 * [
 *  {
 *    id: (name) string
 *    upvotes: int
 *    downvotes: int
 *    isExcluded: bool
 *  }
 * ]
 *
 * The layers in the list are only instantiated with given informations
 *
 * @param url url of endpoint
 * @param layers empty map which get filed with all pulled layers
 *
 * @return count of pulled layers
 */
int getLayers(std::string url, std::vector<rota::RotaLayer *> *layers);

/**
 * @brief Gets more infomation of layer over an other endpoint an stores them
 * into the layer object Creates RotaMode and RotaTeam object as necessary and
 * stores them in modes or teams
 *
 * expects a map of layer with no infomation about teams and the gamemode
 * given format
 *  [
 *     {
 *       "id": string,
 *       "level": string pretty name
 *       "gamemode": string
 *       "teamOne": string
 *       "teamTwo": string
 *       "createdAt": date
 *       "updatedAt": date
 *     }
 *  ]
 *
 * Note: sorts list of layers
 *
 * @param url url of endpoint
 * @param layers map of given Layers without additional information
 * @param modes empty map of modes
 * @param teams empty map of teams
 */

void injectLayerInfo(std::string url,
                     std::map<std::string, rota::RotaLayer *> *layers,
                     std::map<std::string, RotaMode *> *modes,
                     std::map<std::string, RotaTeam *> *teams);

/**
 * @brief parse given url into base and sub url
 *
 * @param url url to parse
 * @returns parsed base and sub url as tuple of pointers
 */
std::tuple<std::string, std::string> parseUrl(std::string url);

/**
 * @brief sets neightbour off a maps list, based on the neightbour distance
 *        if a map is closer or equal to die neighbourDistance it is a neighbour
 *
 * @param maps all maps
 * @param neighbourDist distance to determine a neighbour of a map
 */
void setNeighbour(std::vector<RotaMap *> *maps, float neighbourDist);

/**
 * @brief calculates the distance between to maps
 *
 * @param map0
 * @param map1
 * @returns the distance
 */
float getMapDist(RotaMap *map0, RotaMap *map1);

class NotImplementedException : public std::logic_error {
  public:
    NotImplementedException()
        : std::logic_error{"Function not yet implemented"} {}
};

/**
 * @brief Dennis will immer mehr Features deswegen gibt es diese exception
 */
class DennisException : public std::logic_error {
  public:
    DennisException()
        : std::logic_error{"Wieso ist das Feature noch nicht da?"} {}
};

} // namespace rota
