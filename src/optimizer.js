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

        //TODO vielleicht über config
        this.deltaStepSize = 0.01
    }

    optimize_recursive(currentIndex, lowestDelta, mapWeightKey, minChanged){
        if(this.delta <= lowestDelta){
            return
        }
        
        this.generator.all_maps[currentIndex].map_weight[mapWeightKey] += this.delta;
        //this.generator.generate //TODO 
        this.update_dist();
        cMin = this.calc_current_norm();
        if(this.currentMin > cMin){
            //new min found
            this.currentMin = cMin;

            console.log("new min: "+cMin);
            console.group("mapweights");
            for(let i=0;i<this.generator.all_maps.length;i++){
                console.log(this.generator.all_maps[i].map_weight[mapWeightKey]);
            }
            console.groupEnd();

            this.saveMapWeights();
            this.optimize_recursive(currentIndex,lowestDelta,mapWeightKey, true)
        }else{
            //no new min found
            this.generator.all_maps[currentIndex].map_weight[mapWeightKey] -= this.delta;
            currentIndex++
            if(currentIndex >= this.generator.all_maps.length){
                currentIndex = 0
                if(!minChanged){
                    this.delta -= this.deltaStepSize
                    console.log("new Delta "+this.delta)
                    this.saveDelta();
                }
            }
            this.optimize_recursive(currentIndex,lowestDelta,mapWeightKey,false)
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
        fs.writeFileSync("../data/mapweights.json", this.mapWeights);
    }
    saveDelta(){
        fs.writeFileSync("../data/delta.json", this.delta);
    }
}

op = new Optimizer(config)