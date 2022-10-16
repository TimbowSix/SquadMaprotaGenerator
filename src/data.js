const fs = require('fs')
const statistics = require("./statistics.js")
const utils = require("./utils.js")
const crypto = require("crypto");
const fetch = require("sync-fetch");

function initialize_maps(config){
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

    let used_modes = ["Seed"]
    for(let pool in config.mode_distribution.pools){
        for(let mode in config.mode_distribution.pools[pool]){
            used_modes.push(mode)
        }
    }

    for (let [map_name, biom_values] of Object.entries(bioms)) {
        // skip map if unused / no layers available
        if(!(config["maps"].includes(map_name))){
            continue
        }
        if ((map_name in layers)){
            //layer for map available
            let map = new Map(map_name, biom_values, distances[map_name])
            for(let mode of Object.keys(layers[map_name])){
                //skip mode if not used
                if(!(used_modes.includes(mode))){
                    continue
                }
                for(let layer of layers[map_name][mode]){
                    let l = new Layer(layer["name"], mode, map, layer["votes"])
                    map.add_layer(l)
                }
            }

            map.lock_time = config["biom_spacing"]

            map.layer_locktime = config["layer_locktime"]

            map.sigmoid_values = {
                "mapvote_slope": config["mapvote_slope"],
                "mapvote_shift":config["mapvote_shift"],
                "layervote_slope":config["layervote_slope"],
                "layervote_shift":config["layervote_shift"]
            }

            //pre-calculate layervote weights
            map.calculate_vote_weights_by_mode()
            maps.push(map)

        }else{
            console.log(`WARNING: No layers available for map '${map_name}'`)
        }

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
    // add mapvote weights for every map
    // save mopvote weight sum for every mode
    let mapvote_weight_sum = {} //{mode: sum}
    for(let map of maps){
        map.add_mapvote_weights()
        for(let mode in map.layers){
            if(mode in mapvote_weight_sum){
                mapvote_weight_sum[mode] += map.mapvote_weights[mode]
            }else{
                mapvote_weight_sum[mode] = map.mapvote_weights[mode]
            }
        }
        map.mapvote_weight_sum = mapvote_weight_sum
    }
    if(config["save_expected_map_dist"]){
        const weight_params = JSON.parse(fs.readFileSync("./data/weight_params.json"))
        let current_map_dist = {}
        for(let map of maps){
            current_map_dist[map.name] = {}
            for(let mode in map.layers){
                current_map_dist[map.name][mode] = map.calculate_map_weight(mode, weight_params)
            }
        }
        fs.writeFileSync("./data/current_map_dist.json",JSON.stringify(current_map_dist, null, 2))
    }

    //calculate cluster overlap
    const parse_neighbors = (arr) => {
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

    //calc max locktime from dist
    let mode_probs = {} //{mode:{map:prob}}
    for(let map of maps){
        for(let mode in map.mapvote_weights){
            if(mode in mode_probs) mode_probs[mode][map.name] = map.mapvote_weights[mode]
            else mode_probs[mode] = {[map.name]:map.mapvote_weights[mode]}
        }
    }

    for(let mode in mode_probs){
        for(let map of maps){
            map.lock_time_modifier[mode] = 0
            if(Object.keys(mode_probs[mode]).includes(map.name) && map.cluster_overlap > 0 && config["use_lock_time_modifier"]){
                map.lock_time_modifier[mode] = 1
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

//redundant?
function normalize_mapvote_weights(maps, save_expected_map_dist=false){
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

    if(save_expected_map_dist){
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
}

class Map{
    constructor(name, bioms, distances){
        this.name = name
        // layers = {mode:[layer]}
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
        // Layer Locktime
        this.mapvote_weight_sum = {}
        this.layer_locktime = 0
        this.locked_layers = [] //[{"locktime": 1, "layer": layer}]
        this.sigmoid_values = {}
    }
    lock_layer(layer){
        this.locked_layers.push({"locktime": this.layer_locktime, "layer":layer})
        const index = this.layers[layer.mode].indexOf(layer)
        if(index>-1){
            this.layers[layer.mode].splice(index, 1)
            if(this.layers[layer.mode].length <1){
                delete this.layers[layer.mode]
            }
            this.new_weight(layer.mode)
        }else{
            console.log(`WARNING: Layer ${layer.name} not found in Map ${this.name}`)
        }
    }
    new_weight(mode){
        const old_weight = this.mapvote_weights[mode]
        this.add_mapvote_weights(mode)
        this.mapvote_weight_sum[mode] - old_weight + this.mapvote_weights[mode]
        this.calculate_vote_weights_by_mode(mode)
    }
    reset_layer_locktime(){
        for(let locked_layer of this.locked_layers){
            locked_layer.locktime = 0
        }
        this.decrease_layer_lock_time()
    }
    decrease_layer_lock_time(){
        let valid = []
        for(let locked_layer of this.locked_layers){
            locked_layer.locktime -= 1
            if(locked_layer.locktime <=0){
                valid.push(locked_layer)
            }
        }
        for(let locked_layer of valid){ //remove layer from locked list
            const index = this.locked_layers.indexOf(locked_layer)
            if(index >-1){
                this.locked_layers.splice(index, 1)
            }
            if(locked_layer.layer.mode in this.layers){
                this.layers[locked_layer.layer.mode].push(locked_layer.layer)
            }else{
                this.layers[locked_layer.layer.mode] = [locked_layer.layer]
            }
        }
        if(valid.length > 0){
            let modes = new Set
            for(let locked_layer of valid){
                modes.add(locked_layer.layer.mode)
            }
            for(let mode of modes){
                this.new_weight(mode)
            }

        }
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
    add_mapvote_weights(mode){
        let modes = Object.keys(this.layers)
        if(mode){
            if(!(mode in this.layers)){
                return
            }
            modes = [mode]
        }
        let slope = this.sigmoid_values["mapvote_slope"]
        let shift = this.sigmoid_values["mapvote_shift"]
        if(this.layers.length == 0) {
            console.log(`No layers added to map ${this.name}, could not calculate mapvote_weights!`);
            return
        }
        let votesum = {}
        for(let mode of modes){
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
    calculate_vote_weights_by_mode(mode){
        let modes = Object.keys(this.layers)
        if(mode){
            if(!(mode in this.layers)){
                return
            }
            modes = [mode]
        }
        //if (Object.entries(this.layers).length === 0) throw Error(`map '${this.name}' has no layers to calculate weights`)
        let sigmoid_slope = this.sigmoid_values["layervote_slope"]
        let sigmoid_shift = this.sigmoid_values["layervote_shift"]

        for(let mode of modes){
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
        let y = this.mapvote_weights[mode] / this.mapvote_weight_sum[mode]
        let weight = params[0] + params[1]*x + 10*params[2]*y + params[3]*x**2 + 10*params[4]*x*y + 100*params[5]*y**2
        return weight
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
    let config = JSON.parse(fs.readFileSync("./config.json"))
    let data = get_data(config.layer_vote_api_url)
    let layers = data["AxisLabels"]
    let upvotes = data["DataSet"][0]
    let downvotes = data["DataSet"][1]
    let availability = data["DataSet"][2]
    let maps = {}
    for(let i=0; i<layers.length; i++){
        if (availability[i] > 0) continue
        let layer = layers[i]
        layer = utils.formatLayer(layer)
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

function fix_unavailables(){
    let config = JSON.parse(fs.readFileSync("./config.json"))
    if(!(config["fix_unavailables"])) return


    // mode availability check
    let maps = initialize_maps(config)
    let av_modes = []
    for(let map of maps){
        for(let mode in map.layers){
            if(map.layers[mode].length>0) av_modes.push(mode)
        }
    }
    for(let pool in config["mode_distribution"]["pools"]){
        let pool_sum = 0
        for(let mode in config["mode_distribution"]["pools"][pool]){
            if(!(av_modes.includes(mode))){
                config["mode_distribution"]["pools"][pool][mode] = 0
            }
            pool_sum += config["mode_distribution"]["pools"][pool][mode]
        }
        if(pool_sum === 0){
            config["mode_distribution"]["pool_distribution"][pool] = 0
        }
    }
    fs.writeFileSync("./data/mode_distribution_overwrite.json", JSON.stringify(config["mode_distribution"], null, 2))


    // map availability check
    const layers = JSON.parse(fs.readFileSync("./data/layers.json"))
    let av_maps = []
    for(let map of maps){
        if(Object.keys(map.layers).length > 0) av_maps.push(map.name)
    }
    av_maps.sort()
    fs.writeFileSync("./data/maps_overwrite.json", JSON.stringify(av_maps, null, 2))


}

function build_config(warning=false){
    let config = JSON.parse(fs.readFileSync("./config.json"))
    let maps_overwrite = JSON.parse(fs.readFileSync("./data/maps_overwrite.json"))
    let o_maps = config["maps"]
    o_maps.sort()
    if(JSON.stringify(o_maps) != JSON.stringify(maps_overwrite)){
        if(warning) console.log("WARNING: Some maps are unavailable")
        config["maps"] = maps_overwrite
    }

    let mode_distribution_overwrite = JSON.parse(fs.readFileSync("./data/mode_distribution_overwrite.json"))
    if(JSON.stringify(mode_distribution_overwrite) != JSON.stringify(config["mode_distribution"])){
        if(warning) console.log("WARNING: Some modes are unavailable")
        config["mode_distribution"] = mode_distribution_overwrite
    }
    return config
}

function check_changes(){
    fix_unavailables()
    let save = JSON.parse(fs.readFileSync("./data/save.json"))
    let config = JSON.parse(fs.readFileSync("./config.json"))
    let check = {}
    check["bioms"] = crypto.createHash("md5").update(JSON.stringify(JSON.parse(fs.readFileSync("./data/bioms.json")))).digest("hex")
    let c_config = {}
    c_config["biom_spacing"] = config["biom_spacing"]
    c_config["use_lock_time_modifier"] = config["use_lock_time_modifier"]
    check["config"] = crypto.createHash("md5").update(JSON.stringify(c_config)).digest("hex")

    let maps = JSON.parse(fs.readFileSync("./data/maps_overwrite.json"))
    maps.sort()
    check["maps_overwrite"] = crypto.createHash("md5").update(JSON.stringify(maps)).digest("hex")

    let modes = JSON.parse(fs.readFileSync("./data/mode_distribution_overwrite.json"))
    check["modes_overwrite"] = crypto.createHash("md5").update(JSON.stringify(modes)).digest("hex")

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
    //fix_unavailables()
    let maps = JSON.parse(fs.readFileSync("./data/maps_overwrite.json"))
    console.log(JSON.stringify(maps))
}

module.exports = { Map, Layer, initialize_maps, get_layers, check_changes, build_config };
