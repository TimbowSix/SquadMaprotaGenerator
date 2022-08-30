const utils = require("./utils.js")
const fs = require("fs")

function getAllMapDistances(allMapsDict){
    let numberOfMaps = Object.keys(allMapsDict).length
    let distancesDict = {}
    for(let k = 0; k < numberOfMaps; k++){
        let mapkey = Object.keys(allMapsDict)[k]
        let temp = {}
        for(let i = 0; i<numberOfMaps; i++){
            if (mapkey == Object.keys(allMapsDict)[i]) temp[Object.keys(allMapsDict)[i]] = 0
            else{
                let absValue1 = Math.sqrt(utils.sumArr(utils.squareArr(allMapsDict[mapkey], allMapsDict[mapkey])))
                let absValue2 = Math.sqrt(utils.sumArr(utils.squareArr(allMapsDict[Object.keys(allMapsDict)[i]], allMapsDict[Object.keys(allMapsDict)[i]])))
                temp[Object.keys(allMapsDict)[i]] = Math.acos(utils.sumArr(utils.multiplyArr(allMapsDict[mapkey], allMapsDict[Object.keys(allMapsDict)[i]])) / (absValue1 * absValue2))
            }
            distancesDict[mapkey] = temp
        }
    }
    return distancesDict
}

function sigmoid(x, slope, shift=0){
    let arg = slope*(x+shift)
    return 1/(1+Math.exp(-arg))
}

function sigmoidArr(x, slope, shift=0){
    let res = []
    for(i=0; i<x.length; i++){
        res.push.apply(res, [sigmoid(x, slope, shift)])
    }
    return res
}

module.exports = { getAllMapDistances };

function main(){
    let bioms = JSON.parse(fs.readFileSync("./data/bioms.json"))
    //let dist = getAllMapDistances(bioms)
    console.time("test")
    //console.log(dist)
    for(let i=0; i<1000; i++) getAllMapDistances(bioms)
    console.timeEnd("test")
}

if (require.main === module) {
    main()
}