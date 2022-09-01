const utils = require("./utils.js")
const fs = require("fs")

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

module.exports = { getAllMapDistances, getValidMaps, sigmoidArr, sigmoid, calcMapDistribution };

function main(){
    let bioms = JSON.parse(fs.readFileSync("./data/bioms.json"))
    //let dist = getAllMapDistances(bioms)
    console.time("test")
    //console.log(dist)
    for(let i=0; i<1000; i++) getAllMapDistances(bioms)
    console.timeEnd("test")
}

if (require.main === module) {
    main()
}