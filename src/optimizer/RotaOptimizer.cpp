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

    void print_kernel(std::vector<boost::numeric::ublas::vector<float>> kernel){
        std::cout << "=====MEMROY KERNEL=====" << std::endl;
        for(unsigned j=0; j<kernel.size(); j++){
            for(unsigned i=0; i<kernel[j].size(); i++){
                std::cout << kernel[j][i] << "   ";
            }
            std::cout << std::endl;
        }
    }

    std::vector<boost::numeric::ublas::vector<float>> initMem(int dim, int baseSize){
        std::vector<boost::numeric::ublas::vector<float>> mem(dim);
        for(unsigned k=0; k<dim; k++){
            mem[k] = boost::numeric::ublas::vector<float>(baseSize);
        }
        mem[0](0) = 1.0;
        return mem;
    }

    RotaOptimizer::RotaOptimizer(){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        generator = std::mt19937(seed);            // the generator seeded with the random device
        kernelSize = 4;
        maxEvolveSteps = 1000;
        T0 = 0.03;
        T=T0;
        stateBaseSize = 22;
        iterationMax = 1000;
        slope = 0.05;
        memorykernel = initMem(kernelSize, stateBaseSize);
        clusters = {
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

    };
    RotaOptimizer::~RotaOptimizer(){

    };

    void SetRow(boost::numeric::ublas::matrix<float>& mat, int rowindex, float value){
        for(unsigned i=0; i < mat.size1(); i++)
            for(unsigned j=0; j < mat.size2(); j++)
                if(i==rowindex){
                    mat(i,j) = value;
                }
    };

    float RotaOptimizer::UpdateTemperature(float T0, float s, int i){
        return T0*exp(-s*i);
        // return T0*(1-s*i);
    }

    float WeightFit(float x,float y){
        float f = 0.0;
        f =  -0.156
            +0.1269*x
            +29.99*y
            -0.03288*pow(x,2)
            -17.62*x*y
            -426.5*pow(y,2)
            +0.004327*pow(x,3)
            +3.779*pow(x,2)*y
            +301.7*x*pow(y,2)
            +2467*pow(y,3)
            -0.0003173*pow(x,4)
            -0.1802*pow(x,3)*y
            -33.03*pow(x,2)*pow(y,2)
            -1210*x*pow(y,3)
            -4025*pow(y,4);
        // f =  -0.1029
        //     +0.04482*x
        //     +28.93*y
        //     -0.01165*pow(x,2)
        //     -9.405*x*y
        //     -1049*pow(y,2)
        //     -0.006852*pow(x,3)
        //     -0.4641*pow(x,2)*y
        //     +485.9*x*pow(y,2)
        //     +1640*pow(y,3)
        //     +0.000747*pow(x,4)
        //     +0.6061*pow(x,3)*y
        //     -40.78*pow(x,2)*pow(y,2)
        //     -5069*x*pow(y,3)
        //     -10530*pow(y,4)
        //     -0.05659*pow(x,4)*y
        //     -0.0257*pow(x,3)*pow(y,2)
        //     +193.4*pow(x,2)*pow(y,3)
        //     +1634*x*pow(y,4)
        //     +23500*pow(y,5);
        return f;
    }

    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateSeed(int dim){
        std::uniform_real_distribution<float> distribute(0,1);   //uniform-dist wrapper for rng, 1 is excluded
        boost::numeric::ublas::matrix<float> mat (dim, dim);
        float f = 0;
        for (unsigned i = 0; i < mat.size1 (); ++ i){
            //SetRow(mat, i, distribute(this->generator));
            f=WeightFit(clusters[i].size(),comparisonState(i));
            SetRow(mat, i, f);
        }
        return MatrixToProbabilityMatrix(mat);
    }

    float RotaOptimizer::StateDifference(boost::numeric::ublas::vector<float> state1, boost::numeric::ublas::vector<float> state2){
        float sum = 0.0;
        // check dimension match
        for (unsigned i = 0; i < state1.size(); ++ i)
            // take only the first column and calculate the difference squared
            // this is possible for EVOLVED STATES only because the columns of the initial matrices converge to the long-term probabilities
            sum += pow(state1(i)-state2(i), 2);
        return sum;
    };

    float RotaOptimizer::StateDifference(boost::numeric::ublas::vector<float> state1, boost::numeric::ublas::vector<float> state2, std::vector<float>& list){
        float sum = 0.0;
        float x = 0.0;
        // check dimension match
        for (unsigned i = 0; i < state1.size(); ++ i){
            // take only the first column and calculate the difference squared
            // this is possible for EVOLVED STATES only because the columns of the initial matrices converge to the long-term probabilities
            x = pow(state1(i)-state2(i), 2);
            sum += x;
            list[i] = x;
        }
        return sum;
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
                newstate(i,0) += random*s;
                // All entries must be positive or zero to be a probability matrix
                if(newstate(i,0) < 0.0){
                    newstate(i,0) -= 2*random*s;
                }
            }
        }
        return MatrixToProbabilityMatrix(newstate);
    };

    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T, std::vector<float> grid_fitness){
        std::uniform_real_distribution<> distribute(0,1);
        float exponent = 1.0/16.0;
        float random;
        float factor_const = 1000000;
        boost::numeric::ublas::matrix<float> newstate(state);
        for(unsigned i=0; i < newstate.size1(); i++){
            for(unsigned j=0; j < newstate.size2(); j++){
                random = distribute(this->generator)*s*pow(grid_fitness[i],exponent)*factor_const;
                newstate(i,j) += random;
                // All entries must be positive or zero to be a probability matrix
                if(newstate(i,j) < 0.0){
                    newstate(i,j) -= 2*random;
                }
            }
        }
        return MatrixToProbabilityMatrix(newstate);
    };


    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T, std::vector<float> grid_fitness, boost::numeric::ublas::matrix<float>& agent){
        std::uniform_real_distribution<> distribute(-1,1);
        float exponent = 1.0/16.0;
        float random;
        float factor_const = 0.0003;//100000000;
        float agentmax = 0.0;
        boost::numeric::ublas::matrix<float> newstate(state);
        for(unsigned i=0; i < newstate.size1(); i++){
            if(grid_fitness[i]>0.000000001){
                agentmax = abs(newstate(i,0) - agent(i,0));
                random = newstate(i,0);
                // factor_const = atanh(grid_fitness[i]/(*std::max_element(grid_fitness.begin(), grid_fitness.end())) - 0.01);//pow(grid_fitness[i],exponent);
                random = distribute(this->generator)*s*factor_const*agentmax;
                newstate(i,0) += random;
                // All entries must be positive or zero to be a probability matrix
                if(newstate(i,0) < 0.0){
                    SetRow(newstate, i, newstate(i,0)-2*random);
                }
                else{
                    SetRow(newstate, i, newstate(i,0));
                }
                // std::cout << "===================" << std::endl;
                // std::cout << "Factor: " << factor_const << std::endl;
                // std::cout << "diff: " << i << " | " << grid_fitness[i] << std:: endl;
            }
            else{
                std::cout << "HOLD POSITION " << i << std::endl;
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

    void RotaOptimizer::UpdateMemoryKernel(boost::numeric::ublas::vector<float>& evolvedState, std::vector<boost::numeric::ublas::vector<float>>& kernel){
        // cycle kernel 
        for(unsigned i=kernelSize; i>0; i--){
            for(unsigned j=0; j<stateBaseSize; j++){
                if(i>1){
                    kernel[i-1](j) = kernel[i-2](j);
                }
                else{
                    kernel[0](j) = evolvedState(j);
                }

            }
        }
    };

    void choose_vector(boost::numeric::ublas::vector<float>& v, float r){
        float temp = 0.0;
        int index = 0;
        bool found = false;
        for(unsigned i=0; i<v.size(); i++){
            temp += v(i);
            if(temp < r || temp >= r && found){
                v(i) = 0.0;
            }
            else{
                v(i) = 1.0;
                found = true;
            }
        }
    }
void print_vector(boost::numeric::ublas::vector<float> vec){
    for(unsigned i=0; i<vec.size(); i++){
        std::cout << vec(i) << std::endl;
    }
}
    boost::numeric::ublas::vector<float> RotaOptimizer::Evolve(boost::numeric::ublas::matrix<float>& state){
        std::uniform_real_distribution<float> distribute(0,1); 
        boost::numeric::ublas::matrix<float> temp(state);
        boost::numeric::ublas::matrix<float> state_copy(state);
        boost::numeric::ublas::vector<float> copy_vector(stateBaseSize);
        boost::numeric::ublas::vector<float> count_vector(stateBaseSize);
        for(unsigned i=0; i<count_vector.size(); i++){
            count_vector(i) = 0.0;
        }
        // Init trafo matrix as zero-matrix
        boost::numeric::ublas::matrix<float> trafo(state.size1(), state.size2());
        for(unsigned i=0; i<state.size1(); i++)
            for(unsigned j=0; j<state.size2(); j++)
                trafo(i,j) = 0.0;

        float factor = 0.0;
        if(kernelSize == 0){// no memory kernel, only matmul necessary for state evolution
            // for(unsigned i=0; i<this->maxEvolveSteps; i++)
            //     temp = boost::numeric::ublas::prod(state, temp);
        }
        else    // finite, non-vanishing memory kernel
        {
            for(unsigned i=0; i<this->maxEvolveSteps; i++){
                temp = state;
                for(unsigned j=0; j<kernelSize; j++){
                    for(unsigned k=0; k<stateBaseSize; k++){
                        if(memorykernel[j][k] != 0.0){
                            for(unsigned h=0; h<clusters[k].size(); h++)
                                SetRowZero(temp, clusters[k][h]);
                        }
                    }
                }
                temp = MatrixToProbabilityMatrix(temp);
                copy_vector = boost::numeric::ublas::prod(temp, memorykernel[0]);
                choose_vector(copy_vector, distribute(this->generator));
                UpdateMemoryKernel(copy_vector, memorykernel);
                count_vector += copy_vector;
            }
        }
        return count_vector*(1/((float)maxEvolveSteps));
    };

    bool RotaOptimizer::AcceptMove(float fitvalue_difference){
        std::uniform_real_distribution<float> distribute(0,1); 

        return fitvalue_difference < 0 || exp(-fitvalue_difference/this->T) > distribute(this->generator);
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