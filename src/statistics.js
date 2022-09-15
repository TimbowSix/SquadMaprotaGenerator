const utils = require("./utils.js")
const fs = require("fs");


function getValidMaps(allMaps, lastChosenMap){
    if(lastChosenMap == null){
        return allMaps;
    }
    for(let i=0;i<allMaps.length;i++){
        allMaps[i].decrease_lock_time();
    }
    for(let i=0;i<allMaps.length;i++){
        if(lastChosenMap.name == allMaps[i].name){
            for(let j=0;j<allMaps[i].neighbor_count;j++){
                allMaps[i].neighbors[j].update_lock_time();
            }
        }
    }
    let valid_maps = [];
    for(let i=0;i<allMaps.length;i++){
        if(allMaps[i].current_lock_time == 0){
            valid_maps.push(allMaps[i]);
        }
    }
    return valid_maps;
}

function getAllMapDistances(allMapsDict){
    let numberOfMaps = Object.keys(allMapsDict).length
    let distancesDict = {}
    for(let k = 0; k < numberOfMaps; k++){
        let mapkey = Object.keys(allMapsDict)[k]
        let temp = {}
        for(let i = 0; i<numberOfMaps; i++){
            if (mapkey == Object.keys(allMapsDict)[i]) temp[Object.keys(allMapsDict)[i]] = 0
            else{
                let absValue1 = Math.sqrt(utils.sumArr(utils.squareArr(allMapsDict[mapkey], allMapsDict[mapkey])))
                let absValue2 = Math.sqrt(utils.sumArr(utils.squareArr(allMapsDict[Object.keys(allMapsDict)[i]], allMapsDict[Object.keys(allMapsDict)[i]])))
                temp[Object.keys(allMapsDict)[i]] = Math.acos(utils.sumArr(utils.multiplyArr(allMapsDict[mapkey], allMapsDict[Object.keys(allMapsDict)[i]])) / (absValue1 * absValue2))
            }
            distancesDict[mapkey] = temp
        }
    }
    return distancesDict
}

function calcMapDistribution(maps){
    let tempSum = {}
    for(let map of maps){
        for(let pool of Object.keys(map.mapvote_weights)){
            map.total_probabilities[pool] = sigmoid(map.mapvote_weights[pool], 0.1, 0)  // TWEAK SHIFT AND SLOPE!! -> config
            if(tempSum[pool]){
                tempSum[pool] += map.total_probabilities[pool]
            }
            else{
                tempSum[pool] = map.total_probabilities[pool]
            }
        }
    }
    //normalize
    for(let map of maps){
        for(let pool of Object.keys(map.mapvote_weights)){
            if(map.total_probabilities[pool]){
                map.total_probabilities[pool] /= tempSum[pool];
            }
        }
    }
}

function sigmoid(x, slope, shift=0){
    let arg = slope*(x+shift)
    return 1/(1+Math.exp(-arg))
}

function sigmoidArr(x, slope, shift=0){
    let res = []
    for(let i=0; i<x.length; i++){
        res.push(sigmoid(x[i], slope, shift))
    }
    return res
}

function calc_stats(){
    let config = require("../config.json")
    let gen = require("./generator.js");
    

    config["number_of_rotas"] = 1
    config["number_of_layers"] = 100000
    config["seed_layer"] = 0
    config["use_vote_weight"] = true
    config["use_map_weight"] = true

    let mr = new gen.Maprota(config)
    mr.generate_rota(false,true)

    let current_dist = JSON.parse(fs.readFileSync("./data/current_map_dist.json"))

    let rota_dist = {}
    let sum = {}

    for(let layer of mr.rotation){
        if(layer.map.name in rota_dist){
            rota_dist[layer.map.name][layer.mode]++
            if (layer.mode in sum){
                sum[layer.mode]++
            }else{
                sum[layer.mode] = 1
            }
        }else{
            rota_dist[layer.map.name] = {}
            for(let m of Object.keys(layer.map.layers)){
                rota_dist[layer.map.name][m] = 0
            }
            rota_dist[layer.map.name][layer.mode] = 1
        }
    }
    
    for(let map of Object.keys(rota_dist)){
        for(let mode of Object.keys(rota_dist[map])){
            if(rota_dist[map][mode] != 0){
                rota_dist[map][mode] /= sum[mode]
            }
        }
    }
    
    return calc_dist_error(current_dist, rota_dist)
}

function calc_dist_error(reference_dist, measured_dist){
    let error_per_mode = {}
    for(let map of Object.keys(reference_dist)){
        for(let mode of Object.keys(reference_dist[map])){

            let error = Math.pow(reference_dist[map][mode] - measured_dist[map][mode], 2)

            if(mode in error_per_mode){
                error_per_mode[mode] += error
            }else{
                error_per_mode[mode] = error
            }
        }
    }
    return error_per_mode
}

module.exports = { getAllMapDistances, getValidMaps, sigmoidArr, sigmoid, calcMapDistribution, calc_stats };

function main(){
    //let bioms = JSON.parse(fs.readFileSync("./data/bioms.json"))
    //let dist = getAllMapDistances(bioms)
    //console.time("test")
    //console.log(dist)
    //for(let i=0; i<1000; i++) getAllMapDistances(bioms)
    //console.timeEnd("test")
}

if (require.main === module) {
    main()
}