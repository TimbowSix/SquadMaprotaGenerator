#include <bits/stdc++.h>
#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"

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
    optimizer::RotaOptimizer opt;

    // Generate a seeding-state
    boost::numeric::ublas::matrix<float> state = opt.GenerateSeed(10);
    boost::numeric::ublas::matrix<float> state_buffer(state);
    float current_fit_val = __FLT_MAX__;
    float fit_buffer = 0.0;
    opt.comparisonState = boost::numeric::ublas::vector<float>(10);
    opt.comparisonState(0) = 0.1;
    opt.comparisonState(1) = 0.15;
    opt.comparisonState(2) = 0.05;
    opt.comparisonState(3) = 0.05;
    opt.comparisonState(4) = 0.03;
    opt.comparisonState(5) = 0.06;
    opt.comparisonState(6) = 0.01;
    opt.comparisonState(7) = 0.05;
    opt.comparisonState(8) = 0.1;
    opt.comparisonState(9) = 0.4;

    print_matrix(state);
    bool DEBUG = true;
    // Calculate fit-value
    // Until either maximum iterations or some abort-condition
    for(unsigned i=0; i<opt.iterationMax; i++){   
        // Evolve the state 
        boost::numeric::ublas::vector<float> evolved_state = opt.Evolve(state_buffer);
        fit_buffer = opt.StateDifference(evolved_state, opt.comparisonState);
        
        // if the fit-value decreases accept, otherwise only accept with a probability > 0
        if(DEBUG)
            std::cout << "fit_value: " << fit_buffer << std::endl;
        if(opt.AcceptMove(fit_buffer-current_fit_val)){
            state = state_buffer;
            current_fit_val = fit_buffer;
        }
        if(DEBUG)
            std::cout << "accepted_state_fit_value: " << current_fit_val << std::endl;

        // decrease temperature, for T->0 the probability -> 0 thus the algorithm converges to a "hill-climb"
        opt.T0 = opt.UpdateTemperature(opt.T0, 0.000005, i+1);

        // Step to a neighbour state of the previous state (NOT the evolved state!)
        if(i != opt.maxEvolveSteps-1){
            state_buffer = opt.GenerateNeighbour(state, std::max(opt.T0,(float)1E-5), 1);
        }
        if(DEBUG){
            std::cout<< "T0: " << opt.T0<<std::endl;
            std::cout << "=====================" << std::endl;
        }
    }
    time(&end);
    boost::numeric::ublas::vector<float> s = opt.Evolve(state);
    print_vector(s);
    double time_taken = double(end - start);
    std::cout << "Time taken by program is : " << time_taken << std::endl;
    std::cout << " sec " << std::endl;
    return 0;
}