import fs from fs

/**
 * returns random element from array
 * @param {Array} arr
 * @param {[number]} weights chances for every element, defaults to 1/arr.length
 * @returns
 */
 function choice<T>(arr: Array<T>, weights?: Array<number> | undefined): T | undefined{
    if(weights == undefined){
        return arr[Math.floor(Math.random()*arr.length)];
    }

    let sum = round(sumArr(weights), 4);
    if (sum != 1) {
        console.log(weights)
        throw Error("weights do not sum to 1")
    }

    if (arr.length != weights.length) throw Error("arr and weights don't have the same length")

    let w:Array<number> = []
    for(let i = 0; i<weights.length; i++){
        if (i == 0) w.push(weights[i])
        else w.push(weights[i]+w[w.length-1])
    }

    let rnd:number = Math.random()
    for(let i = 0; i<arr.length; i++){
        if (rnd <= w[i]) return arr[i]
    }
    return undefined
 }



 function get_maps_modi_dict(maps: Array<RotaMap>, modi: Array<Mode>){
    let dict: Map<Mode, Map<RotaMap, number>> = new Map()
    for(let map of maps){
        for(let mode of modi){
            if(dict.get(mode) == undefined){
                dict.set(mode, new Map())
            }

            if(map.map_weight.get(mode)){
                dict.get(mode)?.set(map, Math.random())
            }
            else{
                dict.get(mode)?.set(map, 0)
            }

        }
    }

    for(let mode of dict.keys()){
        let sum = 0
        let temp = dict.get(mode)

        if(temp == undefined) throw Error("no mode in maps modi dict")

        for(let weight of temp.values()){
            sum+=weight
        }
        for(let weight_key of temp.keys()){
            let curr: number | undefined = dict.get(mode)?.get(weight_key)

            if(curr == undefined) throw Error ("no number in maps modi dict")

            dict.get(mode)?.set(weight_key, (curr/sum))
        }

    }

    return dict
}



 /**
 * rounds number to x decimal places
 * @param {number} val number
 * @param {int} digits decimal places
 * @returns
 */
function round(val: number, digits:number =2){
    return Math.round((val + Number.EPSILON) * (Math.pow(10,digits))) / Math.pow(10,digits)
}

/**
 * returns sum of an array of numbers
 * @param {[number]} arr
 * @returns {number}
 */
 function sumArr(arr: Array<number>){
    return arr.reduce(function(pv, cv) { return pv + cv; }, 0)
}


function create_UUID(): string{
    let dt = new Date().getTime();
    let uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        let r = (dt + Math.random()*16)%16 | 0;
        dt = Math.floor(dt/16);
        return (c=='x' ? r :(r&0x3|0x8)).toString(16);
    });
    return uuid;
}
