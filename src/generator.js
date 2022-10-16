const utils = require("./utils.js")
const data = require("./data.js")
const statistics = require("./statistics.js")
const fs = require("fs")

class Maprota {
    /**
     * setup for maprota generation
     * @param {object} config
     */
    constructor(config){
        this.config = config
        this.rotation = []
        this.modes = []
        this.maps = []
        this.mode_buffer = ""

        this.all_maps = data.initialize_maps(this.config, this.config["use_map_weight"])
        this.weight_params = JSON.parse(fs.readFileSync("./data/weight_params.json"))

    }
    reset(){
        this.rotation = []
        this.modes = []
        this.maps = []
        this.mode_buffer = ""
        for(let map of this.all_maps){
            map.current_lock_time = 0
        }
    }
    /**
     * Selects a random game mode based on the modes in the mode pools and the corresponding probabilities set in the configuration
     * @param {Array} latest_modes optional, necessary for correct distributions
     * @param {string} custom_pool optional, choose which mode pool to use
     * @returns {string} mode string
     */
    choose_mode(latest_modes=null, custom_pool=null) {
        if (!(latest_modes)) latest_modes = []
        let mode_distribution = structuredClone(this.config["mode_distribution"])
        let pools = Object.keys(mode_distribution["pool_distribution"])
        let weight = Object.values(mode_distribution["pool_distribution"])
        weight = utils.normalize(weight)
        let pool = custom_pool ? custom_pool : utils.choice(pools, weight)
        if (latest_modes.length != 0){
            let latest
            if(latest_modes.length<mode_distribution["pool_spacing"]) latest = latest_modes
            else latest = latest_modes.slice(latest_modes.length-mode_distribution["pool_spacing"],latest_modes.length)
            if (latest.some((mode) => !(mode_distribution["pools"]["main"].hasOwnProperty(mode)))){
                pool = "main"
            }
            if(mode_distribution["space_main"] & pool == "main" & mode_distribution["pools"]["main"].hasOwnProperty(latest_modes[latest_modes.length-1])){
                delete mode_distribution["pools"][pool][latest_modes[latest_modes.length-1]]
            }
        }
        let modes = Object.keys(mode_distribution["pools"][pool])
        let chances = utils.normalize(Object.values(mode_distribution["pools"][pool]))
        return utils.choice(modes, chances)
    }
    /**
     * Gets a list of layers and returns a random layer from this list.
     * @param {Array} layers list of layers to choose from
     * @param {boolean} weighted layers weighted by their votes
     * @returns {data.Layer} layer object
     */
    choose_layer(layers, weighted=true){
        let weight = null
        if(weighted && this.config["use_vote_weight"]) weight = this.layer_weight(layers)
        return utils.choice(layers, weight)
    }
    /**
     * randomly chooses a layer of a mode from a specific map
     * @param {data.Map} map map to be chosen from
     * @param {string} mode mode to be chosen from
     * @param {boolean} weighted if random layer probability should be weighted by their layervotes
     * @returns {data.Layer}
     */
    choose_layer_from_map(map, mode, weighted=true){
        let weight = null
        if(weighted && this.config["use_vote_weight"]){
            weight = map.vote_weights_by_mode[mode]
        }
        let layer =  utils.choice(map.layers[mode], weight)
        // Add Layer Locktime
        layer.map.lock_layer(layer)
        return layer
    }
    /**
     * Gets a array of layers, finds their mapvotes and converts them to a Array of weights
     * @param {Array} layers array of layers
     * @returns {Array}
     */
    layer_weight(layers, slope=1, shift=0){
        let votes = []
        for(let layer of layers) votes.push(layer.votes)
        //return statistics.convert_mapvote_to_weights(votes, 1)
        let weights = utils.sigmoidArr(votes, slope, shift)
        return utils.normalize(weights)
    }
    /**
     * returns valid maps for current biom distribution
     * @param {string} current_mode
     * @returns {Array}
     */
    valid_maps(current_mode){
        let maps = []
        while (maps.length == 0){
            maps = statistics.getValidMaps(this.all_maps, this.maps.at(-1), current_mode)

        }
        // decrease layer_locktime
        for(let map of this.all_maps){
            map.decrease_layer_lock_time()
        }

        return maps
    }
    /**
     * gets an array of maps, returns an array with all available maps for given mode
     * @param {Array} maps
     * @param {string} mode
     * @returns {Array}
     */
    av_maps(maps, mode){
        let valid_maps = []
        for(let map of maps){
            if(mode in map.layers) valid_maps.push(map)
        }
        return valid_maps
    }
    /**
     * gets an array of maps, checks if given mode is available for map and draws one map with mapweight probability
     * @param {Array} maps
     * @param {string} mode
     * @returns {data.Map}
     */
    choose_map(maps, mode){
        let weights = []
        let valid_maps = []
        for(let map of maps){
            if(mode in map.layers){ //doppelt? -> av_maps
                valid_maps.push(map)
                //failsave //set weight to 1 if no weight available
                let weight = map.calculate_map_weight(mode, this.weight_params)
                if (!(weight)) {
                    //console.log(`WARNING: map '${map.name}' has undefined map_weight ; this will cause errors in the expected map distribution`)
                    weights.push(1)
                }else weights.push(weight+1)
                //weights.push(map.map_weight+1)
            }
        }
        weights = utils.normalize(weights)
        if(utils.round(utils.sumArr(weights), 4)!= 1) {
            console.log(weights)
        }
        return utils.choice(valid_maps, weights)
    }
    /**
     * Creates a new rotation based on the parameters set in the configuration.
     * Returns the rotation as a list of layers.
     * @param {boolean} str_output defines if output should be an array of layer strings or else layer objects
     * @returns {Array}
     */
    generate_rota(str_output=true, reset=true){
        if(reset) this.reset()
        let mode = this.choose_mode(null, "main")
        this.modes.push(mode)
        let v_maps = this.valid_maps(mode)
        let maps = this.av_maps(v_maps, mode)
        let map = this.choose_map(maps, mode)
        //let layer = this.choose_layer(map.layers[mode])
        let layer = this.choose_layer_from_map(map, mode)
        this.rotation.push(layer)
        this.maps.push(map)
        for(let i=0; i<this.config["number_of_layers"]-1-this.config["seed_layer"]; i++){
            if(this.mode_buffer === "") mode = this.choose_mode(this.modes)
            else mode = this.mode_buffer
            v_maps = this.valid_maps(mode)
            maps = this.av_maps(v_maps, mode)
            if(maps.length === 0){
                this.mode_buffer = mode
                mode = this.choose_mode(null, "main")
            }else this.mode_buffer = ""
            maps = this.av_maps(v_maps, mode)
            this.modes.push(mode)
            map = this.choose_map(maps, mode)
            this.maps.push(map)
            //layer = this.choose_layer(map.layers[mode])
            layer = this.choose_layer_from_map(map, mode)
            this.rotation.push(layer)
        }

        // Add seed layers
        if(this.config["seed_layer"] > 0){
            let seed_maps = []
            for(let i = 0; i<this.config["seed_layer"]; i++){
                if(seed_maps.length <= 0){
                    for(let map of this.all_maps) if("Seed" in map.layers) seed_maps.push(map)
                }
                let seed_map = utils.choice(seed_maps)
                let chosen_layer = this.choose_layer(seed_map.layers.Seed, false)
                this.rotation.unshift(chosen_layer)
                let index = seed_maps.indexOf(seed_map);
                seed_maps.splice(index, 1)
            }
        }
        if(str_output)return this.toString()
        else return this.rotation
    }
    /**
     * returns an array of current maprotation as array of strings
     * @returns {Array}
     */
    toString(){
        let rota = []
        for(let layer of this.rotation){
            rota.push(layer.name)
        }
        return rota
    }
    maps_by_mode(){
        //{mode:{map:count}}
        let map_counts = {}
        for(let layer of this.rotation){
            let map = layer.map.name
            let mode = layer.mode
            if(mode in map_counts){
                if(map in map_counts[mode]){
                    map_counts[mode][map]++
                }else{
                    map_counts[mode][map] = 1
                }
            }else{
                map_counts[mode] = {}
                map_counts[mode][map] = 1
            }
        }
        return map_counts
    }
}


if (require.main === module) {
    let config = data.build_config()
    let rota = new Maprota(config)
    console.time("Execution Time")
    rota.generate_rota()
    //console.log(rota.maps_by_mode())
    console.timeEnd("Execution Time")
}

module.exports = { Maprota };
