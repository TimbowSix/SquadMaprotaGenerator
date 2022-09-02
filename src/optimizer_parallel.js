const {Worker, workerData} = require('node:worker_threads')

const main_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: 'main' });
const intermediate_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: 'intermediate' });
const rest_pool_worker = new Worker("./src/optimizer_worker.js",{ workerData: 'rest' });