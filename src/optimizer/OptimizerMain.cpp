#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"
#include "OptimizerConfig.hpp"

void print_matrix(boost::numeric::ublas::matrix<float> mat){
    for(unsigned j=0; j<mat.size1(); j++){
        for(unsigned i=0; i<mat.size2(); i++){
                std::cout << mat(j,i) << "   ";
            }
            std::cout << std::endl;
        }
}
void print_vector(boost::numeric::ublas::vector<float> vec){
    for(unsigned i=0; i<vec.size(); i++){
        std::cout << vec(i) << std::endl;
    }
}


int main(void){
    time_t start, end;
    time(&start);

    std::vector<float> comparisonState(22);
    comparisonState[0] = 0.000331373;
    comparisonState[1] = 0.019808;
    comparisonState[2] = 0.0793753;
    comparisonState[3] = 0.0855731;
    comparisonState[4] = 0.0303223;
    comparisonState[5] = 0.0581194;
    comparisonState[6] = 0.0247349;
    comparisonState[7] = 0.0852877;
    comparisonState[8] = 0.085538;
    comparisonState[9] = 0.0843088;
    comparisonState[10] = 0.00304394;
    comparisonState[11] = 0.0459895;
    comparisonState[12] = 0.0221827;
    comparisonState[13] = 0.0156107;
    comparisonState[14] = 0.000810466;
    comparisonState[15] = 0.00351631;
    comparisonState[16] = 0.0843088;
    comparisonState[17] = 0.0855718;
    comparisonState[18] = 0.0793753;
    comparisonState[19] = 0.000810466;
    comparisonState[20] = 0.019808;
    comparisonState[21] = 0.085573;

    std::map<int, std::vector<int>> clusters = {
            {0, {0}}, 
            {1, {1,11}},
            {2, {2}},
            {3, {3, 9, 18, 21}},
            {4, {4, 12}},
            {5, {5}},
            {6, {6, 15}},
            {7, {7}},
            {8, {8, 3, 9}},
            {9, {9, 3, 8}},
            {10, {10}}, 
            {11, {1}},
            {12, {12, 4}},
            {13, {13}},
            {14, {14}},
            {15, {15, 6}},
            {16, {16}},
            {17, {17}},
            {18, {18}},
            {19, {19, 5, 4}},
            {20, {20}},
            {21, {21, 3, 9}},
            };

    optimizer::OptimizerConfig config(22, 4, clusters, comparisonState);

    optimizer::RotaOptimizer opt(config);

    // Generate a seeding-state
    float current_fit_val = __FLT_MAX__;
    float fit_buffer = 0.0;
    int number_agents = 2;


    std::vector<boost::numeric::ublas::matrix<float>> agents(number_agents);

    // boost::numeric::ublas::matrix<float> state = opt.GenerateSeed(10);
    boost::numeric::ublas::matrix<float> state_buffer(agents[0]);
    opt.comparisonState = boost::numeric::ublas::vector<float>(22);
    opt.comparisonState(0) = 0.000331373;
    opt.comparisonState(1) = 0.019808;
    opt.comparisonState(2) = 0.0793753;
    opt.comparisonState(3) = 0.0855731;
    opt.comparisonState(4) = 0.0303223;
    opt.comparisonState(5) = 0.0581194;
    opt.comparisonState(6) = 0.0247349;
    opt.comparisonState(7) = 0.0852877;
    opt.comparisonState(8) = 0.085538;
    opt.comparisonState(9) = 0.0843088;
    opt.comparisonState(10) = 0.00304394;
    opt.comparisonState(11) = 0.0459895;
    opt.comparisonState(12) = 0.0221827;
    opt.comparisonState(13) = 0.0156107;
    opt.comparisonState(14) = 0.000810466;
    opt.comparisonState(15) = 0.00351631;
    opt.comparisonState(16) = 0.0843088;
    opt.comparisonState(17) = 0.0855718;
    opt.comparisonState(18) = 0.0793753;
    opt.comparisonState(19) = 0.000810466;
    opt.comparisonState(20) = 0.019808;
    opt.comparisonState(21) = 0.085573;
    for(unsigned i=0; i<number_agents; i++){
        agents[i] = opt.GenerateSeed(22);
    }
    std::vector<float> diffList(22);
    print_matrix(agents[0]);
    std::ofstream file;
    file.open("data.dat");
    bool DEBUG = true;
    int agent_index = 0;
    std::random_device os_seed;             // seed used by the mersenne-twister-engine
    const uint_least32_t seed = os_seed();  

    std::mt19937 gen = std::mt19937(seed);            // the generator seeded with the random device
    std::uniform_int_distribution<> dist_uniform(0, number_agents-1);
    // Calculate fit-value
    // Until either maximum iterations or some abort-condition
    for(unsigned i=0; i<opt.iterationMax; i++){ 
        for(unsigned j=0; j<number_agents; j++){
            agent_index = j;
            if(number_agents>1){
                while(agent_index == j){
                    agent_index = dist_uniform(gen);
                }
            }

            state_buffer = agents[j];
            // Evolve the state 
            boost::numeric::ublas::vector<float> evolved_state = opt.Evolve(state_buffer);
            fit_buffer = opt.StateDifference(evolved_state, opt.comparisonState, diffList);
            // if the fit-value decreases accept, otherwise only accept with a probability > 0
            if(DEBUG)
                std::cout << "fit_value: " << fit_buffer << std::endl;
            if(opt.AcceptMove(fit_buffer-current_fit_val)){
                agents[j] = state_buffer;
                current_fit_val = fit_buffer;
            }
            if(j==0){
                file << current_fit_val << std::endl;
            }
            if(DEBUG)
                std::cout << "accepted_state_fit_value: " << current_fit_val << std::endl;

            // decrease temperature, for T->0 the probability -> 0 thus the algorithm converges to a "hill-climb"

            // Step to a neighbour state of the previous state (NOT the evolved state!)
            if(i != opt.maxEvolveSteps-1){
                state_buffer = opt.GenerateNeighbour(agents[j], 1, 1, diffList, agents[agent_index]);
            }
            if(DEBUG){
                std::cout<< "T: " << opt.T<<std::endl;
                std::cout << "=====================" << std::endl;
            }
        opt.T = opt.UpdateTemperature(opt.T0, 0.011, i+1);//1.0/((float)opt.iterationMax), i+1);// 0.015, i+1);
        }
    }
    std::cout << "============================" << std::endl;
    print_matrix(agents[0]);
    time(&end);
    boost::numeric::ublas::vector<float> s = opt.Evolve(agents[0]);
    print_vector(s);
    double time_taken = double(end - start);
    std::cout << "Time taken by program is : " << time_taken << std::endl;
    std::cout << " sec " << std::endl;
    file.close();
    return 0;
}