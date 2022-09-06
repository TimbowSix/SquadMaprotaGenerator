const {Worker, parentPort} = require('node:worker_threads')
const fs = require("fs")

let map_weights = JSON.parse(fs.readFileSync("./data/mapweights.json"))

const raas_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'RAAS', "dist": null} });
const aas_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'AAS', "dist": null} });
const inv_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Invasion', "dist": null} });
const tc_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'TC', "dist": null} });
const des_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Destruction', "dist": null} });
const ins_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Insurgency', "dist": null} });

let done = [0,0,0,0,0,0]
let running = true



raas_pool_worker.on('message', (msg) => {
    let m = 'RAAS'
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[0] = 1
})

aas_pool_worker.on('message', (msg) => {
    let m = 'AAS'
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[1] = 1
})


inv_pool_worker.on('message', (msg) => {
    let m = 'Invasion'
    //console.log(Object.keys(map_weights))
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[2] = 1
})


tc_pool_worker.on('message', (msg) => {
    let m = 'TC'
    //console.log(Object.keys(map_weights))
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[3] = 1
})


des_pool_worker.on('message', (msg) => {
    let m = 'Destruction'
    //console.log(Object.keys(map_weights))
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[4] = 1
})


des_pool_worker.on('message', (msg) => {
    let m = 'Insurgency'
    //console.log(Object.keys(map_weights))
    for(let map of msg.all_maps){
        if(Object.keys(map_weights[map.name]).includes(m)){
            map_weights[map.name][m] = map.map_weight[m]
        }
    }
    done[5] = 1
})


//wait for finish
while(running){
    running = false
    for(let d of done){
        if(d == 0){
            running = true
        }        
    }
}

fs.writeFileSync("mapweights_found.json",JSON.stringify(map_weights, null, 2))