#include <iostream>
#include <math.h>
#include <random>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    void print_matrix(boost::numeric::ublas::matrix<float> mat){
        std::cout << "=====MATRIX=====" << std::endl;    
        for(unsigned j=0; j<mat.size1(); j++){
            for(unsigned i=0; i<mat.size2(); i++){
                    std::cout << mat(j,i) << "   ";
                }
                std::cout << std::endl;
            }
        }

    void print_kernel(std::vector<std::vector<float>> kernel){
        std::cout << "=====MEMROY KERNEL=====" << std::endl;
        for(unsigned j=0; j<kernel.size(); j++){
            for(unsigned i=0; i<kernel[j].size(); i++){
                std::cout << kernel[j][i] << "   ";
            }
            std::cout << std::endl;
        }
    }

    RotaOptimizer::RotaOptimizer(){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        generator = std::mt19937(seed);            // the generator seeded with the random device
        kernelSize = 3;
        maxEvolveSteps = 2;
        T0 = 50.0;
        stateBaseSize = 5;
        iterationMax = 2;
        slope = 0.05;
        memorykernel = {  
            {0.0, 0.0, 1.0, 0.0, 0.0},
            {0.0, 0.0, 0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0, 0.0, 0.0}};
        clusters = {
            {0, {0, 1}}, 
            {1, {1, 0}},
            {2, {2}},
            {3, {3}},
            {4, {4}}};

    };
    RotaOptimizer::~RotaOptimizer(){

    };

    float RotaOptimizer::UpdateTemperature(float T0, float s, int i){
        return T0*exp(-s*i);
    }

    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateSeed(int dim){
        std::uniform_real_distribution<> distribute(0,1);   //uniform-dist wrapper for rng, 1 is excluded
        boost::numeric::ublas::matrix<float> mat (dim, dim);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = distribute(this->generator);

        return MatrixToProbabilityMatrix(mat);
    }

    float RotaOptimizer::StateDifference(boost::numeric::ublas::matrix<float> state1, boost::numeric::ublas::matrix<float> state2){
        float sum = 0.0;
        // check dimension match
        if(state1.size1() == state2.size1() && state1.size2() == state2.size2()){
            for (unsigned i = 0; i < state1.size1(); ++ i)
                // take only the first column and calculate the difference squared
                // this is possible for EVOLVED STATES only because the columns of the initial matrices converge to the long-term probabilities
                sum += pow(state1(i,0)-state2(i,0), 2);
            return sum;
        }
        else{
            std::cout << "dim missmatch" << std::endl;
            return -1.0;
        }


    };

    boost::numeric::ublas::matrix<float> RotaOptimizer::MatrixToProbabilityMatrix(boost::numeric::ublas::matrix<float> mat){
        float sum;
        for(unsigned j = 0; j < mat.size2(); ++ j){
            sum = 0.0;
            for(unsigned i = 0; i < mat.size1(); ++ i)
                sum += mat(i,j);
            for(unsigned i = 0; i < mat.size1(); ++ i)
                if(mat(i,j) != 0.0)
                    mat(i,j)/=sum;
        }
        return mat;
    };

    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T){
        std::uniform_real_distribution<> distribute(0,s);
        float random;
        boost::numeric::ublas::matrix<float> newstate(state);
        for(unsigned i=0; i < newstate.size1(); i++){
            for(unsigned j=0; j < newstate.size2(); j++){
                random = distribute(this->generator);
                newstate(i,j) += random*s;
                // All entries must be positive or zero to be a probability matrix
                if(newstate(i,j) < 0.0){
                    newstate(i,j) -= 2*random*s;
                }
            }
        }
        return MatrixToProbabilityMatrix(newstate);
    };

    void RotaOptimizer::SetRowZero(boost::numeric::ublas::matrix<float>& mat, int rowindex){
        for(unsigned i=0; i < mat.size1(); i++)
            for(unsigned j=0; j < mat.size2(); j++)
                if(i==rowindex){
                    mat(i,j) = 0.0;
                }
    };

    void RotaOptimizer::UpdateMemoryKernel(boost::numeric::ublas::matrix<float>& evolvedState, std::vector<std::vector<float>>& kernel){
        std::vector<float> column(evolvedState.size1());
        // get the first column
        for(unsigned i=0; i<evolvedState.size1(); i++){
            column[i] = evolvedState(i,0);
        }

        // cycle kernel 
        for(unsigned i=kernel.size(); i>0; i--){
            for(unsigned j=0; j<kernel.size(); j++){
                if(i>1){
                    kernel[i-1][j] = kernel[i-2][j];
                }
                else{
                    kernel[0][j] = column[j];
                }

            }
        }
    };

    boost::numeric::ublas::matrix<float> RotaOptimizer::Evolve(boost::numeric::ublas::matrix<float>& state){
        boost::numeric::ublas::matrix<float> temp(state);
        // Init trafo matrix as zero-matrix
        boost::numeric::ublas::matrix<float> trafo(state.size1(), state.size2());
        for(unsigned i=0; i<state.size1(); i++)
            for(unsigned j=0; j<state.size2(); j++)
                trafo(i,j) = 0.0;

        float factor = 0.0;
        if(kernelSize == 0){// no memory kernel, only matmul necessary for state evolution
            for(unsigned i=0; i<this->maxEvolveSteps; i++)
                temp = boost::numeric::ublas::prod(state, temp);
        }
        else    // finite, non-vanishing memory kernel
        {
            for(unsigned i=0; i<this->maxEvolveSteps; i++){
                trafo = 0.0*trafo;
                temp = state;
                for(unsigned j=0; j<this->stateBaseSize; j++){
                    for(unsigned k=0; k<this->kernelSize; k++){
                        factor = this->memorykernel[k][j]/this->kernelSize;
                        if(factor != 0.0){
                            for(unsigned m=0; m<clusters[j].size(); m++){
                                SetRowZero(temp, m);
                            }
                            trafo += factor*temp;
                            temp = state;
                        }
                    }
                }
                trafo = prod(MatrixToProbabilityMatrix(trafo),state);
                print_matrix(trafo);
                UpdateMemoryKernel(trafo, memorykernel);
                print_kernel(memorykernel);
            }
        }
        return trafo;
    };

    bool RotaOptimizer::AcceptMove(float fitvalue_difference){
        std::uniform_real_distribution<> distribute(0,1); 

        return fitvalue_difference < 0 || exp(-fitvalue_difference/this->T0) > distribute(this->generator);
    };

    boost::numeric::ublas::matrix<float> RotaOptimizer::ComparisonState_FromProbabilities(std::vector<float> probabilities){
        boost::numeric::ublas::matrix<float> mat(probabilities.size(),probabilities.size());
        for(unsigned i=0; i<mat.size2(); i++){
            for(unsigned j=0; j<probabilities.size(); j++){
                mat(j,i) = probabilities[j];
            }
        }
        return mat;
    };
}