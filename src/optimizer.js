let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config){
        this.config = config;
        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

        this.generator = new gen.Maprota(this.config);

        this.wUni = 1/this.generator.all_maps.length;

        //load existing values
        this.mapWeights = fs.readFileSync("../data/mapweights.json"); //TODO umstellen auf drei verschiedenen Weights Typen
        this.delta = fs.readFileSync("../data/delta.json")
        
        //init maps
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].distribution = 0;
        }
        //generate rota
        //this.generator. TODO WO function
        this.update_dist();
        this.currentMin = this.calc_current_norm();
    }

    optimize_recursive(currentIndex, lowestDelta, minChanged){
        if(this.delta <= lowestDelta){
            return
        }
        
        this.generator.all_maps[currentIndex].map_weight += this.delta;
        //this.generator.generate //TODO 
        this.update_dist();
        cMin = this.calc_current_norm();
        if(this.currentMin > cMin){
            
        }
    }

    calc_current_norm(){
        temp = 0
        for(let i=0;i<this.generator.all_maps.length;i++){
            wTemp += Math.pow(this.generator.all_maps[i].distribution - this.wUni, 2)
        }
    }
    update_dist(){
        tempSum = 0
        //reset dist
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].dist = 0;
        }
        //update
        for(let i=0;i<this.generator.maps.length;i++){
            this.generator.maps[i].dist++;
            tempSum++;
        }
        //normalize
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].dist /= tempSum;
        }
    }
    saveMapWeights(){
        fs.writeFileSync("../data/mapweights.json", this.mapWeights)
    }
}

op = new Optimizer(config)