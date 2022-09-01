const fs = require('fs')
const statistics = require("./statistics.js")
const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;
const utils = require("./utils.js")


function initialize_maps(config, use_map_weights=true){
    let bioms = require("../data/bioms.json")
    let map_weights = require("../data/mapweights.json")
    let distances = statistics.getAllMapDistances(bioms)
    let maps = []
    let layers = require("../data/layers.json")

    for (const [map_name, biom_values] of Object.entries(bioms)) {
        // skip map if no layers available
        if (!(map_name in layers)) continue
        let weight = 0
        if (use_map_weights){
            // use map_weight_corrction = 0 if map is not in mapweights
            try{
                weight = map_weights[map_name]
            }catch(e){
                console.log("WARNING: Map '"+map_name+"' has no saved correction weights, this may destroy the map distribution!")
            }
        }

        let map_ = new Map(map_name, biom_values, weight, distances[map_name])
        for(let mode of Object.keys(layers[map_name])){
            for(let layer of layers[map_name][mode]){
                let l = new Layer(layer["name"], mode, map_, layer["votes"])
                map_.add_layer(l)
            } 
        }
        map_.lock_time = config["biom_spacing"]
        map_.calculate_vote_weights_by_mode()
        maps.push(map_)
    }

    //init neighbors 
    let cluster_radius = config["min_biom_distance"]
    for(let i=0;i<maps.length;i++){
        maps[i].neighbors = [];
        maps[i].neighbor_count = 0;
        for(let j=0;j<maps.length;j++){
            if(maps[i].distances[maps[j].name] < cluster_radius){
                maps[i].neighbors.push(maps[j]);
                maps[i].neighbor_count++;
            }
        }
    }
    for(let map of maps){
        map.set_layer_by_pools(config)
        map.add_mapvote_weights()
    }
    return maps
}

class Map{
    constructor(name, bioms, map_weights, distances, ){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = map_weights
        this.mapvote_weights = {}
        this.total_probabilities = {}
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        this.layer_by_pools = {}
        this.target_map_dist = {}
        //for optimizer
        this.distribution = 0
        this.mode_groups = []
        this.vote_weights_by_mode = {}

    }
    add_layer(layer){
        if (!(layer.mode in this.layers)) this.layers[layer.mode] = [layer]
        else this.layers[layer.mode].push(layer)
    }
    decrease_lock_time(){
        if(this.current_lock_time >= 1){
            this.current_lock_time--;
        }
    }
    update_lock_time(){
        this.current_lock_time = this.lock_time
    }
    add_mapvote_weights(){
        let votesum = {}
        let weights = {}
        let means = {}
        let sum = 0
        if(this.layers.length != 0){
            for(let l of Object.keys(this.layers)){
                if(this.layer_by_pools[l]){
                    for(let layer of Object.values(this.layers[l])){
                        if(votesum[this.layer_by_pools[l]]){
                            votesum[this.layer_by_pools[l]].push(layer.votes)
                        }
                        else{
                            votesum[this.layer_by_pools[l]] = [layer.votes]
                        }
                    }
                }
            }
            let temp = {}
            for(let pool of Object.keys(votesum)){
                means[pool] = 1/votesum[pool].length*utils.sumArr(votesum[pool])
                for(let v of Object.values(votesum[pool])){
                    if(weights[pool]){
                        weights[pool].push(Math.exp(-Math.pow(means[pool] - v, 2)))
                    }
                    else{
                        weights[pool] = [Math.exp(-Math.pow(means[pool] - v, 2))]
                    }
                }
                let sum = utils.sumArr(weights[pool]) 
                for(let w of Object.keys(weights[pool])){
                    weights[pool][w] = weights[pool][w]/sum
                }
                for(let i=0; i<votesum[pool].length; i++){
                    weights[pool][i] *= votesum[pool][i]
                }
                temp[pool] = utils.sumArr(weights[pool])
            }
            this.mapvote_weights = temp
        }
        else{
            console.log(`No layers added to map ${this.name}, could not calculate mapvote_weights!`)
        }
    }

    set_layer_by_pools(config){
        this.layer_by_pools = get_mode_to_pools_dict(config)
    }
    calculate_vote_weights_by_mode(sigmoid_slope=1, sigmoid_shift=0){
        if (Object.entries(this.layers).length === 0) throw Error(`map '${this.name}' has no layers to calculate weights`)

        for(let mode of Object.keys(this.layers)){
            let votes = []
            for(let layer of this.layers[mode]) votes.push(layer.votes)
            let weights = utils.normalize(statistics.sigmoidArr(votes, sigmoid_slope, sigmoid_shift))
            this.vote_weights_by_mode[mode] = weights
        }
    }
}

class Layer{
    constructor(name, mode, map, votes){
        this.name = name
        this.mode = mode
        this.map = map
        this.votes = votes
    }
}

function get_mode_to_pools_dict(config){
    let temp = {}
    for(let mode_group of Object.keys(config["mode_distribution"]["pools"])){
        if(mode_group != null){
            for(let mode of Object.keys(config["mode_distribution"]["pools"][mode_group])){               
                temp[mode] = mode_group 
            }
        }
    }
    return temp
}

function get_layers(){
    let theUrl = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req"
    let xmlHttpReq = new XMLHttpRequest();
    xmlHttpReq.open("GET", theUrl, false);
    xmlHttpReq.send(null);

    let data = JSON.parse(xmlHttpReq.responseText)
    let layers = data["AxisLabels"]
    let upvotes = data["DataSet"][0]
    let downvotes = data["DataSet"][1]
    let availability = data["DataSet"][2]
    let maps = {}
    for(let i=0; i<layers.length; i++){
        if (availability[i] > 0) continue
        let layer = utils.formatLayer(layers[i])
        let layer_values = layer.split("_")
        let map = layer_values[0]
        let mode = layer_values[1]

        let data = {"name": layer, "votes": upvotes[i]+downvotes[i]}
        if (maps.hasOwnProperty(map)){
            if(maps[map].hasOwnProperty(mode)){
                maps[map][mode].push(data)
            }else{
                maps[map][mode] = [data]
            }
        }else maps[map] = {mode: [data]}
    }
    return maps
}

// Test Stuff here
if (require.main === module) {
    let config = require("../config.json")
    let maps = initialize_maps(config)
    console.log(maps[0])
    statistics.calcMapDistribution(maps)
    let müll = 0
    for(let i=0; i<maps.length; i++){
        if(maps[i].total_probabilities["rest"]){
            müll += maps[i].total_probabilities["rest"]
            console.log(`Rest mode sum: ${müll}`)
        }
    }
}

module.exports = { Map, Layer, initialize_maps, get_layers };