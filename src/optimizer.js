let utils = require("./utils.js")
let gen = require("./generator.js")
//let config = require("../config.json")
let weight_params = require("../data/weight_params.json")
let fs = require("fs");

class Optimizer{
    constructor(config, mode, reset = true, distribution = null, console_output = false, use_extern_map_weights_and_delta = false, save_maps = true, start_delta=0.15, runIndex, write_output_files){
        this.weight_params = weight_params
        this.runIndex = runIndex
        this.config = config
        this.console_output = console_output
        this.use_save_maps = save_maps
        this.current_mode = mode
        this.current_modeG = this.get_mode_group_from_mode(mode)
        this.write_output_files = write_output_files
        this.distribution = distribution


        this.config['seed_layer'] = 0;
        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = 50000;
        this.config['use_vote_weight'] = false;
        this.config["update_layers"] = false
        this.config["max_same_team"] = 0

        this.uuid = utils.create_UUID()


        if(!Object.keys(weight_params).includes(this.current_mode)){
            weight_params[this.current_mode] = [0,0,0,0,0,0]
        }


        this.generator = new gen.Maprota(this.config);

        //set requested distribution
        this.desired_dist = {};
        if(distribution == null){
            let maps = [];
            for(let map of this.generator.all_maps){
                if(this.map_has_mode(map, this.current_mode)){
                    maps.push(map);
                }else{
                    //map.map_weight[mode] = 0;
                    this.desired_dist[map.name] = 0;
                }
            }

            let temp = 1/maps.length;
            for(let map of maps){
                this.desired_dist[map.name] = temp;
            }
        }else{
            this.desired_dist = distribution;
            let temp = Object.keys(this.desired_dist);
            for(let map of this.generator.all_maps){
                if(!temp.includes(map.name)){
                    this.desired_dist[map.name] = 0
                    //console.log(map.name+" has been added to distribution for "+this.current_mode)
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
            this.delta = start_delta;
        }

        //init maps
        for(let map of this.generator.all_maps){
            map.distribution = 0;
        }
        if(reset){
            this.delta = start_delta;
            this.saveDelta();
        }

        //generate rota
        this.generator.generate_rota();
        this.update_dist();
        this.currentMin = this.calc_current_norm();

        //TODO vielleicht über config
        this.deltaStepSize = 0.05;

        // save current run information (distribution)
        if(this.save_run_info)
        this.write_run_info()
    }

    optimize_recursive(currentIndex, lowestDelta, minChanged){ //TODO map weight group umsetzten
        if(this.delta <= lowestDelta){
            return
        }

        this.update_map_weights_and_formula(currentIndex, true);
        this.generator.generate_rota();
        this.update_dist();
        let cMin = this.calc_current_norm();

        if(this.currentMin > cMin){
            //new min found
            this.currentMin = cMin;
            if(this.console_output){
                console.log("new min by +: "+cMin);
                console.log("mapweights for "+ this.current_mode);
                console.log(this.weight_params[this.current_mode])
                /*for(let map of this.generator.all_maps){
                    process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                }*/
                console.log();
            }


            this.save_maps();
            this.optimize_recursive(currentIndex,lowestDelta, true);
        }else{
            //no new min in plus direction found
            this.update_map_weights_and_formula(currentIndex, false);
            //check negative direction
            let cMinM = this.currentMin;

            this.update_map_weights_and_formula(currentIndex, false);
            this.generator.generate_rota();
            this.update_dist();
            cMinM = this.calc_current_norm();


            if(cMinM < this.currentMin){
                this.currentMin = cMinM
                //new min found
                if(this.console_output){
                    console.log("new min by -: "+cMinM);
                    console.log("mapweights for "+ this.current_mode);
                    console.log(this.weight_params[this.current_mode])
                    /*for(let map of this.generator.all_maps){
                        process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                    }*/
                    console.log();
                }

                this.save_maps();
                this.optimize_recursive(currentIndex,lowestDelta, true)
            }else{

                this.update_map_weights_and_formula(currentIndex, true);

                currentIndex++
                if(currentIndex >= this.weight_params[this.current_mode].length){
                    currentIndex = 0
                    if(!minChanged){
                        this.delta -= this.deltaStepSize
                        if(this.console_output){
                            console.log("new Delta "+this.delta)
                        }
                        this.saveDelta();
                    }
                }
                this.optimize_recursive(currentIndex,lowestDelta, false)
            }
            this.write_last_min()
            return this.weight_params
        }
    }

    update_map_weights_and_formula(paramIndex, up){

        //update param in formula
        if(up){
            this.weight_params[this.current_mode][paramIndex] += this.delta
        }else{
            this.weight_params[this.current_mode][paramIndex] -= this.delta
        }
        //update map weights
        this.generator.weight_params = this.weight_params
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
            let t = Math.pow(this.generator.all_maps[i].distribution - this.desired_dist[this.generator.all_maps[i].name], 2);
            wTemp += t;
        }
        return Math.sqrt(wTemp);
    }

    update_dist(){
        let tempSum = 0
        let maps_by_mode = this.generator.maps_by_mode();
        for(let map of this.generator.all_maps){
            if(Object.keys(maps_by_mode).includes(this.current_mode) && Object.keys(maps_by_mode[this.current_mode]).includes(map.name)){
                let temp = maps_by_mode[this.current_mode][map.name]
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

    saveDelta(){
        if(this.use_extern_map_weights_and_delta && this.write_output_files){
            fs.writeFileSync("./optimizer_data/"+this.runIndex+"/delta.json", JSON.stringify(this.delta));
        }
    }

    save_maps(){
        if(this.use_save_maps && this.write_output_files){
            let path = "./optimizer_data/"+this.runIndex+"/optimizer_maps_history_"+this.current_mode+"_"+this.uuid+".json";
            let history = []
            try {
                history = JSON.parse(fs.readFileSync(path))
            }catch(e) {
                //create dir
                try{
                    fs.mkdirSync("./optimizer_data/"+this.runIndex+"/")
                }catch(f){

                }
            }
            let maps_by_mode = this.generator.maps_by_mode();
            history.push(maps_by_mode)
            fs.writeFileSync(path, JSON.stringify(history, null, 2))
        }
    }

    write_run_info(){
        if(this.write_output_files){
            let path = "./optimizer_data/"+this.runIndex+"/run_info_"+this.uuid+"_"+this.current_mode+".json"
            try{
                fs.writeFileSync(path, JSON.stringify(this.desired_dist, null, 2))
            }catch(e){
                fs.mkdirSync("./optimizer_data/"+this.runIndex)
                fs.writeFileSync(path, JSON.stringify(this.desired_dist, null, 2))
            }
        }
    }

    write_last_min(){
        if(this.write_output_files){
            let path = "./optimizer_data/"+this.runIndex+"/last_min_"+this.uuid+"_"+this.current_mode+".json"
            fs.writeFileSync(path,JSON.stringify(this.currentMin, null, 2))
        }
    }

    start_optimizer(){
        if(this.console_output){
            console.log("Starte Optimizer for "+this.current_mode);
            console.log("start min "+this.currentMin);
            console.log(this.weight_params[this.current_mode])
            for(let map of this.generator.all_maps){
                if(map.map_weight[this.current_mode]){
                    process.stdout.write(map.name+" "+map.map_weight[this.current_mode]+" ");
                }
            }
            console.log();
        }
        return this.optimize_recursive(0, 0.01, this.current_mode, false);
    }
}

module.exports = { Optimizer };


if (require.main === module) {
    const data = require("./data.js")
    const config = data.build_config()
    let current_mode = "RAAS"
    dist = JSON.parse(fs.readFileSync("./data/current_map_dist.json"))

    mode_dist = {}

    for(let map of Object.keys(dist)){
        if(current_mode in dist[map]){
            mode_dist[map] = dist[map][current_mode]
        }
    }
    console.log(mode_dist)
    op = new Optimizer(config, current_mode, reset=true, distribution = mode_dist, console_output = true, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 2, 1, true)
    let a = op.start_optimizer()
    console.log(a)

}
