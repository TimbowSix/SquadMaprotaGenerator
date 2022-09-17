const fs = require('fs')
const statistics = require("./statistics.js")
const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;
const utils = require("./utils.js")


function initialize_maps(config, use_map_weights=true){
    let layers
    if (config["update_layers"]){
        layers = get_layers()
        fs.writeFileSync("./data/layers.json", JSON.stringify(layers, null, 2))
    }else{
        layers = require("../data/layers.json")
    }
    let bioms = require("../data/bioms.json")
    let distances = statistics.getAllMapDistances(bioms)
    let maps = []
    
    let modes = new Set()
    for (let [map_name, biom_values] of Object.entries(bioms)) {
        // skip map if no layers available
        if (!(map_name in layers)) continue

        let map = new Map(map_name, biom_values, distances[map_name])
        for(let mode of Object.keys(layers[map_name])){
            for(let layer of layers[map_name][mode]){
                let l = new Layer(layer["name"], mode, map, layer["votes"])
                map.add_layer(l)
            }
        }

        map.lock_time = config["biom_spacing"]
        //pre-calculate layervote weights
        map.calculate_vote_weights_by_mode(config["layervote_slope"], config["layervote_shift"])
        maps.push(map)

        
        for(let mode in map.layers) modes.add(mode)
    }
    //check if for every mode in config is at least one map available
    let config_modes = []
    for(let pool in config["mode_distribution"]["pools"]) for(let mode in config["mode_distribution"]["pools"][pool]) if(config["mode_distribution"]["pools"][pool][mode]>0) config_modes.push(mode)
    for(let mode of config_modes){
        if(!(modes.has(mode))) throw Error(`No maps available for mode '${mode}'.\nMake sure that the probability of the mode is set to 0 or remove this mode if you don't intend to use it`)
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
        map.set_layer_by_pools(config) //redundant?
        map.add_mapvote_weights(config["mapvote_slope"], config["mapvote_shift"])
    }

    // normalize mapvote weights by mode
    let mode_probs = {} //{mode:{map:prob}}
    for(let map of maps){
        for(let mode in map.mapvote_weights){
            if(mode in mode_probs) mode_probs[mode][map.name] = map.mapvote_weights[mode]
            else mode_probs[mode] = {[map.name]:map.mapvote_weights[mode]}
        }
    }

    for(let mode in mode_probs){
        let mode_weights = Object.values(mode_probs[mode])
        mode_weights = utils.normalize(mode_weights)

        let mode_maps = Object.keys(mode_probs[mode])
        for(let i=0; i<mode_maps.length; i++){
            mode_probs[mode][mode_maps[i]] = mode_weights[i]
        }
    }

    for(let map of maps){
        let weights = {}
        for(let mode in map.layers){
            weights[mode] = mode_probs[mode][map.name]
        }
        map.mapvote_weights = weights
    }

    if(config["save_expected_map_dist"]){
        let map_dist = {}
        for(let map of maps){
            if(!(map.name in map_dist)){
                map_dist[map.name] = {}
            }
            for(let mode in mode_probs){
                if(Object.keys(mode_probs[mode]).includes(map.name)){
                    if (map.name in map_dist) map_dist[map.name][mode] = mode_probs[mode][map.name]
                    else map_dist[map.name] = {[mode]:mode_probs[mode][map.name]}
                }
            }
        }
        fs.writeFileSync("./data/current_map_dist.json",JSON.stringify(map_dist, null, 2))
    }
    //check for settings feasibility 

    //get main pool an intermediate pool modes 
    let tempModes = Object.keys(config["mode_distribution"]["pools"]["main"])
    tempModes = tempModes.concat(Object.keys(config["mode_distribution"]["pools"]["intermediate"]))
    tempModes = tempModes.concat(Object.keys(config["mode_distribution"]["pools"]["rest"]))

    //calc max locktime from dist  
    for(let mode in mode_probs){
        for(let map of maps){
            if(Object.keys(mode_probs[mode]).includes(map.name)){
                let pTemp = 0
                for(let neighbor of map.neighbors){
                    if(Object.keys(mode_probs[mode]).includes(neighbor.name) && neighbor.name != map.name){
                        pTemp += Math.pow((1-mode_probs[mode][neighbor.name]), config["biom_spacing"])
                    }
                }
                pTemp = mode_probs[mode][map.name] * (1+pTemp)
                pTemp = 1/pTemp
                if(pTemp < config["biom_spacing"]){
                    //max lock time smaller than config lock time 
                    map.lock_time_modifier[mode] = 1
                }else{
                    map.lock_time_modifier[mode] = 0
                }
            }
        }
    }


    //calculate cluster overlap
    function parse_neighbors(arr){
        let out = []
        for(let i of arr) out.push(i.name)
        out.sort()
        return out
    }
    for(let map of maps) {
        let clusters = []
        //let own = parse_neighbors(map.neighbors).join()
        for(let neighbor of map.neighbors){
            if(neighbor===map) continue
            let nc = parse_neighbors(neighbor.neighbors).join()
            let contains = false
            for(let cluster of clusters) if(cluster==nc) contains=true
            if (!contains) clusters.push(nc)
        }
        map.cluster_overlap = +!(map.neighbor_count-1-clusters.length)
    }

    // initially calculate actual weights
    if(use_map_weights){
        for(let map of maps){
            for(let mode in map.layers){
                map.calculate_map_weight(mode, config["weight_params"][mode])
            }
        }
    }else{
        for(let map of maps){
            for(let mode in map.layers){
                let params = []
                for(i=0;i<config["weight_params"][mode].length; i++) params.push(0) //compatibility 
                map.calculate_map_weight(mode, params)
            }
        }
    }

    return maps
}

