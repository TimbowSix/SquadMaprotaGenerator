#include <iostream>
#include <math.h>
#include <random>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    RotaOptimizer::RotaOptimizer(){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        generator = std::mt19937(seed);            // the generator seeded with the random device

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
                newstate(i,j) += random;
                // All entries must be positive or zero to be a probability matrix
                if(newstate(i,j) < 0.0){
                    newstate(i,j) -= 2*random;
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
}