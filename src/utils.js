
/**
 * returns random element from array
 * @param {Array} arr 
 * @param {[number]} weights chances for every element, defaults to 1/arr.length
 * @returns 
 */
function choice(arr, weights=null){
    if(!(weights)){
        return arr[Math.floor(Math.random()*arr.length)];
    }
    let sum = round(sumArr(weights), 4);
    if (sum != 1) {
        throw Error("weights do not sum to 1")
    }
    if (arr.length != weights.length) throw Error("arr and weights don't have the same length")
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

function create_UUID(){
    var dt = new Date().getTime();
    var uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = (dt + Math.random()*16)%16 | 0;
        dt = Math.floor(dt/16);
        return (c=='x' ? r :(r&0x3|0x8)).toString(16);
    });
    return uuid;
}


/**
 * returns a dictionary where each input string gets a distribution
 * @param {[string]} modes
 * @param {number} numberMaps
 */
function get_mode_dist_dict(modes, numberMaps){
    let dict = {}
    for(let m of modes){
        dict[m] = get_random_dist(numberMaps)
    }
    return dict
}

// Math kram
/**
 * returns an array which entries sum up to one
 * @param {number} entries
 * @returns {[number]}
 */
function get_random_dist(entries){
    let dist = []
    for(let i=0; i<entries; i++){
        dist.push(Math.random())
    }
    return normalize(dist)
}

/**
 * normalizes an array of numbers
 * @param {[number]} arr 
 * @returns {[number]}
 */
function normalize(arr){
    if(arr.length <= 0) return arr
    let sum = sumArr(arr);
    for (let i = 0; i<arr.length; i++){
        arr[i] = arr[i]/sum
    }
    return arr
}

/**
 * rounds number to x decimal places
 * @param {number} val number
 * @param {int} digits decimal places
 * @returns 
 */
function round(val, digits=2){
    return Math.round((val + Number.EPSILON) * (Math.pow(10,digits))) / Math.pow(10,digits)
}

/**
 * squares every element of an array
 * @param {[number]} arr 
 * @returns {[number]}
 */
function squareArr(arr){
    let n_arr = []
    for(let ele of arr) n_arr.push(Math.pow(ele, 2))
    return n_arr
}

/**
 * returns sum of an array of numbers
 * @param {[number]} arr 
 * @returns {number}
 */
function sumArr(arr){
    return arr.reduce(function(pv, cv) { return pv + cv; }, 0)
}

/**
 * multiplies two arrays element wise
 * @param {[number]} arr1 
 * @param {[number]} arr2 
 * @returns {[number]}
 */
function multiplyArr(arr1, arr2){
    let n_arr = []
    arr1.forEach((ele, ind) => {
        n_arr.push(ele*arr2[ind])
      })
    return n_arr
}

/**
 * formats a layer string to preffered readable casing format.
 * If mode or map is not found it only gets to lower case
 * @param {string} layer is expected to have the standard format: map_mode_version
 * @returns {string} formatet Map_Mode_version
 */
function formatLayer(layer){
    layer = layer.toLowerCase()
    let values = layer.split("_")
    let map =  values[0]
    let mode = values[1]
    let ver = values[2]
    let modes = {"aas": "AAS", "raas": "RAAS", "tc": "TC", "ta": "TA", "invasion": "Invasion", "destruction": "Destruction","insurgency": "Insurgency", "seed": "Seed"}
    mode = modes.hasOwnProperty(mode) ? modes[mode] : mode

    maps = {'albasrah': 'AlBasrah','anvil': 'Anvil','belaya': 'Belaya','blackcoast': 'BlackCoast','chora': 'Chora','fallujah': 'Fallujah',
    'foolsroad': 'FoolsRoad','goosebay': 'GooseBay','gorodok': 'Gorodok','kamdesh': 'Kamdesh','kohat': 'Kohat','kokan': 'Kokan','lashkarvalley': 'LashkarValley',
    'logar': 'Logar','manic': 'Manic','mestia': 'Mestia','mutaha': 'Mutaha','narva': 'Narva','skorpo': 'Skorpo','sumari': 'Sumari','tallil': 'Tallil',
    'yehorivka': 'Yehorivka'
    }

    map = maps.hasOwnProperty(map) ? maps[map] : map

    return `${map}_${mode}_${ver}`
}

function arithmeticWeightedMean(weights, data){
    let temp = 0
    for(let i = 0; i < data.length; i++){
        temp += data[i]*weights[i]
    }
    return temp
}

if (require.main === module) {
    let test =  "koKan_rAAs_v1"
    console.log(formatLayer(test))
}

module.exports = { choice, normalize, squareArr, sumArr, multiplyArr, formatLayer, round, get_random_dist, get_mode_dist_dict, create_UUID};