#include <iostream>
#include <math.h>
#include <random>
#include <boost/numeric/ublas/matrix.hpp>
#include <fstream>

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
            for(unsigned j=0; j<baseSize; j++){
                mem[k](j) = 0.0;
            }
        }
        mem[0](0) = 1.0;
        return mem;
    }

    boost::numeric::ublas::vector<float> ToBoost(std::vector<float> v_in){
        boost::numeric::ublas::vector<float> v_out(v_in.size());
        for(unsigned i=0; i<v_out.size(); i++){
            v_out(i) = v_in[i];
        }
        return v_out;
    }

    RotaOptimizer::RotaOptimizer(){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        this->generator = std::mt19937(seed);            // the generator seeded with the random device
    };
            
    RotaOptimizer::RotaOptimizer(OptimizerConfig config){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        this->generator = std::mt19937(seed);            // the generator seeded with the random device
        this->kernelSize = config.kernelsize;
        this->maxEvolveSteps = config.maxEvolveSteps;
        this->T0 = config.T0;
        this->T=T0;
        this->iterationMax = config.iterationMax;
        this->slope = config.slope;
        this->stateBaseSize = config.stateBaseSize;
        this->memorykernel = initMem(kernelSize, stateBaseSize);
        this->clusters = config.clusters;
        this->comparisonState = ToBoost(config.mapProbabilities);
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
        float factor_const = 0.00003;//100000000;
        float agentmax = 1.0;
        boost::numeric::ublas::matrix<float> newstate(state);
        for(unsigned i=0; i < newstate.size1(); i++){
            random = newstate(i,0);
            random = distribute(this->generator)*s*factor_const*agentmax;
            newstate(i,0) += random;
            
            // All entries must be positive or zero to be a probability matrix
            if(newstate(i,0) < 0.0){
                SetRow(newstate, i, newstate(i,0)-2*random);
            }
            else{
                SetRow(newstate, i, newstate(i,0));
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

    std::vector<float> MatrixWeights(boost::numeric::ublas::matrix<float> v_in){
        std::vector<float> v_out(v_in.size1());
        for(unsigned i=0; i<v_in.size1(); i++){
            v_out[i] = v_in(i,0);
        }
        return v_out;
    }

    std::vector<float> RotaOptimizer::Run(bool debug){
        if(debug){
            time_t start, end;
            time(&start);
        }

        boost::numeric::ublas::matrix<float> state(this->GenerateSeed(this->stateBaseSize));
        boost::numeric::ublas::vector<float> evolved_state(this->stateBaseSize);
        std::vector<float> diffList(this->stateBaseSize);

        std::ofstream file;
        if(debug){
            file.open("data.dat");
        }

        boost::numeric::ublas::matrix<float> state_buffer(this->GenerateSeed(this->stateBaseSize));

        float current_fit_val = __FLT_MAX__;
        float fit_buffer = 0.0;

        for(unsigned i=0; i<this->iterationMax; i++){
            evolved_state = this->Evolve(state_buffer);
            print_matrix(state_buffer);
            print_vector(evolved_state);
            fit_buffer = this->StateDifference(evolved_state, this->comparisonState, diffList);
            if(debug){
                std::cout << "fit_value: " << fit_buffer << std::endl;
            }
            if(this->AcceptMove(fit_buffer-current_fit_val)){
                state = state_buffer;
                current_fit_val = fit_buffer;
            }

            // Step to a neighbour state of the previous state (NOT the evolved state!)
            if(i != this->maxEvolveSteps-1){
                state_buffer = this->GenerateNeighbour(state, 1, 1, diffList, state);
            }

            this->T = this->UpdateTemperature(this->T0, 0.011, i+1);
            if(debug){
                file << current_fit_val << std::endl;
                std::cout << "accepted_state_fit_value: " << current_fit_val << std::endl;
                std::cout<< "T: " << this->T<<std::endl;
                std::cout << "=====================" << std::endl;
            }
        }

        return MatrixWeights(state);
    }
}