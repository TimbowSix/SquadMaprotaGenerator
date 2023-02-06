#pragma once

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <iostream>
#include <map>
#include <random>

#include "OptimizerConfig.hpp"
namespace optimizer {
/// @brief Optimizer Class
class RotaOptimizer {
  private:
    int kernelSize;
    int stateBaseSize;
    float slope;
    std::mt19937 generator;
    std::mt19937 *generator_ptr;
    std::vector<boost::numeric::ublas::vector<int>> memorykernel;
    float WeightFit(int mapIndex);
    void SetRow(boost::numeric::ublas::matrix<float> &mat, int rowindex,
                float value);
    boost::numeric::ublas::vector<float> ToBoost(std::vector<float> v_in);
    int choose_vector(boost::numeric::ublas::vector<float> &v, float r);

    /// @brief Transforms a matrix into a probability matrix where each columns
    /// entries sum up to one.
    /// @param mat
    /// @return
    boost::numeric::ublas::matrix<float>
    MatrixToProbabilityMatrix(boost::numeric::ublas::matrix<float> mat);
    std::map<int, std::vector<int>> clusters;

  public:
    boost::numeric::ublas::vector<float> comparisonState;
    int iterationMax;
    int maxEvolveSteps;
    float T0;
    float T;
    /*
    T0: Initial temperature
    kernelSize: Length of the memory kernel
    stateBaseSize: Number of Maps
    maxEvolveSteps: Number of map-rota steps/matrix multiplications during one
    optimization step iterationMax: Maximum number of optimization steps slope:
    Temperature decrement slope comparisonState: The state you want to mimic
    with the internal weights, usually calucated from the mapvotes
    */
    RotaOptimizer();
    RotaOptimizer(OptimizerConfig config);
    ~RotaOptimizer();
    /*
    Summary: Generates an initial state for the optimizer to start with.
    Params: dim dimension of the square matrix
    Return: Matrix<float>
    */
    boost::numeric::ublas::matrix<float> GenerateSeed(int dim);
    /*
    Summary: Metric on the space of states. Returns the distance between two
    states. Usually called energy Params: matrix<float> state1 matrix<float>
    state2 Return: float
    */
    float StateDifference(boost::numeric::ublas::vector<float> &state1,
                          boost::numeric::ublas::vector<float> &state2,
                          std::vector<float> &list);
    /*
    Summary: Decreases the temperature by some defined temperature profile
    Params:
        float T0, initial temperature
        float s, slope
        int i, step starting at zero
    Return: float
    */
    float UpdateTemperature(float T0, float s, int i);

    boost::numeric::ublas::matrix<float>
    GenerateNeighbour(boost::numeric::ublas::matrix<float> &state, float s,
                      float T, std::vector<float> &grid_fitness,
                      boost::numeric::ublas::matrix<float> &agent);

    /*
    Summary: Adds the given map-representative to the memory kernel and removes
    the most oldest one if the kernel length is full Params: matrix<float>
    evolvedState, the already evolved state array kernel, the memory kernel
    Return: array, the memory kernel
    */
    void UpdateMemoryKernel(
        int index,
        std::vector<boost::numeric::ublas::vector<int>> &kernel);
    /*
    Summary: Evolves the current state according to some procedure that imitates
    that map rota algorithm Params: matrix<float> state map<int,matrix<float>>
    clusters, the map-cluster map containing which state has which neighbours
    Return: matrix<float>, a new state
    */
    boost::numeric::ublas::vector<float>
    Evolve(boost::numeric::ublas::matrix<float> &state);
    /*
    Returns true if the state difference is either negative or greater than some
    random float in [0,1)
    */
    bool AcceptMove(float state_difference);

    std::vector<float> Run(bool debug = false);
};
}; // namespace optimizer