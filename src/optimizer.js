let data = require("./data.js")
let utils = require("./utils.js")
let stat = require("./statistics.js")
let gen = require("./generator.js")
let config = require("../config.json")
let fs = require("fs");

class Optimizer{
    constructor(config, mode_group, reset = true, distribution = null, console_output = false, use_extern_map_weights_and_delta = false, save_maps = true, start_delta=0.15, estimate = true){
        this.estimate = estimate;
        this.console_output = console_output;
        this.use_save_maps = save_maps;
        this.current_mode_group = mode_group;
        this.config = config;
        this.stepwidth = start_delta

        if(this.config["min_biom_distance"] != 0.5){
            throw Error("Der Optimizer ist nicht auf den min_biom_distance getrimmt");
        }

        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

        this.config["mode_distribution"]["pool_distribution"]["main"] = 0
        this.config["mode_distribution"]["pool_distribution"]["intermediate"] = 0
        this.config["mode_distribution"]["pool_distribution"]["rest"] = 0

        this.config["mode_distribution"]["pool_distribution"][mode_group] = 1
        this.config["mode_distribution"]["pool_spacing"] = 0

        let temp = 1/Object.keys(this.config["mode_distribution"]["pools"][mode_group]).length;
        for(let mode of Object.keys(this.config["mode_distribution"]["pools"][mode_group])){
            this.config["mode_distribution"]["pools"][mode_group][mode] = temp;
        }
        console.log(this.config["mode_distribution"]["pools"]);

    
        this.generator = new gen.Maprota(this.config);

        this.wUni = {};
        if(distribution == null){

            let maps = [];
            for(let map of this.generator.all_maps){
                if(this.map_has_mode_group(map, this.current_mode_group)){
                    maps.push(maps);
                }else{
                    for(let mode of this.get_modes_of_mode_group(this.current_mode_group)){
                        map.map_weight[mode] = 0;
                    }
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
                for(let mode of this.get_modes_of_mode_group(this.current_mode_group)){
                    map.map_weight[mode] = this.estimate_map_weight_even_dist(map, this.current_mode_group);
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
                throw Error("mode group not in maps");
            }
        }
        
        this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, true);
        this.generator.generate_rota(reset=true);
        this.update_dist();
        let cMin = this.calc_current_norm();

        if(this.currentMin > cMin){
            //new min found
            this.currentMin = cMin;
            if(this.console_output){
                console.log("new min by +: "+cMin);
                console.log("mapweights for "+ mode_group);
                for(let map of this.generator.all_maps){
                    if(map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]] != null){
                        process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]]+" ");
                    }else if(map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[1]] != null){
                        process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[1]]+" ");
                    }
                }
                console.log();
            }

            this.saveMapWeights();
            this.save_maps();
            this.optimize_recursive(currentIndex,lowestDelta,mode_group, true);
        }else{
            let counted_down = false;
            //no new min in plus direction found
            this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, false);
            //check negative direction
            let cMinM = this.currentMin;
            if((this.generator.all_maps[currentIndex].map_weight[this.get_modes_of_mode_group(mode_group)[0]] + 1  - this.delta) > 0){
                counted_down = true;
                this.update_mode_key_group(this.generator.all_maps[currentIndex], mode_group, false);
                this.generator.generate_rota(reset=true);
                this.update_dist();
                cMinM = this.calc_current_norm();
            }

            if(cMinM < this.currentMin){
                this.currentMin = cMinM
                //new min found
                if(this.console_output){
                    console.log("new min by -: "+cMinM);
                    console.log("mapweights for "+ mode_group);
                    for(let map of this.generator.all_maps){
                        if(map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]] != null){
                            process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[0]]+" ");
                        }else if(map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[1]] != null){
                            process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][mode_group])[1]]+" ");
                        }
                    }
                    console.log();
                }
                this.saveMapWeights();
                this.save_maps();
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
                        if(this.console_output){
                            console.log("new Delta "+this.delta)
                        }
                        this.saveDelta();
                    }
                }
                this.optimize_recursive(currentIndex,lowestDelta,mode_group,false)
            }
            return this.generator;
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

    map_has_mode_group(map, mode_group){
        return mode_group in map.mode_groups
    }

    get_modes_of_mode_group(mode_group){
        return Object.keys(this.config["mode_distribution"]["pools"][mode_group]);
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
            let path = "./optimizer_maps_history_"+this.current_mode_group+".json";
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
            console.log("Starte Optimizer for "+this.current_mode_group);
            console.log("start min "+this.currentMin);
            for(let map of this.generator.all_maps){
                if(map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][this.current_mode_group])[0]]){
                    process.stdout.write(map.name+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][this.current_mode_group])[0]]+" "+map.map_weight[Object.keys(this.config["mode_distribution"]["pools"][this.current_mode_group])[1]]+" ");
                }
            }
            console.log();
        }
        this.optimize_recursive(0, 0.1, this.current_mode_group, false);
    }
    
    
}

