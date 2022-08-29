const https = require('https');
const fs = require('fs')
const statistics = require("statistics.js")

class Layer_Data{
    initialize_maps(use_map_weights=true){
        let data = new Data.read()
        let bioms = data["bioms"]
        let map_weights = data["map_weights"]
        let distances = statistics.getAllMapDistances(bioms)
        let maps = []
        let layers = JSON.parse(fs.readFileSync("../data/layers.json", 'utf8'));

        for (const [map_name, biom_values] of Object.entries(bioms)) {
            // skip map if no layers available
            if (!(map_name in layers)) continue
            let weight = 0
            if (use_map_weights){
                // use map_weight_corrction = 0 if map is not in mapweights
                try{
                    weight = map_weights[map_name]
                }catch(e){
                    console.log("WARNING: Map '"+map_name+"' has no saved correction weights, this may destroy the map distribution!")
                }
            }
            let map_ = new Map(map_name, biom_values, weight, distances[map_name])
            for(let mode of layers[map_name]){
                for(let layer of layers[map_name][mode]){
                    l = new Layer(layer["name"], mode, map_, layer["votes"])
                    map_.add_layer(l)
                } 
            }
            maps.push(map_)
        }
        return maps      
    }
}

class Map{
    constructor(name, bioms, map_weight, distances, ){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = map_weight
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        //regulator stuff
        this.Kp = 0
        this.Ki = 0
        this.setValue = 0
        this.currIntVal = 0
    }
    add_layer(layer){
        if (! layer.mode in this.layers) this.layers[layer.mode] = [layer]
        else this.layers[layer.mode].push(layer)
    }
}

class Layer{
    constructor(name, mode, map, votes){
        this.name = name
        this.mode = mode
        this.map = map
        this.votes = votes
    }
}

class Data{
    async get_layers(){
        let url = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req"
        let options = {hostname: "welovesquad.com", path:"/wp-admin/admin-ajax.php?action=getLayerVotes_req", port: 443, method: "GET"}

        const req = https.request(options, res => {
            console.log(`statusCode: ${res.statusCode}`);
            res.on('data', d => {
                //process.stdout.write(d);
                let response = JSON.parse(d);
            });
        });
        req.on('error', error => {
            console.error(error);
        });
        
        req.end(); 
    }
    read(){
        let file = fs.readFileSync("./data/data")
        return JSON.parse((atob(file)))
    }
}

async function main(){
    let data = new Data()
    //let d = await data.get_layers()
    d = data.read()
    console.log(d)
}

main()
module.exports = { Map, Layer, Layer_Data, Data };