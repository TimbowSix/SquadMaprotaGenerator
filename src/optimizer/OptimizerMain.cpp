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

int main(void){
    optimizer::RotaOptimizer opt;

    // Generate a seeding-state
    boost::numeric::ublas::matrix<float> state = opt.GenerateSeed(3);
    boost::numeric::ublas::matrix<float> state_buffer(state);
    float current_fit_val = __FLT_MAX__;
    float fit_buffer = 0.0;
    opt.comparisonState = opt.ComparisonState_FromProbabilities({0.9, 0.05, 0.05});

    print_matrix(state);

    // Calculate fit-value
    // Until either maximum iterations or some abort-condition
    for(unsigned i=0; i<opt.iterationMax; i++){   
        // Evolve the state 
        boost::numeric::ublas::matrix<float> evolved_state = opt.Evolve(state_buffer);
        fit_buffer = opt.StateDifference(evolved_state, opt.comparisonState);
        // if the fit-value decreases accept, otherwise only accept with a probability > 0
        std::cout << "fit_value: " << fit_buffer << std::endl;
        if(opt.AcceptMove(fit_buffer-current_fit_val)){
            state = state_buffer;
            current_fit_val = fit_buffer;
            std::cout << "accepted: true" << std::endl;
        }
        else{
            std::cout << "accepted: false" << std::endl;
        }
        std::cout << "accepted_state_fit_value: " << current_fit_val << std::endl;
        // decrease temperature, for T->0 the probability -> 0 thus the algorithm converges to a "hill-climb"
        opt.T0 = opt.UpdateTemperature(opt.T0, 0.00001, i+1);

        // Step to a neighbour state of the previous state (NOT the evolved state!)
        if(i != opt.maxEvolveSteps-1){
            state_buffer = opt.GenerateNeighbour(state, 5, 1);
        }
        std::cout<< "T0: " << opt.T0<<std::endl;
        std::cout << "=====================" << std::endl;
    }
    print_matrix(opt.Evolve(state));

    return 0;
}