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

        this.all_maps = data.initialize_maps()
        //for optimizer
        this.distribution = 0
    }
    choose_mode(latest_modes = null) {
        if (!(latest_modes)) latest_modes = []
        let mode_distribution = structuredClone(this.config["mode_distribution"])
        let pools = Object.keys(mode_distribution["pool_distribution"])
        let weight = Object.values(mode_distribution["pool_distribution"])
        let pool = utils.choice(pools, weight)
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
    choose_map(mode){
        let valid_maps = []

        while (valid_maps.length == 0){
            valid_maps = statistics.getValidMaps(this.maps)
        }

        


    }
}

function main(){
    test = []
    rota = new Maprota(config)
    rnd = rota.choose_mode()
    test.push(rnd)
    for (let i = 0 ; i<100000; i++){
        rnd = rota.choose_mode(latest_modes=test)
        test.push(rnd)
    }
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