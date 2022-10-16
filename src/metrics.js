const data = require('./data');
const config = data.build_config();
const gen = require('./generator.js');
const utils = require('./utils.js');
const fs = require('fs');

class Metrics {
    constructor() {
        this.config = config;
        this.nr_of_layers = 100000;

        this.config['number_of_rotas'] = 1;
        this.config['number_of_layers'] = this.nr_of_layers;
        this.config['seed_layer'] = 0;
        this.config['use_vote_weight'] = true;
        this.config['use_map_weight'] = true;

        this.mr = new gen.Maprota(this.config);
        this.mr.generate_rota(false, true);
    }
    calc_map_dist_error() {
        let reference_dist = JSON.parse(
            fs.readFileSync('./data/current_map_dist.json')
        );

        let rota_dist = {};
        let sum = {};
        //format data
        for (let layer of this.mr.rotation) {
            if (layer.map.name in rota_dist) {
                rota_dist[layer.map.name][layer.mode]++;
                if (layer.mode in sum) {
                    sum[layer.mode]++;
                } else {
                    sum[layer.mode] = 1;
                }
            } else {
                rota_dist[layer.map.name] = {};
                for (let m of Object.keys(layer.map.layers)) {
                    rota_dist[layer.map.name][m] = 0;
                }
                rota_dist[layer.map.name][layer.mode] = 1;
            }
        }

        for (let map of Object.keys(rota_dist)) {
            for (let mode of Object.keys(rota_dist[map])) {
                if (rota_dist[map][mode] != 0) {
                    rota_dist[map][mode] /= sum[mode];
                }
            }
        }
        //calc dist error
        let error_per_mode = {};
        for (let map of Object.keys(reference_dist)) {
            for (let mode of Object.keys(reference_dist[map])) {
                let error = Math.pow(
                    reference_dist[map][mode] - rota_dist[map][mode],
                    2
                );

                if (mode in error_per_mode) {
                    error_per_mode[mode] += error;
                } else {
                    error_per_mode[mode] = error;
                }
            }
        }
        return error_per_mode;
    }
    calc_modi_dist_error() {
        let reference_modi_dist = {};

        //get ref modi dist
        for (let m of Object.keys(this.config['mode_distribution']['pools'])) {
            for (let n of Object.keys(
                this.config['mode_distribution']['pools'][m]
            )) {
                reference_modi_dist[n] =
                    this.config['mode_distribution']['pools'][m][n] *
                    this.config['mode_distribution']['pool_distribution'][m];
            }
        }

        let current_modi_dist = {};

        //get rota modi dist
        for (let mode of this.mr.modes) {
            if (Object.keys(current_modi_dist).includes(mode)) {
                current_modi_dist[mode]++;
            } else {
                current_modi_dist[mode] = 1;
            }
        }
        //normalize
        for (let mode of Object.keys(reference_modi_dist)) {
            if (Object.keys(current_modi_dist).includes(mode)) {
                current_modi_dist[mode] /= this.mr.modes.length;
            } else {
                current_modi_dist[mode] = 0;
            }
        }
        //calc error
        let error = 0;
        for (let mode of Object.keys(reference_modi_dist)) {
            error += Math.pow(
                reference_modi_dist[mode] - current_modi_dist[mode],
                2
            );
        }
        return error;
    }
    calc_mean_biom_distance() {
        let sum = 0;
        let last_map = this.mr.maps[0];
        for (let i = 1; i < this.mr.maps.length; i++) {
            sum += this.mr.maps[i].distances[last_map.name];
            last_map = this.mr.maps[i];
        }
        return sum / this.mr.maps.length;
    }
    calc_moving_average(size) {
        let output = [];
        for (let i = size; i < this.mr.maps.length; i++) {
            let sum = 0;
            let t = [];
            for (let j = 1; j <= size; j++) {
                t.push(
                    this.mr.maps[i - (j - 1)].distances[
                        this.mr.maps[i - j].name
                    ]
                );
                sum +=
                    this.mr.maps[i - (j - 1)].distances[
                        this.mr.maps[i - j].name
                    ];
            }
            //console.log(t)
            //console.log(sum/size)
            //console.log(size)
            output.push(sum / size);
        }
        return output;
    }
    get_all_map_dist() {
        let output = [];
        let last_map = this.mr.maps[0];
        for (let i = 1; i < this.mr.maps.length; i++) {
            output.push(this.mr.maps[i].distances[last_map.name]);
            last_map = this.mr.maps[i];
        }
        return output;
    }
    get_map_repetition() {
        let buffer = {};
        let min_rep = {};
        let output = [];
        let index = 0;
        for (let map of this.mr.maps) {
            if (Object.keys(buffer).includes(map.name)) {
                let min = index - buffer[map.name];

                /*if(!Object.keys(min_rep).includes(map.name)){
                    min_rep[map.name] = min
                }*/

                /*if(min_rep[map.name] > min){
                    min_rep[map.name] = min
                }*/
                min_rep[map.name] = min;
                output.push(min);
            }

            buffer[map.name] = index;

            index++;
        }
        /*let min = min_rep[Object.keys(min_rep)[0]]
        for(let m of Object.keys(min_rep)){
            if(min_rep[m] < min){
                min = min_rep[m]
            }
        }*/
        return output;
    }

