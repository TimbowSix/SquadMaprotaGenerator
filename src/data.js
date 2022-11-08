const fs = require('fs')
const statistics = require("./statistics.js")
const utils = require("./utils.js")
const crypto = require("crypto");
const fetch = require("sync-fetch");

/**
 * Initializing all available maps for the given configuration
 * @param {object} config
 * @returns {[Map]}
 */
function initialize_maps(config){
    let layers
    if (config["update_layers"]){
        layers = get_layers()
        fs.writeFileSync("./data/layers.json", JSON.stringify(layers, null, 2))
    }else{
        layers = JSON.parse(fs.readFileSync("./data/layers.json"))
    }
    let bioms = JSON.parse(fs.readFileSync("./data/bioms.json"))
    bioms = parse_map_size(bioms)
    let distances = statistics.getAllMapDistances(bioms)
    let maps = []

    let used_modes = ["Seed"]
    for(let pool in config.mode_distribution.pools){
        for(let mode in config.mode_distribution.pools[pool]){
            used_modes.push(mode)
        }
    }
    let team_layers
    if (config["update_teams"]){
        team_layers = get_teams()
        fs.writeFileSync("./data/layers_teams.json", JSON.stringify(team_layers, null, 2))
    }else{
        team_layers = JSON.parse(fs.readFileSync("./data/layers_teams.json"))
    }

    for (let [map_name, biom_values] of Object.entries(bioms)) {
        // skip map if unused / no layers available
        if(!(config["maps"].includes(map_name))){
            continue
        }
        if ((map_name in layers)){
            //layer for map available
            let map = new Map(map_name, biom_values, distances[map_name])
            map.sigmoid_values = {
                "mapvote_slope": config["mapvote_slope"],
                "mapvote_shift":config["mapvote_shift"],
                "layervote_slope":config["layervote_slope"],
                "layervote_shift":config["layervote_shift"]
            }
            for(let mode of Object.keys(layers[map_name])){
                //skip mode if not used
                if(!(used_modes.includes(mode))){
                    continue
                }
                for(let layer of layers[map_name][mode]){
                    let l = new Layer(layer["name"], mode, map, layer["votes"])
                    if(!(team_layers[layer.name])){
                        console.log(layer.name)
                        continue
                    }
                    l.teamOne = team_layers[layer.name]["teamOne"]
                    l.teamTwo = team_layers[layer.name]["teamTwo"]
                    l.lock_time = config["layer_locktime"]
                    map.add_layer(l)
                }
            }

            map.lock_time = config["biom_spacing"]

            //map.layer_locktime = config["layer_locktime"]

            //pre-calculate layervote weights
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
        let current_dist = get_dist(maps)
        fs.writeFileSync("./data/current_map_dist.json",JSON.stringify(current_dist, null, 2))
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

/**
 * calculating the expected distribution for every given map
 * @param {[Map]} maps
 * @returns
 */
function get_dist(maps){
    const weight_params = JSON.parse(fs.readFileSync("./data/weight_params.json"))
    let current_map_dist = {}
    for(let map of maps){
        current_map_dist[map.name] = {}
        for(let mode in map.layers){
            current_map_dist[map.name][mode] = map.mapvote_weights[mode] / map.mapvote_weight_sum[mode]
        }
    }
    return current_map_dist
}

/**
 * parsing the mapsize in km² to a value between 0 and 1 for every given map
 * @param {object} bioms
 * @returns
 */
function parse_map_size(bioms){
    let max = 0
    let min = Number.MAX_SAFE_INTEGER
    for(let map in bioms){
        if(bioms[map][0] > max) max = bioms[map][0]
        if (bioms[map][0] < min) min = bioms[map][0]
    }
    for(let map in bioms){
        bioms[map][0] = (bioms[map][0]-min)/(max-min)
    }
    return bioms
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
        //this.layer_locktime = 0
        //this.locked_layers = [] //[{"locktime": 1, "layer": layer}]
        this.sigmoid_values = {
            "mapvote_slope": 1,
            "mapvote_shift": 0,
            "layervote_slope": 1,
            "layervote_shift": 0
        }
    }

    /**
     * returns all currently for this map available modes
     * @returns {[string]}
     */
    av_modes(){
        let modes = []
        for(let mode in this.layers){
            if(this.layers[mode].some((val) => val.current_lock_time <= 0)){
                modes.push(mode)
            }
        }
        return modes
    }

    /**
     * returns an array of all available layers of a specific mode
     * a layer is available if current_lock_time <= 0
     * @param {string} mode
     * @returns {[Layer]}
     */
    av_layers(mode){
        let layers = []
        for(let layer of this.layers[mode]){
            if(layer.current_lock_time <= 0){
                layers.push(layer)
            }
        }
        return layers
    }

    /**
     * Locking a layer for the set layer_locktime, removing it from the available layers
     * @param {Layer} layer
     * @param {Number} locktime optional
     */
    /*
    lock_layer(layer, locktime){
        let lt = this.layer_locktime
        if (locktime){
            lt = locktime
        }
        this.locked_layers.push({"locktime": lt, "layer":layer})
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
    */
    /**
     * calculating new mapvote weight sum for a specific mode
     * @param {string} mode
     */
    new_weight(mode){
        const old_weight = this.mapvote_weights[mode]
        this.add_mapvote_weights(mode)
        this.mapvote_weight_sum[mode] -= old_weight
        this.mapvote_weight_sum[mode] += this.mapvote_weights[mode]
    }

    /**
     * resetting the layer locktime for every layer, making every currently locked layer available again.
     */
    reset_layer_locktime(){
        for(let mode in this.layers){
            for(let layer of this.layers[mode]){
                layer.lock_time = 0
            }
        }
        this.new_weight()
    }

    /**
     * decreasing the layer locktime by one for every locked layer of this map
     */
    decrease_layer_lock_time(){
        for(let mode in this.layers){
            for(let layer of this.layers[mode]){
                layer.decrease_lock_time()
            }
        }
    }

    /**
     * adding a new layer as a property of this map
     * @param {Layer} layer
     */
    add_layer(layer){
        if (!(layer.mode in this.layers)) this.layers[layer.mode] = [layer]
        else this.layers[layer.mode].push(layer)
        let slope = this.sigmoid_values["layervote_slope"]
        let shift = this.sigmoid_values["layervote_shift"]
        layer.calculate_vote_weight(slope, shift)
    }

    /**
     * decreasing the locktime by one
     */
    decrease_lock_time(){
        if(this.current_lock_time >= 1){
            this.current_lock_time--;
        }
    }

    /**
     * setting the current locktime of this map to the saved standard lock time
     */
    update_lock_time(){
        this.current_lock_time = this.lock_time
    }

    /**
     * Calculating the mapvote weight for a specific mode.
     * Calculating for every for this map available mode, if no mode given.
     * @param {string} mode optional
     * @returns
     */
    add_mapvote_weights(mode){
        //let modes = Object.keys(this.layers)
        let modes = this.av_modes()
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
            for(let layer of this.av_layers(mode)) votes.push(layer.votes)
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
        for(let mode in temp){
            this.mapvote_weights[mode] = temp[mode]
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
        this.teamOne
        this.teamTwo
        this.vote_weight = 0
        this.locktime = 0
        this.current_lock_time = 0
    }

    /**
     * calculating layer vote weight with sigmoid function
     * @param {*} slope sigmoid slope
     * @param {*} shift sigmoid shift
     */
    calculate_vote_weight(slope, shift){
        this.vote_weight = utils.sigmoid(this.votes, slope, shift)
    }

    /**
    * decreasing locktime by one
    */
    decrease_lock_time(){
        if(this.current_lock_time >= 1){
            this.current_lock_time--;
            if(this.current_lock_time <= 0){
                this.map.new_weight()
            }
        }
    }

    /**
     * setting current locktime of this map to the saved standard lock time
     * setting current locktime to custom locktime if given
     * @param {number} locktime optional
     */
    update_lock_time(locktime){
        if(locktime){
            this.current_lock_time = locktime
        }else{
            this.current_lock_time = this.lock_time
        }
        this.map.new_weight()
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

/**
 * retrieves layers, votes and maps from a fetched input file
 * @returns {object}
 */
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

function get_teams(){
    const config = JSON.parse(fs.readFileSync("./config.json"))
    let data = get_data(config.team_api_url)
    let layers = {}
    for(let layer of data){
        layers[layer.id] = {"teamOne": layer.teamOne, "teamTwo": layer.teamTwo}
    }
    return layers
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

/**
 * removing unavailable modes/maps und recalculating probabilities if necessary
 * @returns
 */
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

/**
 * building a config object based on the config.json and possible overwrites
 * @param {boolean} warning display a warning if there is anything unavailable
 * @returns {object}
 */
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

/**
 * checking if important changes in the configuration have been made
 * @returns {boolean}
 */
function check_changes(){
    fix_unavailables()
    let save = JSON.parse(fs.readFileSync("./data/save.json"))
    let config = JSON.parse(fs.readFileSync("./config.json"))
    let check = {}
    check["bioms"] = crypto.createHash("md5").update(JSON.stringify(JSON.parse(fs.readFileSync("./data/bioms.json")))).digest("hex")
    let c_config = {}
    c_config["biom_spacing"] = config["biom_spacing"]
    c_config["use_lock_time_modifier"] = config["use_lock_time_modifier"]
    c_config["layer_locktime"] = config["layer_locktime"]
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

/**
 * fetching a given url for a json file.
 * @param {string} url
 * @returns
 */
function get_data(url){
    return fetch(url).json()
}

// Test Stuff here
if (require.main === module) {
    console.log(get_teams()["Skorpo_AAS_v1"])
}

module.exports = { Map, Layer, initialize_maps, get_layers, check_changes, build_config, get_dist };
