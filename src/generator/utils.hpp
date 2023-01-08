/**
 * @file utils
 * @brief Utilities with are used bei die Squad MapRota generator
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */

#pragma once
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
 * @brief normalizes all values in list
 *
 * sum can be used, by not using it sum = NULL
 *
 * @param arr list of values to normalize
 * @param sum sum of values in the vector
 */
void normalize(std::vector<float> *arr, float *sum);

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
int getLayers(std::string url, std::map<std::string, rota::RotaLayer *> *layers);

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

} // namespace rota
