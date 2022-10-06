const fs = require('fs')
const statistics = require("./statistics.js")
const utils = require("./utils.js")
const crypto = require("crypto");
const fetch = require("sync-fetch")

function initialize_maps(config, use_map_weights=true){
    let layers
    if (config["update_layers"]){
        layers = get_layers()
        fs.writeFileSync("./data/layers.json", JSON.stringify(layers, null, 2))
    }else{
        layers = require("../data/layers.json")
    }
    let bioms = require("../data/bioms.json")
    bioms = parse_map_size(bioms)
    let distances = statistics.getAllMapDistances(bioms)
    let maps = []
    
    let modes = new Set()
    for (let [map_name, biom_values] of Object.entries(bioms)) {
        // error map if no layers available
        if (!(map_name in layers)) throw Error(`No layers available for map '${map_name}'`)

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
        map.cluster_overlap = (map.neighbor_count-1-clusters.length)
    }

    // initially calculate actual weights
    let weight_params = require("../data/weight_params.json")
    if(use_map_weights){
        for(let map of maps){
            for(let mode in map.layers){
                map.calculate_map_weight(mode, weight_params[mode])
            }
        }
    }else{
        for(let map of maps){
            for(let mode in map.layers){
                let params = []
                for(i=0;i<weight_params[mode].length; i++) params.push(0) //compatibility 
                map.calculate_map_weight(mode, params)
            }
        }
    }

    // TODO pro mode?
    //check for settings feasibility 

    //get main pool an intermediate pool modes 
    let tempModes = Object.keys(config["mode_distribution"]["pools"]["main"])
    tempModes = tempModes.concat(Object.keys(config["mode_distribution"]["pools"]["intermediate"]))
    tempModes = tempModes.concat(Object.keys(config["mode_distribution"]["pools"]["rest"]))


    //calc max locktime from dist  
    for(let mode in mode_probs){
        for(let map of maps){
            if(Object.keys(mode_probs[mode]).includes(map.name)){
                if(map.cluster_overlap > 0 && config["use_lock_time_modifier"]){
                    map.lock_time_modifier[mode] = 1
                }else{
                    map.lock_time_modifier[mode] = 0
                }
            }
        }
    }

    return maps
}

function parse_map_size(bioms){
    let max = 0
    let min = Number.MAX_SAFE_INTEGER
    for(let map in bioms){
        if(bioms[map][0] > max) max =bioms[map][0]
        if (bioms[map][0] < min) min =bioms[map][0]
    }
    for(let map in bioms){
        bioms[map][0] = (bioms[map][0]-min)/(max-min)
    }
    return bioms
}

class Map{
    constructor(name, bioms, distances){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = {}
        this.mapvote_weights = {}
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        this.lock_time_modifier = {}
        this.vote_weights_by_mode = {}
        //for optimizer
        this.distribution = 0
        
        this.cluster_overlap = 0 //pro mode?

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
            let weights = utils.normalize(utils.sigmoidArr(votes, sigmoid_slope, sigmoid_shift))
            this.vote_weights_by_mode[mode] = weights
        }
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
    let data = get_data(theUrl)
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

function check_changes(){
    let save = JSON.parse(fs.readFileSync("./data/save.json"))
    let config = JSON.parse(fs.readFileSync("./config.json"))
    let check = {}
    check["bioms"] = crypto.createHash("md5").update(JSON.stringify(JSON.parse(fs.readFileSync("./data/bioms.json")))).digest("hex")
    let c_config = {}
    c_config["biom_spacing"] = config["biom_spacing"]
    c_config["mode_distribution"] = config["mode_distribution"] 
    c_config["use_lock_time_modifier"] = config["use_lock_time_modifier"]
    check["config"] = crypto.createHash("md5").update(JSON.stringify(c_config)).digest("hex")
    if (crypto.createHash("md5").update(JSON.stringify(save)).digest("hex") != crypto.createHash("md5").update(JSON.stringify(check)).digest("hex")){
        fs.writeFileSync("./data/save.json", JSON.stringify(check))
        return true
    }else return false
}

function get_data(url){
    return fetch(url).json()
}

// Test Stuff here
if (require.main === module) {
    console.log(get_data("https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req"))
}

module.exports = { Map, Layer, initialize_maps, get_layers, check_changes };