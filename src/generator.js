let fs = require("fs");
let config = require("../config.json")
let utils = require("./utils.js")

latest_modes = []

class Maprota {
    constructor(config){
        this.config = config
        this.rotation = []
        this.modes = []
        this.layers = fs.readFileSync("./data/layers.json")
    }
    choose_mode(latest_modes = []) {
        let mode_distribution = structuredClone(this.config["mode_distribution"])
        let pools = Object.keys(mode_distribution["pool_distribution"])
        let weight = Object.values(mode_distribution["pool_distribution"])
        let pool = utils.weightedChoice(pools, weight)
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
        return utils.weightedChoice(modes, weight=chances)
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

console.time("Execution Time")
main()
console.timeEnd("Execution Time")