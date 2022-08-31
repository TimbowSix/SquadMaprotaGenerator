let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config, mode_group){
        this.current_mode_group = mode_group
        this.config = config;
        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

        this.config["mode_distribution"]["pool_distribution"]["main"] = 0
        this.config["mode_distribution"]["pool_distribution"]["intermediate"] = 0
        this.config["mode_distribution"]["pool_distribution"]["rest"] = 0

        this.config["mode_distribution"]["pool_distribution"][mode_group] = 1
        

        this.generator = new gen.Maprota(this.config);

        this.wUni = 1/this.generator.all_maps.length; //TODO nicht mehr richtig muss dann tartet mapvote dist sein

        //load existing values
        
        this.mapWeights = JSON.parse(fs.readFileSync("./data/mapweights.json")); //TODO umstellen auf drei verschiedenen Weights Typen
        

        try{
            this.delta = JSON.parse(fs.readFileSync("./data/delta1.json"));
        }catch(err){
            this.delta = 1
            this.saveDelta()
        }
        
        //init maps
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].distribution = 0;
        }
        //generate rota
        this.generator.generate_rota();
        this.update_dist();
        this.currentMin = this.calc_current_norm();

        //TODO vielleicht über config
        this.deltaStepSize = 0.01;
    }

    optimize_recursive(currentIndex, lowestDelta, map_weight_group, minChanged){ //TODO map weight group umsetzten
        if(this.delta <= lowestDelta){
            return
        }
        
        this.generator.all_maps[currentIndex].map_weight[mapWeightKey] += this.delta;
        this.generator.generate_rota();
        this.update_dist();
        let cMin = this.calc_current_norm();
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
            //no new min in plus direction found
            //TODO minus direction
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
        let wTemp = 0
        for(let i=0;i<this.generator.all_maps.length;i++){
            wTemp += Math.pow(this.generator.all_maps[i].distribution - this.wUni, 2);
        }
        return Math.sqrt(wTemp);
    }

    update_dist(){
        let tempSum = 0
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
        fs.writeFileSync("./data/mapweights.json", JSON.stringify(this.mapWeights));
    }
    saveDelta(){
        fs.writeFileSync("./data/delta1.json", JSON.stringify(this.delta));
    }

    start_optimizer(){
        this.optimize_recursive(0, 0.1, this.current_mode, false)
    }
}

op = new Optimizer(config, "main")
for(let map of op.generator.all_maps){
    console.log(map.name)
    console.log(map.map_weight)
}
//op.start_optimizer()