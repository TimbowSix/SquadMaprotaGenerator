const generator = require("./src/generator.js")
const fs = require("fs")

function main(){
    let config = require("./config.json")
    for(i=0; i<config["number_of_rotas"]; i++){
        let gen = new generator.Maprota(config)
        let rota = gen.generate_rota()
        fs.writeFileSync(`layer_${i+1}.cfg`, rota.join("\n"))
    }
}

if (require.main === module) {
    //console.time("Execution Time")
    main()
    //console.timeEnd("Execution Time")
}