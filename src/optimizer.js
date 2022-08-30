let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config){
        this.config = config
        this.config['seed_layer'] = 0
        this.config['number_of_rotas'] = 1
        this.config['number_of_layers'] = 50000
        this.config['use_vote_weight'] = false

        this.generator = new gen.Maprota(this.config)

        //load existing values
        this.map_weights = fs.readFileSync("../data/mapweights.json") //TODO umstellen auf drei verschiedenen Weights Typen

    }
}

op = new Optimizer(config)