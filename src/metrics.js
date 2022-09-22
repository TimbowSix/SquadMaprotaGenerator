const config = require("../config.json");
const gen = require("./generator.js");
const fs = require("fs")

class Metrics{
    constructor(config){
        this.config = config

        this.config["number_of_rotas"] = 1
        this.config["number_of_layers"] = 100000
        this.config["seed_layer"] = 0
        this.config["use_vote_weight"] = true
        this.config["use_map_weight"] = true

        this.mr = new gen.Maprota(this.config)
        this.mr.generate_rota(false,true)
    }
    calc_map_dist_error(){
        let reference_dist = JSON.parse(fs.readFileSync("./data/current_map_dist.json"))

        let rota_dist = {}
        let sum = {}
        //format data
        for(let layer of this.mr.rotation){
            if(layer.map.name in rota_dist){
                rota_dist[layer.map.name][layer.mode]++
                if (layer.mode in sum){
                    sum[layer.mode]++
                }else{
                    sum[layer.mode] = 1
                }
            }else{
                rota_dist[layer.map.name] = {}
                for(let m of Object.keys(layer.map.layers)){
                    rota_dist[layer.map.name][m] = 0
                }
                rota_dist[layer.map.name][layer.mode] = 1
            }
        }
        
        for(let map of Object.keys(rota_dist)){
            for(let mode of Object.keys(rota_dist[map])){
                if(rota_dist[map][mode] != 0){
                    rota_dist[map][mode] /= sum[mode]
                }
            }
        }
        //calc dist error
        let error_per_mode = {}
        for(let map of Object.keys(reference_dist)){
            for(let mode of Object.keys(reference_dist[map])){

                let error = Math.pow(reference_dist[map][mode] - rota_dist[map][mode], 2)

                if(mode in error_per_mode){
                    error_per_mode[mode] += error
                }else{
                    error_per_mode[mode] = error
                }
            }
        }
        return error_per_mode
    }
    calc_modi_dist_error(){

        let reference_modi_dist = {}

        //get ref modi dist
        for(let m of Object.keys(this.config["mode_distribution"]["pools"])){
            for(let n of Object.keys(this.config["mode_distribution"]["pools"][m])){
                reference_modi_dist[n] = this.config["mode_distribution"]["pools"][m][n] * this.config["mode_distribution"]["pool_distribution"][m]
            }
        }
       

        let current_modi_dist = {}

        //get rota modi dist
        for(let mode of this.mr.modes){
            if(Object.keys(current_modi_dist).includes(mode)){
                current_modi_dist[mode]++
            }else{
                current_modi_dist[mode] = 1
            }
        }
        //normalize
        for(let mode of Object.keys(reference_modi_dist)){
            if(Object.keys(current_modi_dist).includes(mode)){
                current_modi_dist[mode] /= this.mr.modes.length
            }else{
                current_modi_dist[mode] = 0
            }
        }
        //calc error 
        let error = 0
        for(let mode of Object.keys(reference_modi_dist)){
            error += Math.pow(reference_modi_dist[mode] - current_modi_dist[mode], 2)
        }
        return error
    }
    calc_mean_biom_distance(){
        let sum = 0
        let last_map = this.mr.maps[0]
        for(let i=1;i<this.mr.maps.length;i++){
            sum += this.mr.maps[i].distances[last_map.name]
            last_map = this.mr.maps[i]
        }
        return sum / this.mr.maps.length
    }
    calc_moving_average(size){
        let output = []
        for(let i=size-1;i<this.mr.maps.length;i++){
            let sum = 0 
            for(let j = 0;j<size;j++){
                sum += this.mr.maps[i].distances[this.mr.maps[i-j].name]
            }
            output.push(sum / size)
        }
        return output
    }
    get_min_map_repetition(){
        let buffer = {}
        let min_rep = {}
        let index = 0
        for(let map of this.mr.maps){

            if(Object.keys(buffer).includes(map.name)){
                let min =  index - buffer[map.name]

                if(!Object.keys(min_rep).includes(map.name)){
                    min_rep[map.name] = min
                }

                if(min_rep[map.name] > min){
                    min_rep[map.name] = min
                }
            }

            buffer[map.name] = index

            index++
        }
        let min = min_rep[Object.keys(min_rep)[0]]
        for(let m of Object.keys(min_rep)){
            if(min_rep[m] < min){
                min = min_rep[m]
            }
        }
        return min
    }
}


if (require.main === module) {
    let temp = new Metrics(config)
    console.log(temp.calc_map_dist_error())
    console.log(temp.calc_modi_dist_error())
    console.log(temp.calc_mean_biom_distance())
    console.log(temp.get_min_map_repetition())
}