    get_patterns(size) {
        let k = size - 1;
        let current_clusters = {};
        let buffer = [];

        for (let i = 0; i < k; i++) {
            buffer.push(this.mr.maps[i]);
        }

        for (let i = k; i < this.mr.maps.length; i++) {
            buffer.push(this.mr.maps[i]);
            let temp_name = '';
            for (let j = 0; j <= k; j++) {
                temp_name += buffer[j].name + '=>';
            }
            buffer.shift();
            if (Object.keys(current_clusters).includes(temp_name)) {
                current_clusters[temp_name]++;
            } else {
                current_clusters[temp_name] = 1;
            }
        }

        //let e = this.nr_of_rotas / Math.pow(this.mr.all_maps.length, size)

        let temp = [];
        for (let i of Object.keys(current_clusters)) {
            temp.push(current_clusters[i]);
        }
        /*
        let out = {}
        for(let t of temp){
            if(Object.keys(out).includes(t+"")){
                out[t+""]++
            }else{
                out[t+""] = 1
            }
        }

        console.log(out)

        let output = {}
        for(let x of Object.keys(out)){
            //output.push(out[t] / this.mr.all_maps.length*(Math.pow((1/22), (size*t))))
            let p = (Math.pow((1/22), size))
            //console.log("out "+out[x])
            console.log(utils.binomial(this.nr_of_rotas, x)+" "+Math.pow(p,x)+" "+Math.pow((1-p),(this.nr_of_rotas-x)))
            //console.log("blub "+utils.binomial(this.mr.all_maps.length, x)*Math.pow(p,x)*Math.pow((1-p),(this.nr_of_rotas-x)) * 22)
            console.log((utils.binomial(this.nr_of_rotas, x)*Math.pow(p,x)*Math.pow((1-p),(this.nr_of_rotas-x)) * this.nr_of_rotas))
            output[x] = (out[x] / (utils.binomial(this.nr_of_rotas, x)*Math.pow(p,x)*Math.pow((1-p),(this.nr_of_rotas-x)) * this.nr_of_rotas))
        }*/

        fs.writeFileSync(
            'test_patterns_' + size + '.json',
            JSON.stringify(temp)
        );
        return current_clusters;
    }
    binomial(n, x, p) {
        return utils.binomial(n, x) * Math.pow(p, x) * Math.pow(1 - p, n - x);
    }

    get_layer_repetition() {
        let buffer = {};
        let buffer_index = {};
        let min_rep = {};
        let output = [];

        for (let layer of this.mr.rotation) {
            if (Object.keys(buffer).includes(layer.map.name)) {
                if (Object.keys(buffer[layer.map.name]).includes(layer.name)) {
                    let min =
                        buffer_index[layer.map.name] -
                        buffer[layer.map.name][layer.name];
                    min_rep[layer.name] = min;
                    output.push(min);
                }
                buffer_index[layer.map.name]++;
                buffer[layer.map.name][layer.name] =
                    buffer_index[layer.map.name];
            } else {
                buffer_index[layer.map.name] = 0;
                buffer[layer.map.name] = {};
                buffer[layer.map.name][layer.name] = 0;
            }
        }
        return output;
    }
}

if (require.main === module) {
    let temp = new Metrics(config);
    //fs.writeFileSync("test_moving_avg.json", JSON.stringify(temp.calc_moving_average(5)))
    //fs.writeFileSync("moving_avg.json", JSON.stringify(temp.calc_moving_average(5)))
    //temp.get_patterns(2)
    //temp.get_patterns(3)
    //temp.get_patterns(4)
    console.log(temp.get_layer_repetition());
    fs.writeFileSync(
        './test_layer_rep.json',
        JSON.stringify(temp.get_layer_repetition(), null, 2)
    );
}
