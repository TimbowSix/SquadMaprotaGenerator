const {workerData, parentPort} = require('node:worker_threads')
const opt = require('./optimizer.js')
const config = require("../config.json")


console.log("start optimizer for "+ workerData["mode"])
op = new opt.Optimizer(config, workerData["mode"], reset=workerData["reset"], distribution = workerData["dist"], console_output = false, use_extern_map_weights_and_delta = false,save_maps=true,start_delta = 2, workerData["runIndex"], false)
console.time("Execution Time "+workerData["mode"])
op.start_optimizer()
parentPort.postMessage(op.weight_params[workerData["mode"]])
console.timeEnd("Execution Time "+workerData["mode"])
