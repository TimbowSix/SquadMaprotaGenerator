const {Worker, parentPort} = require('node:worker_threads')

//const raas_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'RAAS', "dist": null} });
//const aas_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'AAS', "dist": null} });
//const inv_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Invasion', "dist": null} });
//const tc_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'TC', "dist": null} });
//const des_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Destruction', "dist": null} });
const ins_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: {"mode": 'Insurgency', "dist": null} });
        
ins_pool_worker.once('return', (msg) => {
    console.log(msg)
})