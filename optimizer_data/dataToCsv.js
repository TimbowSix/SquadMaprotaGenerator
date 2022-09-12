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
let dirs = getDirectories(".")

//modi einstellen 
let modi = ["RAAS", "AAS", "Invasion", "TC", "Insurgency", "Destruction"]
let pathOfCsv = "datacsv_"+Date.now()+".csv"

let mapWeightsFiles = []
let output = [["map","neighbor","RAAS", "AAS", "Invasion", "TC", "Insurgency", "Destruction"]]

for(let dir of dirs){
    for (let file of fs.readdirSync("./"+dir)){
        let temp = file.split("_")[0]
        if(temp == "mapweights"){
            mapWeightsFiles.push("./"+dir+"/"+file)
        }
    }
}

for(let mpFile of mapWeightsFiles){
    let f = JSON.parse(fs.readFileSync(mpFile))
    for(let map of g.all_maps){
        let temp = [map.name , map.neighbor_count]
        for(let mode of modi){
            if(f[map.name][mode]){
                temp.push(f[map.name][mode])
            }else{
                temp.push(0)
            }
        }
        output.push(temp)
    }
}
//output to csv
let csv = ""
for(let i of output){
    csv += i.join(",") +"\r\n"
}

fs.writeFileSync(pathOfCsv, csv)

