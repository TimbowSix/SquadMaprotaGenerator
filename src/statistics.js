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

function calcMapDistribution(allMaps, weightKeys){
    //TODO slope sigmoid über config ?
    for(weight_key of weightKeys){
        let tempSum = 0;
        for(map of allMaps){
            map.target_map_dist[weight_key] = 0;
            let avgVotes = 0; //mu
            let layers = []
            //sum layer votes
            for(mode of Object.keys(map.layers)){
                for(layer of mode){
                    avgVotes += layer.votes;
                    layers.push(layer);
                }
            }
            if(layersCount == 0){
                throw "map with no layers";
            }
            avgVotes /= layers.length;

            //calc ú
            let weightSum = 0;
            let wi = 0;
            for(layer of layers){
                //calc wi
                wi = math.exp(- math.sqrt(avgVotes-layer.votes));
                //calc W
                weightSum += wi;
                map.target_map_dist[weight_key] += wi * layer.votes;
            }
            map.target_map_dist[weight_key] /= weightSum;
            //into sigmoid
            map.target_map_dist[weight_key] = sigmoid(map.target_map_dist[weightSum], 0.1, 0);
            tempSum += map.target_map_dist[weight_key];
        }
        //normalize
        for(map of allMaps){
            map.target_map_dist[weight_key] /= tempSum;
        }
    }
}

function sigmoid(x, slope, shift=0){
    let arg = slope*(x+shift)
    return 1/(1+Math.exp(-arg))
}

function sigmoidArr(x, slope, shift=0){
    let res = []
    for(i=0; i<x.length; i++){
        res.push.apply(res, [sigmoid(x, slope, shift)])
    }
    return res
}

module.exports = { getAllMapDistances, getValidMaps };

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