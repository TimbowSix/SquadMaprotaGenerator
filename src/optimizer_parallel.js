const {Worker, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const fs = require("fs")
const config = require("../config.json")
const utils = require('./utils.js')
const gen = require('./generator.js')

let final_map_weights = JSON.parse(fs.readFileSync("./data/mapweights.json"))
let workers = []


let parallel_optimizer = null


class WorkerEntry{
    constructor(mode, dist = null, runIndex){
        this.mode = mode
        this.dist = dist
        if(dist != null){
            this.dist = dist[mode]
        }
        this.done = false
        this.worker = null
        this.runIndex = runIndex
    }
    start(reset = true){
        this.done = false
        this.worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": this.mode, "dist": this.dist, "reset": reset, "runIndex": this.runIndex} }); //reset = true map_weights reset to 0 else the weights from file
        this.worker.on('message', (msg) => {
            for(let map of msg.all_maps){
                if(Object.keys(final_map_weights[map.name]).includes(this.mode)){
                    final_map_weights[map.name][this.mode] = map.map_weight[this.mode]
                }
            }
            this.done = true
            let allDone = true
            for(let w of workers){
                if(!w.done){
                    allDone = false
                }
            }

            if(allDone){
                parallel_optimizer.runSeries()
            }

        })
    }
    startSync(mReset = true){
        let op = new opt.Optimizer(config, this.mode, mReset, this.dist, false, false, true, 0.5, false, this.runIndex)
        op.start_optimizer()
        let result = op.generator
        for(let map of result.all_maps){
            if(Object.keys(final_map_weights[map.name]).includes(this.mode)){
                final_map_weights[map.name][this.mode] = map.map_weight[this.mode]
            }
        }
    }
}

class OptimizerParallelOrganizer{
    constructor(modes = [], dist_all = null, runIndex){
        //modes must be sorted by maps size per mode 
        this.dist_all = dist_all
        this.runIndex = runIndex
        for(let mode of modes){
            workers.push(new WorkerEntry(mode,this.dist_all, this.runIndex))
        }
    }
    runParallel(){
        //parallel run
        console.log("run parallel")
        for(let w of workers){
            w.start(false)
        }
    }
    runSeries(){
        //save map weights from parallel run
        fs.writeFileSync("./data/mapweights.json", JSON.stringify(final_map_weights, null, 2));
        fs.writeFileSync("./optimizer_data/"+this.runIndex+"/mapweights_"+this.runIndex+".json", JSON.stringify(final_map_weights, null, 2))
        //run series
        console.timeEnd("Execution Time")
        console.log("run series")
        for(let i=workers.length-1;i>=0;i--){
            console.log("run "+workers[i].mode)
            workers[i].startSync(false);
            fs.writeFileSync("./data/mapweights.json", JSON.stringify(final_map_weights, null, 2));
        }
        //save map weights
        fs.writeFileSync("./optimizer_data/"+this.runIndex+"/mapweights_"+this.runIndex+".json", JSON.stringify(final_map_weights, null, 2))
        console.timeEnd("Execution Time")
    }
}

for(let i=0; i<2; i++){
    let dummy_gen = new gen.Maprota(config)
    let maps = dummy_gen.all_maps
    
    let modi = ["RAAS", "AAS", "Invasion", "TC", "Insurgency", "Destruction"]
    
    let all_maps_dict = utils.get_maps_modi_dict(maps, modi)
    
    let runIndex = Date.now()
    fs.mkdirSync("./optimizer_data/"+runIndex+"/")
    
    parallel_optimizer = new OptimizerParallelOrganizer(modi,all_maps_dict, runIndex)
    parallel_optimizer.runParallel()
}