let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config, mode_group, reset = true, distribution = null){
        this.current_mode_group = mode_group
        this.config = config;

        if(this.config["min_biom_distance"] != 0.5){
            throw error("Der Optimizer ist nicht auf den min_biom_distance getrimmt");
        }

        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

        this.config["mode_distribution"]["pool_distribution"]["main"] = 0
        this.config["mode_distribution"]["pool_distribution"]["intermediate"] = 0
        this.config["mode_distribution"]["pool_distribution"]["rest"] = 0

        this.config["mode_distribution"]["pool_distribution"][mode_group] = 1
        

        this.generator = new gen.Maprota(this.config);

        this.wUni = {};
        if(distribution == null){
            let temp = 1/this.generator.all_maps.length;
            for(let map of this.generator.all_maps){
                this.wUni[map.name] = temp;
            }
        }else{
            this.wUni = distribution;
            let temp = Object.keys(this.wUni);
            for(let map of this.generator.all_maps){
                if(!temp.includes(map.name)){
                    throw error("distribution has not all maps");
                }
            }
        }

        //load existing values
        
        this.mapWeights = JSON.parse(fs.readFileSync("./data/mapweights.json")); //TODO umstellen auf drei verschiedenen Weights Typen
        
        try{
            this.delta = JSON.parse(fs.readFileSync("./data/delta.json"));
        }catch(err){
            this.delta = 0.15
            this.saveDelta()
        }

        this.mode_to_modeGroup = {}
        for(let mode_group of Object.keys(this.config["mode_distribution"]["pools"])){
            if(mode_group != null){
                for (let mode of Object.keys(this.config["mode_distribution"]["pools"][mode_group])){
                    this.mode_to_modeGroup[mode] = mode_group;
                }
            }
        }

        //init maps
        for(let map  of this.generator.all_maps){
            map.distribution = 0;

            if(reset){
                for(let mode of Object.keys(map.map_weight)){
                    map.map_weight[mode] = this.estimate_map_weight_even_dist(map);
                }
            }

            for(let mode of Object.keys(map.layers)){
                let mg = this.mode_to_modeGroup[mode];
                if(!map.mode_groups.includes(mg)){
                    map.mode_groups.push(mg);
                }
            }

        }
        if(reset){
            this.delta = 0.15;
            this.saveDelta();
            this.saveMapWeights();
        }

        //generate rota
        this.generator.generate_rota();
        this.update_dist();
        this.currentMin = this.calc_current_norm();

        //TODO vielleicht über config
        this.deltaStepSize = 0.01;
    }

    optimize_recursive(currentIndex, lowestDelta, mode_group, minChanged){ //TODO map weight group umsetzten
        if(this.delta <= lowestDelta){
            return
        }
        //check if map has mode
        let start_index = currentIndex
        while(!this.generator.all_maps[currentIndex].mode_groups.includes(mode_group)){
            currentIndex++
            if(currentIndex >= this.generator.all_maps.length){
                currentIndex = 0
            }
            if(currentIndex == start_index){
                throw error("mode group not in maps");
            }
        }
        
        this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, true);
        this.generator.generate_rota();
        this.update_dist();
        let cMin = this.calc_current_norm();

        if(this.currentMin > cMin){
            //new min found
            this.currentMin = cMin;

            console.log("new min by +: "+cMin);
            console.log("mapweights for "+ mode_group);
            for(let map of this.generator.all_maps){
                process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]]+" ");
            }
            console.log();

            this.saveMapWeights();
            this.optimize_recursive(currentIndex,lowestDelta,mode_group, true)
        }else{
            let counted_down = false;
            //no new min in plus direction found
            this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, false);
            //check negative direction
            let cMinM = this.currentMin;
            if((this.generator.all_maps[currentIndex].map_weight[this.get_modes_of_mode_group(mode_group)[0]] + 1  - this.delta) > 0){
                counted_down = true;
                this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, false);
                this.generator.generate_rota();
                this.update_dist();
                cMinM = this.calc_current_norm();
            }

            if(cMinM < this.currentMin){
                this.currentMin = cMinM
                //new min found
                console.log("new min by -: "+cMinM);
                console.log("mapweights for "+ mode_group);
                for(let map of this.generator.all_maps){
                    process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]]+" ");
                }
                console.log();

                this.saveMapWeights();
                this.optimize_recursive(currentIndex,lowestDelta,mode_group, true)
            }else{
                if(counted_down){
                    this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, true);
                }
                currentIndex++
                if(currentIndex >= this.generator.all_maps.length){
                    currentIndex = 0
                    if(!minChanged){
                        this.delta -= this.deltaStepSize
                        console.log("new Delta "+this.delta)
                        this.saveDelta();
                    }
                }
                this.optimize_recursive(currentIndex,lowestDelta,mode_group,false)
            }
        }
    }

    update_mode_key_group(map, mode_group, up){
        for(let mode of Object.keys(this.config["mode_distribution"]["pools"][mode_group])){
            if(mode in map.map_weight){
                if(up){
                    map.map_weight[mode] += this.delta;
                }else{
                    map.map_weight[mode] -= this.delta;
                }
            }
        }
    }

    get_modes_of_mode_group(mode_group){
        return Object.keys(this.config["mode_distribution"]["pools"][mode_group]);
    }

    calc_current_norm(){
        let wTemp = 0
        for(let i=0;i<this.generator.all_maps.length;i++){
            wTemp += Math.pow(this.generator.all_maps[i].distribution - this.wUni[this.generator.all_maps[i].name], 2);
        }
        return Math.sqrt(wTemp);
    }

    update_dist(){
        let tempSum = 0
        //reset dist
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].distribution = 0;
        }
        //update
        for(let i=0;i<this.generator.maps.length;i++){
            this.generator.maps[i].distribution++;
            tempSum++;
        }
        //normalize
        for(let i=0;i<this.generator.all_maps.length;i++){
            this.generator.all_maps[i].distribution /= tempSum;
        }
        this.generator.maps = []
        this.generator.layers = []
    }
    saveMapWeights(){
        let temp = {};
        for(let map of this.generator.all_maps){
            temp[map.name] = map.map_weight;
        }
        fs.writeFileSync("./data/mapweights.json", JSON.stringify(temp, null, 2));
    }
    saveDelta(){
        fs.writeFileSync("./data/delta.json", JSON.stringify(this.delta));
    }

    estimate_map_weight_even_dist(map){
        return Math.pow(0.265*map.neighbor_count, 2.209)
    }

    start_optimizer(){
        console.log("Starte Optimizer for "+this.current_mode_group);
        console.log("start min "+this.currentMin);
        this.optimize_recursive(0, 0.1, this.current_mode_group, false);
    }
}

op = new Optimizer(config, "main", reset=true)
console.time("Execution Time")
op.start_optimizer()
console.timeEnd("Execution Time")