class OptimizationObject{
    constructor(config, mode, maps, dx){
        this.mode = "undefined",
        this.maps = []
        this.weights = []
        this.dx = 0
        this.distribution = []
        this.config = config

        if(this.config["min_biom_distance"] != 0.5){
            throw Error("Der Optimizer ist nicht auf den min_biom_distance getrimmt");
        }

        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;

        this.config["mode_distribution"]["pool_distribution"]["main"] = 0
        this.config["mode_distribution"]["pool_distribution"]["intermediate"] = 0
        this.config["mode_distribution"]["pool_distribution"]["rest"] = 0

        this.config["mode_distribution"]["pool_distribution"][mode_group] = 1
        this.config["mode_distribution"]["pool_spacing"] = 0

        let dummy = 1/Object.keys(this.config["mode_distribution"]["pools"][mode_group]).length;
        for(let mode of Object.keys(this.config["mode_distribution"]["pools"][mode_group])){
            this.config["mode_distribution"]["pools"][mode_group][mode] = dummy;
        }
        console.log(this.config["mode_distribution"]["pools"]);

    
        this.generator = new gen.Maprota(this.config);

        let maps = [];
        for(let map of this.generator.all_maps){
            if(this.map_has_mode_group(map, this.current_mode_group)){
                maps.push(maps);
            }else{
                for(let mode of this.get_modes_of_mode_group(this.current_mode_group)){
                    map.map_weight[mode] = 0;
                }
                this.wUni[map.name] = 0;
            }
        }

        let temp = 1/maps.length;
        for(let map of maps){
            this.wUni[map.name] = temp;
        }
    }
    generate_rotas(){

        // MAP ROTA ALG HERE!!!
        this.distribution = []
    }
    // Sets the weights and maps in accordance with the choosen mode
    set_weights(){
        let temp = []
        if(this.mode != "undefined"){
            for(let map of maps){
                if(map.mode == this.mode){
                    this.weights.push(1)
                    temp.push(map)
                }
            }
            this.maps = temp
        }
        else{
            console.log("Define the mode first!")
        }
    }
}

// opt_object is a class containing the weights, choosen mode to optimize and the maprota-generator
function main_optimize(opt_object, end_distribution, threshold){
    currentMin = Infinity
    while(currentMin > threshold){
        oldMin = currentMin
        currentMin = get_next_minimum(opt_object, currentMin, end_distribution)
        // Abort if the minimum did not change in the last cycle(going over all coordinate axes)
        if(oldMin == currentMin){
            threshold = Infinity
        }
    }
}

function get_next_minimum(opt_object, currentMin, end_distribution){
    let temp = currentMin
    let minimum_found = false
    // Do a linesearch for each coordinate axis
    for(let i=0; i++; i<end_distribution.length){
        // Stay on the current axis as long as there is no minimum found
        while(!minimum_found){
            temp = currentMin
            currentMin = get_next_minimum_1d(opt_object, end_distribution, i)
            if(currentMin >= temp){
                minimum_found = true
            }
        }
    }
    return currentMin
}

function get_next_minimum_1d(opt_object, end_distribution, coordinate_index){
    // Calculate locale derivative and deduce slope from its value, alter the weight of the current axis
    let stepwidth = get_new_stepwidth(opt_object.weights, opt_object.dx, coordinate_index)
    opt_object.weights[coordinate_index] += stepwidth

    // Calculate the new optimization value
    return optimizerValue(opt_object, end_distribution)

}

function get_new_stepwidth(current_weights, dx, direction_index){
    let newweights = current_weights
    newweights[direction_index] += dx
    let f1 = optimizer(newweights)
    newweights[direction_index] -= 2*dx
    let f2 = optimizer(newweights)
    let deriv = derivative_twosided(f1, f2, dx)
    return stepwidth_from_slope(deriv)
}
function derivative_twosided(f1, f2, h){
    return (f1- f2)/(2*h)
}
function stepwidth_from_slope(slope, factor=1, shift=1){
    return factor/Math.sqrt(slope+shift)
}

function optimizerValue(opt_object, end_distribution){
    // Generate Rotas
    opt_object.generate_rotas()
    // Get Distribution
    return optimizationFunction(opt_object.distribution, end_distribution)
}

function optimizationFunction(current_distribution, end_distribution){
    let opt = 0
    let diff = current_distribution - end_distribution
    for(let i=0; i < current_distribution.length; i++){
        opt += Math.pow(diff[i],2)
    }
    return Math.sqrt(opt)
}



module.exports = { Optimizer };

console.log(derivative_twosided(testing, 2, 0.001))
op = new Optimizer(config, "rest", reset=true, distribution = null, console_output = true, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 0.5, estimate = false)
console.time("Execution Time")
op.start_optimizer()
console.timeEnd("Execution Time")