from __future__ import annotations
import numpy as np
from scipy.special import expit
import utils
import math
import json


def initialize_maps(config):
    with open("data/layers.json", "r") as layer_f, open("data/bioms.json", "r") as bioms_f:
        layers = json.load(layer_f)
        bioms = json.load(bioms_f)
    distances = getAllMapDistances(bioms)
    maps = []
    for map_name, biom_values in bioms.items():
        if map_name not in layers:
            continue
        map_ = Map(map_name, biom_values, distances[map_name])
        for mode in layers[map_name]:
            for layer in layers[map_name][mode]:
                l = Layer(layer["name"], mode, map_, layer["votes"])
                map_.add_layer(l)

        maps.append(map_)
    
    for map_ in maps:
        map_.add_mapvote_weights(config["mapvote_slope"], config["mapvote_shift"])

    #// normalize mapvote weights by mode
    mode_probs = {} #//{mode:{map:prob}}
    for map_ in maps:
        for mode in map_.mapvote_weights:
            if mode in mode_probs: 
                mode_probs[mode][map_.name] = map_.mapvote_weights[mode]
            else:
                mode_probs[mode] = {map_.name:map_.mapvote_weights[mode]}

    for mode in mode_probs:
        mode_weights = list(mode_probs[mode].values())
        mode_weights = utils.normalize(mode_weights)
        mode_maps = list(mode_probs[mode].keys())
        for ind, mm in enumerate(mode_maps):
            mode_probs[mode][mm] = mode_weights[ind]

    for map_ in maps:
        weights = {}
        for mode in map_.layers:
            weights[mode] = mode_probs[mode][map_.name]
        map_.mapvote_weights = weights

    map_dist = {}
    for map_ in maps:
        if map_.name not in map_dist:
            map_dist[map_.name] = {}
        for mode in mode_probs:
            if map_.name in mode_probs[mode]:
                if (map_.name in map_dist):
                    map_dist[map_.name][mode] = mode_probs[mode][map_.name]
                else:
                    map_dist[map_.name] = {mode:mode_probs[mode][map_.name]}
    with open("pytest.json", "w") as f:
        json.dump(map_dist, f, indent=2)

class Map:
    def __init__(self, name: str, bioms: list,  distances: dict) -> None:
        self.name = name
        # {mode: [Layer]}
        self.layers = {}
        self.bioms = bioms
        self.distances = distances
        self.neighbors = []
        self.neighbor_count = 0

    def add_layer(self, layer: Layer) -> None:
        if layer.mode not in self.layers:
            self.layers[layer.mode] = [layer]
        else:
            # existance check?
            self.layers[layer.mode].append(layer)

    def __repr__(self) -> str:
        return self.name

    def add_mapvote_weights(self, slope, shift):
        if(len(self.layers) == 0):
            print(f"No layers added to map ${self.name}, could not calculate mapvote_weights!")
            return
        votesum = {}
        for mode in self.layers:
            votes = []
            for layer in self.layers[mode]: 
                votes.append(layer.votes)
            votesum[mode] = votes
        means = {}
        weights = {}
        temp = {}
        for mode in votesum:
            means[mode] = 1/len(votesum[mode])*sum(votesum[mode])
            for v in votesum[mode]:
                if(mode in weights):
                    weights[mode].append(math.exp(-math.pow(means[mode] - v, 2)))
                else:
                    weights[mode] = [math.exp(-math.pow(means[mode] - v, 2))]

            weights[mode] = utils.normalize(weights[mode])
            if mode == "AAS":
                print(self.name, weights[mode])

            for i in range(len(votesum[mode])):
                weights[mode][i] *= votesum[mode][i]

            temp[mode] = sigmoid(sum(weights[mode]), slope, shift)
        self.mapvote_weights = temp

class Layer:
    def __init__(self, name: str, mode: str, map: Map, votes: int) -> None:
        self.name = name
        self.mode = mode
        self.map = map
        self.votes = votes
    
    def __repr__(self) -> str:
        return self.name


def getAllMapDistances(allMapsDict):
    numberOfMaps = len(allMapsDict)
    distancesDict = {}
    for k in range(numberOfMaps):
        mapkey = list(allMapsDict.keys())[k]
        temp = {}  # np.zeros(numberOfMaps)
        for i in range(numberOfMaps):
            if (mapkey == list(allMapsDict.keys())[i]):
                temp[list(allMapsDict.keys())[i]] = 0
            # elif(k>i and distancesDict[mapkey][i] != 0):
            #     temp[i] = distancesDict[mapkey][i]
            else:
                absValue1 = np.sqrt(np.sum(np.multiply(allMapsDict[mapkey], allMapsDict[mapkey])))
                absValue2 = np.sqrt(np.sum(
                    np.multiply(allMapsDict[list(allMapsDict.keys())[i]], allMapsDict[list(allMapsDict.keys())[i]])))
                temp[list(allMapsDict.keys())[i]] = np.arccos(
                    np.sum(np.multiply(allMapsDict[mapkey], allMapsDict[list(allMapsDict.keys())[i]])) / (
                            absValue1 * absValue2))
        distancesDict[mapkey] = temp
    return distancesDict

def sigmoid(x, slope, shift):
    arg = slope*(x+shift)
    return 1/(1+math.exp(-arg))


def main():
    with open("config.json", "r") as f:
        config = json.load(f)
    initialize_maps(config)

if __name__ == "__main__":
    main()
