const gen = require("../src/generator.js")
const config = require("../config.json")
const dirTree = require("directory-tree")
const fs = require("fs")


function getDirectories(path) {
    return fs.readdirSync(path).filter(function (file) {
        return fs.statSync(path+'/'+file).isDirectory();
    });
}

const g = new gen.Maprota(config)
let dirs = getDirectories("./optimizer_data/")

//modi einstellen 
let modi = ["RAAS", "AAS", "Invasion", "TC", "Insurgency", "Destruction"]
let pathOfCsv = "datacsv_"+Date.now()+".csv"

let mapWeightsFiles = []
let propByMode = []
let output = [["map","neighbor","p RAAS", "RAAS", "p AAS", "AAS", "p inv", "Invasion", "p tc", "TC", "p ins", "Insurgency", "p des", "Destruction"]]

for(let dir of dirs){
    let modeDict = {}
    for (let file of fs.readdirSync("./optimizer_data/"+dir)){
        let temp = file.split("_")[0]
        if(temp == "mapweights"){
            mapWeightsFiles.push("./optimizer_data/"+dir+"/"+file)
        }
        let t = file.split("_")
        if(t[0] == "run"){
            let m = t[t.length-1].split(".")[0]
            modeDict[m] = JSON.parse(fs.readFileSync("./optimizer_data/"+dir+"/"+file))
        }
    }
    
    propByMode.push(modeDict)
}

let index = 0
for(let mpFile of mapWeightsFiles){
    let f = JSON.parse(fs.readFileSync(mpFile))
    for(let map of g.all_maps){
        let temp = [map.name , map.neighbor_count]
        for(let mode of modi){
            if(f[map.name][mode]){
                temp.push(propByMode[index][mode][map.name])
                temp.push(f[map.name][mode])
            }else{
                temp.push(0)
                temp.push(0)
            }
        }
        output.push(temp)
    }
    index++
}
//output to csv
let csv = ""
for(let i of output){
    csv += i.join(",") +"\r\n"
}

fs.writeFileSync(pathOfCsv, csv)

