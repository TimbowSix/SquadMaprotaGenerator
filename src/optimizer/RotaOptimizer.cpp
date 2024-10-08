#include <boost/numeric/ublas/matrix.hpp>
#include <fstream>
#include <iostream>
#include <math.h>
#include <random>

#include "RotaOptimizer.hpp"

#define DEBUG false

namespace optimizer {
/// ===== FOR DEBUGGING =====
void print_matrix(boost::numeric::ublas::matrix<float> mat) {
    std::cout << "=====MATRIX=====" << std::endl;
    for (unsigned j = 0; j < mat.size1(); j++) {
        for (unsigned i = 0; i < mat.size2(); i++) {
            std::cout << mat(j, i) << "   ";
        }
        std::cout << std::endl;
    }
}

void print_kernel(std::vector<boost::numeric::ublas::vector<int>> kernel) {
    std::cout << "=====MEMROY KERNEL=====" << std::endl;
    for (unsigned j = 0; j < kernel.size(); j++) {
        for (unsigned i = 0; i < kernel[j].size(); i++) {
            std::cout << kernel[j][i] << "   ";
        }
        std::cout << std::endl;
    }
}

void print_vector(boost::numeric::ublas::vector<float> vec) {
    std::cout << "================" << std::endl;
    for (unsigned i = 0; i < vec.size(); i++) {
        std::cout << vec(i) << std::endl;
    }
}
/// ===== FOR DEBUGGING =====

std::vector<boost::numeric::ublas::vector<int>> initMem(int dim, int baseSize) {
    std::vector<boost::numeric::ublas::vector<int>> mem(dim);
    for (unsigned k = 0; k < dim; k++) {
        mem[k] = boost::numeric::ublas::vector<float>(baseSize);
        for (unsigned j = 0; j < baseSize; j++) {
            mem[k](j) = 0;
        }
    }
    mem[0](0) = 1;
    return mem;
}

RotaOptimizer::RotaOptimizer() {
    std::random_device os_seed; // seed used by the mersenne-twister-engine
    const uint_least32_t seed = os_seed();

    this->generator =
        std::mt19937(seed); // the generator seeded with the random device
};

RotaOptimizer::RotaOptimizer(OptimizerConfig config) {
    std::random_device os_seed; // seed used by the mersenne-twister-engine
    const uint_least32_t seed = os_seed();

    this->generator =
        std::mt19937(seed); // the generator seeded with the random device
    this->kernelSize = config.kernelsize;
    this->maxEvolveSteps = config.maxEvolveSteps;
    this->T0 = config.T0;
    this->T = T0;
    this->iterationMax = config.iterationMax;
    this->slope = config.slope;
    this->stateBaseSize = config.stateBaseSize;
    this->memorykernel = initMem(kernelSize, stateBaseSize);
    this->clusters = config.clusters;
    this->comparisonState = ToBoost(config.mapProbabilities);
    this->mode = config.modename;
};
RotaOptimizer::~RotaOptimizer(){};

boost::numeric::ublas::vector<float>
RotaOptimizer::ToBoost(std::vector<float> v_in) {
    boost::numeric::ublas::vector<float> v_out(v_in.size());
    for (unsigned i = 0; i < v_out.size(); i++) {
        v_out(i) = v_in[i];
    }
    return v_out;
}

void RotaOptimizer::SetRow(boost::numeric::ublas::matrix<float> &mat,
                           int rowindex, float value) {
    for (unsigned i = 0; i < mat.size1(); i++)
        for (unsigned j = 0; j < mat.size2(); j++)
            if (i == rowindex) {
                mat(i, j) = value;
            }
};

float RotaOptimizer::UpdateTemperature(float T0, float s, int i) {
    return T0 * exp(-s * i);
}

float RotaOptimizer::WeightFit(int mapIndex) {
    float f = 0.0;
    float x = this->clusters[mapIndex].size();
    float y = this->comparisonState(mapIndex);
    // This is a fourth-order polynomial fit for weight=f(#neighbours, p_map)
    // The parameters where obtained using MATLAB, but you can use any fitting
    // tool you like It is of upmost importance to do it this way to obtain a
    // suitable starting point for the optimizer

    if (this->mode == "RAAS") {
        f = 0.0 + 0.07638 * x + 5.432 * y - 0.06782 * pow(x, 2) +
            3.661 * x * y + 2.13 * pow(y, 2) + 0.0188 * pow(x, 3) -
            0.349 * pow(x, 2) * y - 6.438 * x * pow(y, 2) -
            0.001654 * pow(x, 4) + 0.05273 * pow(x, 3) * y -
            1.985 * pow(x, 2) * pow(y, 2);
    } else if (this->mode == "AAS") {
        f = 0.0 - 0.2882 * x + 8.021 * y + 0.1124 * pow(x, 2) + 4.775 * x * y -
            15.57 * pow(y, 2) - 0.01256 * pow(x, 3) - 0.5402 * pow(x, 2) * y -
            13.88 * x * pow(y, 2);
    } else if (this->mode == "Invasion") {
        f = 0.0 + 0.1671 * x + 7.922 * y - 0.08273 * pow(x, 2) + 2.305 * x * y +
            0.8738 * pow(y, 2) + 0.01196 * pow(x, 3) + 0.2367 * pow(x, 2) * y -
            17.07 * x * pow(y, 2);
    } else if (this->mode == "TC") {
        f = 0.0 + 0.1917 * x + 8.706 * y - 0.09457 * pow(x, 2) + 2.233 * x * y -
            13.64 * pow(y, 2) + 0.01413 * pow(x, 3) + 0.02096 * pow(x, 2) * y -
            11.38 * x * pow(y, 2);
    } else if (this->mode == "Insurgency") {
        f = 0.0 - 0.05491 * x + 4.539 * y + 0.02275 * pow(x, 2) +
            1.456 * x * y - 4.496 * pow(y, 2) - 0.001918 * pow(x, 3) -
            0.02101 * pow(x, 2) * y - 2.708 * x * pow(y, 2);
    } else if (this->mode == "Destruction") {
        f = 0.0 + 0.06 * x + 2.207 * y;
    } else {
        f = 0.0 + 0.07638 * x + 5.432 * y - 0.06782 * pow(x, 2) +
            3.661 * x * y + 2.13 * pow(y, 2) + 0.0188 * pow(x, 3) -
            0.349 * pow(x, 2) * y - 6.438 * x * pow(y, 2) -
            0.001654 * pow(x, 4) + 0.05273 * pow(x, 3) * y -
            1.985 * pow(x, 2) * pow(y, 2);
    }
    return f;
}

boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateSeed(int dim) {
    std::uniform_real_distribution<float> distribute(
        0, 1); // uniform-dist wrapper for rng, 1 is excluded
    boost::numeric::ublas::matrix<float> mat(dim, dim);
    float f = 0;
    for (unsigned i = 0; i < mat.size1(); ++i) {
        f = this->WeightFit(i);
        SetRow(mat, i, f);
    }
    return MatrixToProbabilityMatrix(mat);
}

float RotaOptimizer::StateDifference(
    boost::numeric::ublas::vector<float> &state1,
    boost::numeric::ublas::vector<float> &state2, std::vector<float> &list) {
    float sum = 0.0;
    float x = 0.0;
    // check dimension match
    for (unsigned i = 0; i < state1.size(); ++i) {
        // take only the first column and calculate the difference squared
        // this is possible for EVOLVED STATES only because the columns of the
        // initial matrices converge to the long-term probabilities
        x = pow(state1(i) - state2(i), 2);
        sum += x;
        list[i] = x;
    }
    return sum;
};

boost::numeric::ublas::matrix<float> RotaOptimizer::MatrixToProbabilityMatrix(
    boost::numeric::ublas::matrix<float> mat) {
    float sum;
    for (unsigned j = 0; j < mat.size2(); ++j) {
        sum = 0.0;
        for (unsigned i = 0; i < mat.size1(); ++i)
            sum += mat(i, j);
        for (unsigned i = 0; i < mat.size1(); ++i)
            if (mat(i, j) != 0.0)
                mat(i, j) /= sum;
    }
    return mat;
};

boost::numeric::ublas::matrix<float>
RotaOptimizer::GenerateNeighbour(boost::numeric::ublas::matrix<float> &state,
                                 float s, float T,
                                 std::vector<float> &grid_fitness,
                                 boost::numeric::ublas::matrix<float> &agent) {
    std::uniform_real_distribution<> distribute(-1, 1);
    // float exponent = 1.0/16.0;
    float random;
    float factor_const = 0.007; // 0.00003
    boost::numeric::ublas::matrix<float> newstate(state);
    for (unsigned i = 0; i < newstate.size1(); i++) {
        random = newstate(i, 0);
        random = distribute(this->generator) * s * factor_const;
        newstate(i, 0) += random * grid_fitness[i];

        // All entries must be positive or zero to be a probability matrix
        if (newstate(i, 0) < 0.0) {
            SetRow(newstate, i, newstate(i, 0) - 2 * random);
        } else {
            SetRow(newstate, i, newstate(i, 0));
        }
    }
    return MatrixToProbabilityMatrix(newstate);
};

boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateNeighbourAxis(
    boost::numeric::ublas::matrix<float> &state, float s, float T,
    std::vector<float> &grid_fitness,
    boost::numeric::ublas::matrix<float> &agent) {
    std::uniform_real_distribution<> distribute(-1, 1);
    std::uniform_int_distribution<> distInt(0, state.size1() - 1);
    int axis = distInt(this->generator);
    // float exponent = 1.0/16.0;
    float random;
    float factor_const = 0.007; // 0.00003
    boost::numeric::ublas::matrix<float> newstate(state);

    random = newstate(axis, 0);
    random = distribute(this->generator) * s * factor_const;
    newstate(axis, 0) += random * grid_fitness[axis];

    // All entries must be positive or zero to be a probability matrix
    if (newstate(axis, 0) < 0.0) {
        SetRow(newstate, axis, newstate(axis, 0) - 2 * random);
    } else {
        SetRow(newstate, axis, newstate(axis, 0));
    }

    return MatrixToProbabilityMatrix(newstate);
};

void RotaOptimizer::UpdateMemoryKernel(
    int index, std::vector<boost::numeric::ublas::vector<int>> &kernel) {
    // cycle kernel
    for (unsigned i = kernelSize; i > 0; i--) {
        for (unsigned j = 0; j < stateBaseSize; j++) {
            if (i > 1) {
                kernel[i - 1](j) = kernel[i - 2](j);
            } else {
                if (j == index) {
                    kernel[0](index) = 1;
                } else {
                    kernel[0](j) = 0;
                }
            }
        }
    }
};

int RotaOptimizer::choose_vector(boost::numeric::ublas::vector<float> &v,
                                 float r) {
    float temp = 0.0;
    bool found = false;
    int i = 0;
    while (!found && i < v.size()) {
        temp += v(i);
        if (!(temp < r)) {
            v(i) = 0.0;
            found = true;
        }
        i++;
    }
    return i - 1;
    // for (unsigned i = 0; i < v.size(); i++) {
    //     temp += v(i);
    //     if (temp < r || temp >= r && found) {
    //         v(i) = 0.0;
    //     } else {
    //         v(i) = 1.0;
    //         found = true;
    //     }
    // }
}

boost::numeric::ublas::vector<float>
RotaOptimizer::Evolve(boost::numeric::ublas::matrix<float> &state) {
    int mapindex = -1;
    std::uniform_real_distribution<float> distribute(0, 1);
    boost::numeric::ublas::matrix<float> temp(state);
    boost::numeric::ublas::vector<float> copy_vector(stateBaseSize);
    boost::numeric::ublas::vector<float> count_vector(stateBaseSize);
    for (unsigned i = 0; i < count_vector.size(); i++) {
        count_vector(i) = 0.0;
    }

    for (unsigned i = 0; i < this->maxEvolveSteps; i++) {
        temp = state;
        for (unsigned j = 0; j < kernelSize; j++) {
            for (unsigned k = 0; k < stateBaseSize; k++) {
                if (memorykernel[j][k] != 0.0) {
                    for (unsigned h = 0; h < clusters[k].size(); h++)
                        this->SetRow(temp, clusters[k][h], 0.0);
                }
            }
        }
        temp = MatrixToProbabilityMatrix(temp);
        copy_vector = boost::numeric::ublas::prod(temp, memorykernel[0]);
        mapindex = choose_vector(copy_vector, distribute(this->generator));
        UpdateMemoryKernel(mapindex, memorykernel);
        count_vector(mapindex) += 1.0;
    }
    return count_vector * (1 / ((float)maxEvolveSteps));
};

bool RotaOptimizer::AcceptMove(float fitvalue_difference) {
    std::uniform_real_distribution<float> distribute(0, 1);

    return fitvalue_difference < 0 ||
           exp(-fitvalue_difference / this->T) > distribute(this->generator);
};

std::vector<float> MatrixWeights(boost::numeric::ublas::matrix<float> v_in) {
    std::vector<float> v_out(v_in.size1());
    for (unsigned i = 0; i < v_in.size1(); i++) {
        v_out[i] = v_in(i, 0);
    }
    return v_out;
}

std::vector<float> RotaOptimizer::Run(bool debug) {
#if DEBUG
    if (debug) {
        time_t start, end;
        time(&start);
    }
    std::ofstream file;
    file.open("data.dat");
#endif

    boost::numeric::ublas::matrix<float> state(
        this->GenerateSeed(this->stateBaseSize));
    boost::numeric::ublas::vector<float> evolved_state(this->stateBaseSize);
    std::vector<float> diffList(this->stateBaseSize);

    boost::numeric::ublas::matrix<float> state_buffer(
        this->GenerateSeed(this->stateBaseSize));

    float current_fit_val = __FLT_MAX__;
    float fit_buffer = 0.0;

    for (unsigned i = 0; i < this->iterationMax; i++) {
        evolved_state = this->Evolve(state_buffer);
        fit_buffer = this->StateDifference(evolved_state, this->comparisonState,
                                           diffList);
#if DEBUG
        std::cout << "fit_value: " << fit_buffer << std::endl;
#endif

        if (this->AcceptMove(fit_buffer - current_fit_val)) {
            state = state_buffer;
            current_fit_val = fit_buffer;
        }

        // Step to a neighbour state of the previous state (NOT the evolved
        // state!)
        if (i != this->maxEvolveSteps - 1) {
            state_buffer =
                this->GenerateNeighbourAxis(state, 1, 1, diffList, state);
        }

        this->T = this->UpdateTemperature(
            this->T0, 0.0003, i + 1); // 0.011, 0.001 destrc at 10^5 iterMax
#if DEBUG
        file << current_fit_val << std::endl;
        std::cout << "accepted_state_fit_value: " << current_fit_val
                  << std::endl;
        std::cout << "T: " << this->T << std::endl;
        std::cout << "=====================" << std::endl;
#endif
    }

    return MatrixWeights(state);
}
} // namespace optimizer