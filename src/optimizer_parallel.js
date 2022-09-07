const {Worker, parentPort} = require('node:worker_threads')
const fs = require("fs")

let final_map_weights = JSON.parse(fs.readFileSync("./data/mapweights.json"))

class WorkerEntry{
    constructor(mode, dist = null){
        this.mode = mode
        this.dist = dist
        this.done = false
        this.worker = null
    }
    start(reset = true){
        this.done = false
        this.worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": this.mode, "dist": this.dist, "reset": reset} }); //reset = true map_weights reset to 0 else the weights from file
        this.worker.on('message', (msg) => {
            let m = this.mode
            for(let map of msg.all_maps){
                if(Object.keys(final_map_weights[map.name]).includes(m)){
                    final_map_weights[map.name][m] = map.map_weight[m]
                }
            }
            this.done = true
        })
    }
}

class OptimizerParallelOrganizer{
    constructor(modes = [], dist = null){
        //modes must be sorted by maps size per mode 
        this.dist = dist
        this.workers = {}
        for(let mode of modes){
            this.workers[mode] = new WorkerEntry(mode,this.dist)
        }
    }
    run(){
        //parallel run
        console.log("run parallel")
        for(let w of Object.keys(this.workers)){
            this.workers[w].start(true)
        }
        let running = true
        while(running){
            running = false
            for(let w of Object.keys(this.workers)){
                if(!this.workers[w].done){
                    running = true
                }
            }
        }
        //save Mapweights
        fs.writeFileSync("./data/mapweights.json", JSON.stringify(final_map_weights, null, 2))
        console.log("run series")
        for(let i=this.workers.length-1;i>=0;i--){
            console.log("run "+this.workers[i].mode)
            this.workers[i].start(false);
            while(!this.workers[i].done){}
        }
        fs.writeFileSync("./data/mapweights.json", JSON.stringify(final_map_weights, null, 2))
    }
}

let modi = ["RAAS", "AAS", "Inversion", "TC", "Insurgency", "Destruction"]
let distribution = null //gleichverteilung
let parallel_optimizer = new OptimizerParallelOrganizer(modi,distribution)
parallel_optimizer.run()