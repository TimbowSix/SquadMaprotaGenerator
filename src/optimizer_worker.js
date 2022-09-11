const {workerData, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const config = require("../config.json")
const { fstat } = require('node:fs')
const fs = require("fs") 


console.log("start optimizer for "+ workerData["mode"])
op = new opt.Optimizer(config, workerData["mode"], reset= workerData["reset"], distribution = workerData["dist"], console_output = false, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 0.5, estimate = false, workerData["runIndex"], true)
console.time("Execution Time "+workerData["mode"])
op.start_optimizer()
let result = op.generator
parentPort.postMessage(result)
console.timeEnd("Execution Time "+workerData["mode"])

//fs.writeFileSync("result_"+workerData["mode"]+"_"+Date.now()+".json",JSON.stringify(result,null,2))
