const {workerData} = require('node:worker_threads')
const opt = require('./optimizer.js')
const config = require("../config.json")

console.log("start optimizer for "+ workerData)
op = new opt.Optimizer(config, workerData, reset=true)
console.time("Execution Time")
op.start_optimizer()
console.timeEnd("Execution Time")
