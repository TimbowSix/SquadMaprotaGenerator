const generator = require("./src/generator.js")
const fs = require("fs")
const optimizer = require("./src/optimizer_parallel.js")
const crypto = require("crypto")
const data = require("./src/data.js")


function main(){
    let config = require("./config.json")
    if (config["auto_optimize"] && data.check_changes()){
        console.log(`INFO: relevant data values changed, running optimizer`)
        optimizer.start_optimizer_parallel(main)
        return
    }
    console.log("Starting Rota generation")
    console.time("Generation Time")
    for(let i=0; i<config["number_of_rotas"]; i++){
        console.log(`generate rotation ${i+1}/${config["number_of_rotas"]}`)
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