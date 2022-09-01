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
    return maps
}

class Map{
    constructor(name, bioms, map_weights, distances, ){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = map_weights
        this.mapvote_weights = {}
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        this.layer_by_pools = {}
        this.target_map_dist = {}
        //for optimizer
        this.distribution = 0
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
    
    // ??
    add_mapvote_weights(){
        votes = {}
        weights = {}
        means = {}
        sum = 0
        if(this.layers.length != 0){
            for(let pool in this.layer_by_pools){
                for(let l in this.layer_by_pools[pool]){
                    votes[pool].push.apply(votes[pool], [l.votes])
                }
                means[pool] = 1/votes[pool].length*utils.sumArr(votes[pool])
                for(let votes in votes[pool]){
                    if(weights[pool] != null){
                        weights[pool] += Math.exp(-Math.pow(means[pool] - votes, 2))
                    }
                    else{
                        weights[pool] = 0
                    }
                }
                for(let w in weights[pool]){
                    w = w/utils.sumArr(weights)
                }
            }
        }
        else{
            console.log(`No layers added to map ${this.name}, could not calculate mapvote_weights!`)
        }
    }

    calculate_vote_weights_by_mode(){
        if (Object.entries(this.layers).length === 0) throw Error(`map '${this.name}' has no layers to calculate weights`)

        for(let mode of this.layers){
            let votes = []
            for(let layer of this.layers[mode]) votes.push(layer.votes)
            let weights = utils.normalize(statistics.sigmoidArr(votes))
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
    maps[0].add_mapvote_weights()
    console.log(maps[0])
}

module.exports = { Map, Layer, initialize_maps, get_layers };