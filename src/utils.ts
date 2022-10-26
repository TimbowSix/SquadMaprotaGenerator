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