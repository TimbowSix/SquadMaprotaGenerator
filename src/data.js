const https = require('https');
const fs = require('fs')
const statistics = require("./statistics.js")
const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;

class Layer_Data{
    initialize_maps(use_map_weights=true){
        let data = new Data()
        data = data.read()
        let bioms = data["bioms"]
        let map_weights = data["map_weights"]
        let distances = statistics.getAllMapDistances(bioms)
        let maps = []
        let layers = JSON.parse(fs.readFileSync("./data/layers.json", 'utf8'));

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
            for(let mode of Object.keys(layers[map_name])){
                for(let layer of layers[map_name][mode]){
                    let l = new Layer(layer["name"], mode, map_, layer["votes"])
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
    }
    add_layer(layer){
        if (!(layer.mode in this.layers)) this.layers[layer.mode] = [layer]
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
    get_layers(){
        let theUrl = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req"
        let xmlHttpReq = new XMLHttpRequest();
        xmlHttpReq.open("GET", theUrl, false);
        xmlHttpReq.send(null);
        return xmlHttpReq.responseText
    }
    read(){
        let file = fs.readFileSync("./data/data")
        return JSON.parse((atob(file)))
    }
}

function main(){
    let data = new Data()
    let d = data.get_layers()
    //todo transform data 
    if(d == 0){
        d = data.read()
    }
    console.log(d)
}

if (require.main === module) {
    //main()
    let ld = new Layer_Data()
    let maps = ld.initialize_maps()
    console.log(maps)
}

module.exports = { Map, Layer, Layer_Data, Data };