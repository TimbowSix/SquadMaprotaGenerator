const fs = require("fs");
const config = require("../config.json")
const utils = require("./utils.js")
const data = require("./data.js")
const statistics = require("./statistics.js")

class Maprota {
    constructor(config){
        this.config = config
        this.rotation = []
        this.modes = []
        this.maps = []
        //this.layers = fs.readFileSync("./data/layers.json")
        this.mode_buffer = ""

        this.all_maps = data.initialize_maps(this.config)
        //for optimizer
        this.distribution = 0
    }
    choose_mode(latest_modes = null, custom_pool=null) {
        if (!(latest_modes)) latest_modes = []
        let mode_distribution = structuredClone(this.config["mode_distribution"])
        let pools = Object.keys(mode_distribution["pool_distribution"])
        let weight = Object.values(mode_distribution["pool_distribution"])
        let pool = custom_pool ? custom_pool : utils.choice(pools, weight)
        //let pool = utils.choice(pools, weight)
        if (latest_modes.length != 0){
            let latest = latest_modes.slice(latest_modes.length-mode_distribution["pool_spacing"],latest_modes.length)
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
    choose_layer(layers, weighted=true){
        let weight = null
        if(weighted && this.config["use_vote_weight"]) weight = this.layer_weight(layers)
        return utils.choice(layers, weight)
    }
    layer_weight(layers){
        let votes = []
        for(let layer of layers) votes.push(layer.votes)
        return statistics.convert_mapvote_to_weights(votes, 1)
    }
    av_maps(mode){
        let maps = []

        while (maps.length == 0){
            maps = statistics.getValidMaps(this.all_maps, this.maps.at(-1))
        }

        let valid_maps = []
        for(let map of maps){
            if(mode in map.layers) valid_maps.push(map)
        }
        return valid_maps
    }
    choose_map(maps, mode){
        let weights = []
        for(let map of maps){
            if(mode in map.layers){
                maps.push(map)
                //weights.push(map.map_weight[mode])
                weights.push(map.map_weight)
            }
        }
        //normalize weights?
        return utils.choice(maps, weights)
    }
    add_seeding(){
        let seed_layers = []
        for(let map of this.all_maps){
            if("Seed" in map.layers){
                seed_layers.concat(map.layers["Seed"])
            }
        }
        this.rotation.unshift(this.choose_layer(seed_layers, weighted=false))
    }
    generate_rota(str_output=true){
        let mode = this.choose_mode()
        this.modes.push(mode)
        let maps = this.av_maps(mode)
        let map = this.choose_map(maps, mode)
        let layer = this.choose_layer(map.layers[mode])
        this.rotation.push(layer)
        this.maps.push(map)
        console.log(`start, ${mode}, ${layer.name}, ${map.name}`)
        for(let i=0; i<this.config["number_of_layers"]-1-this.config["seed_layer"]; i++){
            if(this.mode_buffer === "") mode = this.choose_mode(this.modes)
            else mode = this.mode_buffer
            console.log(`mode: ${mode}`)
            maps = this.av_maps(mode)
            if(maps.length === 0){
                this.mode_buffer = mode
                mode = this.choose_mode(custom_pool="main")
                console.log(`mode unavailable, new mode: ${mode}`)
            }else this.mode_buffer = ""

            this.modes.push(mode)
            map = this.choose_map(maps, mode)
            this.maps.push(map)
            layer = this.choose_layer(map.layers[mode])
            this.rotation.push(layer)
            console.log(`mode: ${mode}, ${map.name}, ${layer.name}`)
        }
        if(this.config["seed_layer"] > 0){
            for(let i = 0; i<this.config["seed_layer"]; i++){
                this.add_seeding()
            }
        }
        if(str_output)return this.toString()
        else return this.rotation
    }
    toString(){
        let rota = []
        for(let layer of this.rotation){
            rota.push(layer.name)
        }
        return rota
    }
}

function main(){
    let rota = new Maprota(config)
    let test = rota.generate_rota()
    //let test = []
    //for(let i = 0; i<1000; i++) test.push(rota.choose_mode())
    //console.log(test)
    if (fs.existsSync("./test.txt")) {
        fs.unlinkSync("./test.txt")
    }
    fs.appendFileSync('./test.txt', test.join("\n"))
}

if (require.main === module) {
    console.time("Execution Time")
    main()
    console.timeEnd("Execution Time")
}

module.exports = { Maprota };