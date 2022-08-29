function weightedChoice(arr, weights=[]){
    let sum = weights.reduce(function(pv, cv) { return pv + cv; }, 0);
    if (sum != 1) throw "weights do not sum to 1"
    if (arr.length != weights.length) throw "arr and weights don't have the same length"
    let w = []
    for(let i = 0; i<weights.length; i++){
        if (i == 0) w.push(weights[i])
        else w.push(weights[i]+w[w.length-1])
    }
    rnd = Math.random()
    for(let i = 0; i<arr.length; i++){
        if (rnd <= w[i]) return arr[i]
    }
}

// Math kram
function normalize(arr){
    let sum = arr.reduce(function(pv, cv) { return pv + cv; }, 0);
    for (let i = 0; i<arr.length; i++){
        arr[i] = arr[i]/sum
    }
    return arr
}

function squareArr(arr){
    let n_arr = []
    for(let ele of arr) n_arr.push(Math.pow(ele, 2))
    return n_arr
}

function sumArr(arr){
    return arr.reduce(function(pv, cv) { return pv + cv; }, 0)
}

function multiplyArr(arr1, arr2){
    let n_arr = []
    arr1.forEach((ele, ind) => {
        n_arr.push(ele*arr2[ind])
      })
    return n_arr
}


module.exports = { weightedChoice, normalize, squareArr, sumArr, multiplyArr };

function main(){
    console.log(multiplyArr([1,2,3], [3,2,1]))
}

//main()