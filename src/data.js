const fs = require('fs')
const statistics = require("./statistics.js")
const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;
const utils = require("./utils.js")


function initialize_maps(config, use_map_weights=true){
    let bioms = require("../data/bioms.json")
    let distances = statistics.getAllMapDistances(bioms)
    let maps = []
    let layers = require("../data/layers.json")
    let map_weights = {}
    if (use_map_weights) map_weights = calculate_weights(config)

    for (const [map_name, biom_values] of Object.entries(bioms)) {
        // skip map if no layers available
        if (!(map_name in layers)) continue

        let map_ = new Map(map_name, biom_values, undefined, distances[map_name])
        for(let mode of Object.keys(layers[map_name])){
            for(let layer of layers[map_name][mode]){
                let l = new Layer(layer["name"], mode, map_, layer["votes"])
                map_.add_layer(l)
            }
        }

        //use calculatet map weights or set every weight to 0
        if (use_map_weights){
            map_.map_weight = map_weights[map_.name]
        }else{
            let weight = {}
            for(let mode of Object.keys(map_.layers)){
                weight[mode] = 0
            }
            map_.map_weight = weight
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
        map.add_mapvote_weights(config["mapvote_slope"])
    }
    return maps
}

function calculate_weights(config, save_map_dist=true){
    let maps = initialize_maps(config, false)
    let weights = {} //{map:{mode:weight}}
    let mode_probs = {} //{mode:{map:prob}}
    for(let map of maps){
        for(let mode of Object.keys(map.mapvote_weights)){
            if(mode in mode_probs) mode_probs[mode][map.name] = map.mapvote_weights[mode]
            else mode_probs[mode] = {[map.name]:map.mapvote_weights[mode]}
        }
    }
    let neighbors = {} //{map:neighbor_count}
    for(let map of maps){
        neighbors[map.name] = map.neighbor_count-1
    }

    for(let mode of Object.keys(mode_probs)){
        let mode_weights = Object.values(mode_probs[mode])
        mode_weights = utils.normalize(mode_weights)

        let mode_maps = Object.keys(mode_probs[mode])
        for(let i=0; i<mode_maps.length; i++){
            mode_probs[mode][mode_maps[i]] = mode_weights[i]
        }
    }
    
    if(save_map_dist){
        let map_dist = {}
        for(let map of maps){
            if(!(map.name in map_dist)){
                map_dist[map.name] = {}
            }
            for(let mode of Object.keys(mode_probs)){
                if(Object.keys(mode_probs[mode]).includes(map.name)){
                    map_dist[map.name][mode] = mode_probs[mode][map.name]
                }
            }
        }
        fs.writeFileSync("./data/current_map_dist.json",JSON.stringify(map_dist, null, 2))
    }

    let formulas = require("../data/weight_formulas.json")
    for(let [mode, maps] of Object.entries(mode_probs)){
        for(let map of Object.keys(maps)){
            x = neighbors[map]
            y = mode_probs[mode][map]
            weight = eval(formulas[mode])
            if(map in weights) weights[map][mode] = weight
            else weights[map] = {[mode]:weight}
        }
    }
    return weights
    //fs.writeFileSync("../data/mapweights.json", JSON.stringify(weights, null, 2))
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
        this.mode_groups = [] //TODO kann weg wenn es keiner mehr braucht
        this.vote_weights_by_mode = {}
        this.weight_parameter = {}

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
    add_mapvote_weights(slope=1){
        if(this.layers.length == 0) {
            console.log(`No layers added to map ${this.name}, could not calculate mapvote_weights!`); 
            return
        }
        let votesum = {}
        for(let mode of Object.keys(this.layers)){
            let votes = []
            for(let layer of this.layers[mode]) votes.push(layer.votes)
            votesum[mode] = votes
        }
        let means = {}
        let weights = {}
        let temp = {}
        for(let mode of Object.keys(votesum)){
            means[mode] = 1/votesum[mode].length*utils.sumArr(votesum[mode])
            for(let v of votesum[mode]){
                if(weights[mode]) weights[mode].push(Math.exp(-Math.pow(means[mode] - v, 2)))
                else weights[mode] = [Math.exp(-Math.pow(means[mode] - v, 2))]
            }

            weights[mode] = utils.normalize(weights[mode])

            for(let i=0; i<votesum[mode].length; i++) {
                weights[mode][i] *= votesum[mode][i]
            }
            temp[mode] = utils.sigmoid(utils.sumArr(weights[mode]), slope, 0)
        }
        this.mapvote_weights = temp
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
    set_weight_parameters(params_dict){
        this.weight_parameter = params_dict
    }
    calculate_map_weight(...params){
        let x = this.neighbor_count
        for(let mode of Object.keys(this.layers)){
            let y = this.mapvote_weights[mode]
            this.map_weight[mode] = params[0]+params[1]*x+params[2]*y+params[3]*x*y+params[4]*x**2+params[5]*y**2
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
    let test = calculate_weights(config)
    //fs.writeFileSync("test.json", JSON.stringify(test, null, 2))
    //console.log(test)
    let maps = initialize_maps(config)
    //console.log(maps[maps.length-3])
    //console.log(maps[0].mapvote_weights)
}

module.exports = { Map, Layer, initialize_maps, get_layers };