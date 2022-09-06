let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config, mode, reset = true, distribution = null, console_output = false, use_extern_map_weights_and_delta = false, save_maps = true, start_delta=0.15, estimate = true){
        this.config = config;
        this.estimate = estimate;
        this.console_output = console_output;
        this.use_save_maps = save_maps;
        this.current_mode = mode;
        this.current_modeG = this.get_mode_group_from_mode(mode);
        this.over_run = false;

        if(this.config["min_biom_distance"] != 0.5){
            throw Error("Der Optimizer ist nicht auf den min_biom_distance getrimmt");
        }

        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

    
        this.generator = new gen.Maprota(this.config);

        //set requested distribution
        this.wUni = {};
        if(distribution == null){
            let maps = [];
            for(let map of this.generator.all_maps){
                if(this.map_has_mode(map, this.current_mode)){
                    maps.push(map);
                }else{
                    map.map_weight[mode] = 0;
                    this.wUni[map.name] = 0;
                }
            }

            let temp = 1/maps.length;
            for(let map of maps){
                this.wUni[map.name] = temp;
            }
        }else{
            this.wUni = distribution;
            let temp = Object.keys(this.wUni);
            for(let map of this.generator.all_maps){
                if(!temp.includes(map.name)){
                    throw Error("distribution has not all maps");
                }
            }
        }

        //load existing values
        this.use_extern_map_weights_and_delta = use_extern_map_weights_and_delta;
        if(use_extern_map_weights_and_delta){
            try{
                this.delta = JSON.parse(fs.readFileSync("./data/delta.json"));
            }catch(err){
                this.delta = start_delta;
                this.saveDelta()
            }
        }else{
            this.data = start_delta;
        }

        //init maps
        for(let map  of this.generator.all_maps){
            map.distribution = 0;

            if(reset){
                map.map_weight[mode] = this.estimate_map_weight_even_dist(map, this.current_mode_group);
            }
        }
        if(reset){
            this.delta = start_delta;
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

    optimize_recursive(currentIndex, lowestDelta, minChanged){ //TODO map weight group umsetzten
        if(this.delta <= lowestDelta){
            return
        }
        //check if map has mode
        let start_index = currentIndex
        while(!this.map_has_mode(this.generator.all_maps[currentIndex], this.current_mode)){
            currentIndex++
            if(currentIndex >= this.generator.all_maps.length){
                currentIndex = 0
                this.over_run = true;
            }
            if(currentIndex == start_index){
                throw Error("mode group not in maps");
            }
        }
        
        this.update_mode_key(this.generator.all_maps[currentIndex], true);
        this.generator.generate_rota(reset=true);
        this.update_dist();
        let cMin = this.calc_current_norm();

        if(this.currentMin > cMin){
            //new min found
            this.currentMin = cMin;
            if(this.console_output){
                console.log("new min by +: "+cMin);
                console.log("mapweights for "+ this.current_mode);
                for(let map of this.generator.all_maps){
                    process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                }
                console.log();
            }

            this.saveMapWeights();
            this.save_maps();
            this.optimize_recursive(currentIndex,lowestDelta, true);
        }else{
            let counted_down = false;
            //no new min in plus direction found
            this.update_mode_key(this.generator.all_maps[currentIndex], false);
            //check negative direction
            let cMinM = this.currentMin;
            if((this.generator.all_maps[currentIndex].map_weight[this.current_mode] + 1  - this.delta) > 0){
                counted_down = true;
                this.update_mode_key(this.generator.all_maps[currentIndex], false);
                this.generator.generate_rota(reset=true);
                this.update_dist();
                cMinM = this.calc_current_norm();
            }

            if(cMinM < this.currentMin){
                this.currentMin = cMinM
                //new min found
                if(this.console_output){
                    console.log("new min by -: "+cMinM);
                    console.log("mapweights for "+ this.current_mode);
                    for(let map of this.generator.all_maps){
                        process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                    }
                    console.log();
                }
                this.saveMapWeights();
                this.save_maps();
                this.optimize_recursive(currentIndex,lowestDelta, true)
            }else{
                if(counted_down){
                    this.update_mode_key(this.generator.all_maps[currentIndex], true);
                }
                currentIndex++
                if(currentIndex >= this.generator.all_maps.length || this.over_run){
                    if(!this.over_run){
                        currentIndex = 0
                    }
                    if(!minChanged){
                        this.delta -= this.deltaStepSize
                        if(this.console_output){
                            console.log("new Delta "+this.delta)
                        }
                        this.saveDelta();
                    }
                    this.over_run = false;
                }
                this.optimize_recursive(currentIndex,lowestDelta, false)
            }
            return this.generator;
        }
    }

    update_mode_key(map, up){
        if(this.current_mode in map.map_weight){
            if(up){
                map.map_weight[this.current_mode] += this.delta;
            }else{
                map.map_weight[this.current_mode] -= this.delta;
            }
        } 
    }

    map_has_mode(map, mode){
        for(let m of Object.keys(map.layers)){
            if(m == mode){
                return true;
            }
        }
        return false;
    }

    get_modes_of_mode_group(mode_group){
        return Object.keys(this.config["mode_distribution"]["pools"][mode_group]);
    }

    get_mode_group_from_mode(mode_in){
        for(let modeG of Object.keys(this.config["mode_distribution"]["pools"])){
            for(let mode of Object.keys(this.config["mode_distribution"]["pools"][modeG])){
                if(mode == mode_in){
                    return modeG;
                }
            }
        }
        return null;
    }

    calc_current_norm(){
        let wTemp = 0
        for(let i=0;i<this.generator.all_maps.length;i++){
            let t = Math.pow(this.generator.all_maps[i].distribution - this.wUni[this.generator.all_maps[i].name], 2);
            wTemp += t;
        }
        return Math.sqrt(wTemp);
    }

    update_dist(){
        let tempSum = 0
        let maps_by_mode = this.generator.maps_by_mode();
        for(let map of this.generator.all_maps){
            let temp = maps_by_mode[this.current_mode][map.name]
            if(temp){
                tempSum += temp;
                map.distribution = temp;
            }else{
                tempSum += 0;
                map.distribution = 0;
            }
        }
        //normalize
        for(let map of this.generator.all_maps){
            map.distribution /= tempSum;
        }
    }
    saveMapWeights(){
        if(this.use_extern_map_weights_and_delta){
            let temp = {};
            for(let map of this.generator.all_maps){
                temp[map.name] = map.map_weight;
            }
            fs.writeFileSync("./data/mapweights.json", JSON.stringify(temp, null, 2));
        }
    }
    saveDelta(){
        if(this.use_extern_map_weights_and_delta){
            fs.writeFileSync("./data/delta.json", JSON.stringify(this.delta));
        }
    }

    estimate_map_weight_even_dist(map, mode_group){
        if(mode_group == "main" && this.estimate){
            return Math.pow(0.265*map.neighbor_count, 2.209)
        }else{
            return 0
        }
    }

    save_maps(){
        if(this.use_save_maps){
            let path = "./optimizer_maps_history_"+this.current_mode+".json";
            let history = [] 
            try {
                history = JSON.parse(fs.readFileSync(path))
            }catch(e) {
                //console.log(e)
            }
            let counts = {};
            for (let map of this.generator.maps) {
                counts[map.name] = counts[map.name] ? counts[map.name] + 1 : 1;
            }
            history.push(counts)
            fs.writeFileSync(path, JSON.stringify(history, null, 2))
        }
    }

    start_optimizer(){
        if(this.console_output){
            console.log("Starte Optimizer for "+this.current_mode);
            console.log("start min "+this.currentMin);
            for(let map of this.generator.all_maps){
                if(map.map_weight[this.current_mode]){
                    process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                }
            }
            console.log();
        }
        return this.optimize_recursive(0, 0.1, this.current_mode, false);
    }
}

module.exports = { Optimizer };

/*
op = new Optimizer(config, "Destruction", reset=true, distribution = null, console_output = true, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 0.5, estimate = false)
console.time("Execution Time")
op.start_optimizer()
console.timeEnd("Execution Time")*/