const {workerData, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const config = require("../config.json")
const { fstat } = require('node:fs')
const fs = require("fs") 


console.log("start optimizer for "+ workerData["mode"])
op = new opt.Optimizer(config, workerData["mode"], reset=true, distribution = workerData["dist"], console_output = false, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 0.5, estimate = false)
console.time("Execution Time")
result = op.start_optimizer()
parentPort.postMessage(result)
console.timeEnd("Execution Time")

//fs.writeFileSync("result_"+workerData["mode"]+"_"+Date.now()+".json",JSON.stringify(result,null,2))
