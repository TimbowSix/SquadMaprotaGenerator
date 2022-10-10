const {Worker, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const fs = require("fs")
//const config = require("../config.json")
const utils = require('./utils.js')
const gen = require('./generator.js')
const data = require("./data.js")
const config = data.build_config()

let final_params = JSON.parse(fs.readFileSync("./data/weight_params.json"))
let workers = []
let parallel_optimizer = null

class WorkerEntry{
    constructor(mode, dist = null, runIndex, finish_callback){
        this.finish_callback = finish_callback
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

            //save result
            final_params[this.mode] = msg

            //check if all done
            this.done = true
            let allDone = true
            for(let w of workers){
                if(!w.done){
                    allDone = false
                }
            }

            if(allDone){
                //No Series run for the start
                //parallel_optimizer.runSeries()
                //save params
                fs.writeFileSync("./data/weight_params.json", JSON.stringify(final_params,null,2))
                //callback
                this.finish_callback()
                process.exit()
            }

        })
    }
    //not used for now
    /*
    startSync(mReset = true){
        let op = new opt.Optimizer(config, this.mode, mReset, this.dist, false, false, true, 0.5, false, this.runIndex)
        op.start_optimizer()
        let result = op.generator
        for(let map of result.all_maps){
            if(Object.keys(final_map_weights[map.name]).includes(this.mode)){
                final_map_weights[map.name][this.mode] = map.map_weight[this.mode]
            }
        }
    }*/
}

class OptimizerParallelOrganizer{
    constructor(modes = [], dist_all = null, runIndex, finish_callback){
        //modes must be sorted by maps size per mode
        this.dist_all = dist_all
        this.runIndex = runIndex
        for(let mode of modes){
            workers.push(new WorkerEntry(mode,this.dist_all, this.runIndex, finish_callback))
        }
    }
    runParallel(){
        //parallel run
        console.log("run parallel")
        for(let w of workers){
            w.start(false)
        }
    }
    //not used for now
    /*
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
    }*/
}

/*
//for testing
let maps = dummy_gen.all_maps
let all_maps_dict = utils.get_maps_modi_dict(maps, modi)
*/
//fs.mkdirSync("./optimizer_data/"+runIndex+"/")

function start_optimizer_parallel(callback){
    let dummy_gen = new gen.Maprota(config) // for creating the newest current_map_dist
    let current_dist = JSON.parse(fs.readFileSync("./data/current_map_dist.json"))

    let modi = []
    for(let pools of Object.keys(config["mode_distribution"]["pools"])){
        for(let m of Object.keys(config["mode_distribution"]["pools"][pools])){
            modi.push(m)
        }
    }
    let runIndex = Date.now()

    let dist_modi_dict = {}

    for(let mode of modi){
        let mode_dist = {}
        for(let map of Object.keys(current_dist)){
            if(mode in current_dist[map]){
                mode_dist[map] = current_dist[map][mode]
            }
        }
        dist_modi_dict[mode] = mode_dist
    }

    function after_optimizer(){
        console.log("finish optimizer")
        callback()
    }

    parallel_optimizer = new OptimizerParallelOrganizer(modi, dist_modi_dict, runIndex, after_optimizer)
    parallel_optimizer.runParallel()
}

module.exports = {start_optimizer_parallel}
