const fs = require('fs')
const statistics = require("./statistics.js")
const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;
const utils = require("./utils.js")


class Layer_Data{

    initialize_maps(use_map_weights=true, cluster_radius){
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

        //init neighbors 
        for(let i=0;i<maps.length;i++){
            maps[i].neighbors = [];
            maps[i].neighbor_count = 0;
            for(let j=0;j<maps.length;j++){
                if(maps[i].distances[maps[j].name] < cluster_radius){
                    maps[i].neighbors.push(maps[j]);
                    maps[i].neighbor_count++;
                }
            }
        }

        return maps
    }
}

class Map{
    constructor(name, bioms, map_weights, distances, ){
        this.name = name
        this.layers = {}
        this.bioms = bioms
        this.map_weight = map_weights
        this.mapvote_weights = {}
        this.distances = distances
        this.neighbors = []
        this.neighbor_count = 0
        this.lock_time = 0
        this.current_lock_time = 0
        this.layer_by_pools = {}
    }
    add_layer(layer){
        if (!(layer.mode in this.layers)) this.layers[layer.mode] = [layer]
        else this.layers[layer.mode].push(layer)
    }
    decrease_lock_time(){
        if(this.current_lock_time >= 1){
            this.current_lock_time--;
        }
    }
    update_lock_time(){
        this.current_lock_time = this.lock_time
    }

    add_mapvote_weights(){
        votes = {}
        weights = {}
        means = {}
        sum = 0
        if(this.layers.length != 0){
            for(let pool in this.layer_by_pools){
                for(let l in this.layer_by_pools[pool]){
                    votes[pool].push.apply(votes[pool], [l.votes])
                }
                means[pool] = 1/votes[pool].length*utils.sumArr(votes[pool])
                for(let votes in votes[pool]){
                    if(weights[pool] != null){
                        weights[pool] += Math.exp(-Math.pow(means[pool] - votes, 2))
                    }
                    else{
                        weights[pool] = 0
                    }
                }
                for(let w in weights[pool]){
                    w = w/utils.sumArr(weights)
                }
            }
        }
        else{
            console.log(`No layers added to map ${this.name}, could not calculate mapvote_weights!`)
        }
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

function get_layers(){
    let theUrl = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req"
    let xmlHttpReq = new XMLHttpRequest();
    xmlHttpReq.open("GET", theUrl, false);
    xmlHttpReq.send(null);

    let data = JSON.parse(xmlHttpReq.responseText)
    let layers = data["AxisLabels"]
    let upvotes = data["DataSet"][0]
    let downvotes = data["DataSet"][1]
    let availability = data["DataSet"][2]
    let maps = {}
    for(let i=0; i<layers.length; i++){
        if (availability[i] > 0) continue
        let layer = utils.formatLayer(layers[i])
        let layer_values = layer.split("_")
        let map = layer_values[0]
        let mode = layer_values[1]

        let data = {"name": layer, "votes": upvotes[i]+downvotes[i]}
        if (maps.hasOwnProperty(map)){
            if(maps[map].hasOwnProperty(mode)){
                maps[map][mode].push(data)
            }else{
                maps[map][mode] = [data]
            }
        }else maps[map] = {mode: [data]}
    }
    return maps
}
function read(){
    let file = fs.readFileSync("./data/data", "ascii")
    return JSON.parse(Buffer.from(file, "base64").toString('utf8'))
}
function write(data){
    fs.writeFileSync("./data/data", Buffer.from(JSON.stringify(data)).toString("base64"))
}
function update_c(data, ch=null){
    if (!(ch)) ch = read();
    for (const [k, v] of Object.entries(data)) {
        ch[k] = v
    }
    return ch
}
function update_section(data, section){
    let ch = read()
    ch[section] = data
    write(ch)
}

function main(){
    return
}

if (require.main === module) {
    get_layers()
}

module.exports = { Map, Layer, initialize_maps, get_layers, read, write, update_c, update_section };