class Map{
    constructor(name, bioms, distances){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = {}
        this.mapvote_weights = {}
        this.total_probabilities = {} //redundant?
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        this.layer_by_pools = {} // redundant?
        this.target_map_dist = {} //redundant?
        this.lock_time_modifier = {}
        //for optimizer
        this.distribution = 0
        this.mode_groups = [] //TODO kann weg wenn es keiner mehr braucht //also kann weg?
        this.vote_weights_by_mode = {}
        this.weight_parameter = {}

        this.cluster_overlap

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
    add_mapvote_weights(slope=1, shift=0){
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
            temp[mode] = utils.sigmoid(utils.sumArr(weights[mode]), slope, shift)
        }
        this.mapvote_weights = temp
    }

    // redundant?
    set_layer_by_pools(config){
        this.layer_by_pools = get_mode_to_pools_dict(config)
    }
    /**
     * calculate layervotes to weights
     * @param {float} sigmoid_slope 
     * @param {float} sigmoid_shift 
     */
    calculate_vote_weights_by_mode(sigmoid_slope=1, sigmoid_shift=0){
        if (Object.entries(this.layers).length === 0) throw Error(`map '${this.name}' has no layers to calculate weights`)

        for(let mode of Object.keys(this.layers)){
            let votes = []
            for(let layer of this.layers[mode]) votes.push(layer.votes)
            let weights = utils.normalize(statistics.sigmoidArr(votes, sigmoid_slope, sigmoid_shift))
            this.vote_weights_by_mode[mode] = weights
        }
    }
    // wa do dis do? wa is dis for?
    set_weight_parameters(params_dict){
        this.weight_parameter = params_dict
    }

    /**
     * calculates mapweight for given mode based on given params
     * @param {string} mode Mode for which the weight is to be calculated
     * @param {[float]} params Array of parameters for the calculation
     */
    calculate_map_weight(mode, params){
        if(!(params)) return
        let x = this.neighbor_count - 1
        let y = this.mapvote_weights[mode]
        this.map_weight[mode] = params[0] + params[1]*x + 10*params[2]*y + params[3]*x**2 + 10*params[4]*x*y + 100*params[5]*y**2
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

        if(map in maps){
            if(mode in maps[map]){
                maps[map][mode].push(data)
            }else{
                maps[map][mode] = [data]
            }
        }else{
            maps[map] = {[mode]: [data]}
        }
    }
    return maps
}

function save_mapweights(){
    let config = require("../config.json")
    let maps = initialize_maps(config)
    let weights = {}
    for(let map of maps){
        weights[map.name] = map.map_weight
    }
    fs.writeFileSync("test.json", JSON.stringify(weights, null, 2))
}

// Test Stuff here
if (require.main === module) {
    //save_mapweights()
    let config = require("../config.json")
    let maps = initialize_maps(config)
    //console.log(maps[1].map_weight)
}

module.exports = { Map, Layer, initialize_maps, get_layers };