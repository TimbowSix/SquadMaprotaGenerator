const {Worker, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const fs = require("fs")
const config = require("../config.json")

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
        let op = new opt.Optimizer(config, this.mode, mReset, this.dist, false, false, true, 0.15, false, this.runIndex)
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
        console.time("Execution Time")
        console.log("run parallel")
        for(let w of workers){
            w.start(false)
        }
        //save Mapweights
        fs.writeFileSync("./data/mapweights_"+this.runIndex+".json", JSON.stringify(final_map_weights, null, 2))
    }
    runSeries(){
        console.timeEnd("Execution Time")
        console.log("run series")
        for(let i=workers.length-1;i>=0;i--){
            console.log("run "+workers[i].mode)
            workers[i].startSync(false);
        }
        fs.writeFileSync("./data/mapweights_"+this.runIndex+".json", JSON.stringify(final_map_weights, null, 2))
        console.timeEnd("Execution Time")
    }
}


let modi = ["RAAS", "AAS", "Invasion", "TC", "Insurgency", "Destruction"]
//let modi = ["Destruction"]
let distribution_all = null //gleichverteilung
parallel_optimizer = new OptimizerParallelOrganizer(modi,distribution_all, Date.now())
parallel_optimizer.runParallel()