const generator = require("./src/generator.js")
const fs = require("fs")
const optimizer = require("./src/optimizer_parallel.js")
const crypto = require("crypto")

function main(){
    let config = require("./config.json")
    if (config["auto_optimize"]){
        let save = JSON.parse(fs.readFileSync("./data/save.json"))
        let check = {}
        check["bioms"] = crypto.createHash("md5").update(JSON.stringify(require("./data/bioms.json"))).digest("hex")
        let c_config = {}
        c_config["biom_spacing"] = config["biom_spacing"]
        c_config["mode_distribution"] = config["mode_distribution"] 
        c_config["use_lock_time_modifier"] = config["use_lock_time_modifier"]
        check["config"] = crypto.createHash("md5").update(JSON.stringify(c_config)).digest("hex")

        if(crypto.createHash("md5").update(JSON.stringify(save)).digest("hex") != crypto.createHash("md5").update(JSON.stringify(check)).digest("hex")){
            console.log(`INFO: relevant data values changed, running optimizer`)
            fs.writeFileSync("./data/save.json", JSON.stringify(check))
            optimizer.start_optimizer_parallel(main)
            return
        }
    }
    console.log("Starting Rota generation")
    console.time("Generation Time")
    for(let i=0; i<config["number_of_rotas"]; i++){
        console.log(`generate rotation ${i+1}/${config["number_of_rotas"]}]`)
        let gen = new generator.Maprota(config)
        let rota = gen.generate_rota()
        fs.writeFileSync(`layer_${i+1}.cfg`, rota.join("\n"))
    }
    console.log("Generation complete")
    console.timeEnd("Generation Time")
}

if (require.main === module) {
    //console.time("Execution Time")
    main()
    //console.timeEnd("Execution Time")
}