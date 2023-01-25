#pragma once

#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <random>
#include <map>

namespace optimizer
{
    /// @brief Optimizer Class
    class RotaOptimizer
    {
        private:
            int kernelSize;
            int stateBaseSize;
            float slope;
            std::mt19937 generator;
            std::mt19937* generator_ptr;
            std::vector<boost::numeric::ublas::vector<float>> memorykernel;

            /// @brief Transforms a matrix into a probability matrix where each columns entries sum up to one.
            /// @param mat 
            /// @return 
            boost::numeric::ublas::matrix<float> MatrixToProbabilityMatrix(boost::numeric::ublas::matrix<float> mat);
            std::map<int, std::vector<int>> clusters;

        public:
            boost::numeric::ublas::vector<float> comparisonState;
            int iterationMax;
            int maxEvolveSteps;
            float T0;
            /*
            T0: Initial temperature
            kernelSize: Length of the memory kernel
            stateBaseSize: Number of Maps
            maxEvolveSteps: Number of map-rota steps/matrix multiplications during one optimization step
            iterationMax: Maximum number of optimization steps
            slope: Temperature decrement slope
            comparisonState: The state you want to mimic with the internal weights, usually calucated from the mapvotes
            */
            RotaOptimizer();
            ~RotaOptimizer();
            /*
            Summary: Generates an initial state for the optimizer to start with.
            Params: dim dimension of the square matrix
            Return: Matrix<float>
            */
            boost::numeric::ublas::matrix<float> GenerateSeed(int dim);
            /*
            Summary: Metric on the space of states. Returns the distance between two states. Usually called energy
            Params: 
                matrix<float> state1 
                matrix<float> state2
            Return: float
            */
            float StateDifference(boost::numeric::ublas::vector<float> state1, boost::numeric::ublas::vector<float> state2);
             /*
            Summary: Metric on the space of states. Returns the distance between two states. Usually called energy
            Params: 
                matrix<float> state1 
                matrix<float> state2
            Return: float
            */
            float StateDifference(boost::numeric::ublas::vector<float> state1, boost::numeric::ublas::vector<float> state2, std::vector<float>& list);
            /*
            Summary: Calculates the probability of accepting a state with positive energy difference.
            Params: 
                matrix<float> state1, current state
                matrix<float> state2, compare state
                float T, temperature;
            Return: float
            */
            void TransitionProbability(boost::numeric::ublas::matrix<float> state1, boost::numeric::ublas::matrix<float> state2, float T);
            /*
            Summary: Decreases the temperature by some defined temperature profile
            Params: 
                float T0, initial temperature
                float s, slope
                int i, step starting at zero
            Return: float
            */
            float UpdateTemperature(float T0, float s, int i);
            /*
            Summary: Generates the next state
            Params:
                matrix<float> state, current state
                float s, slope
                float T, temperature
            Return: matrix<float>
            */
            boost::numeric::ublas::matrix<float> GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T);
            /*
            Summary: Generates the next state
            Params:
                matrix<float> state, current state
                float s, slope
                float T, temperature
            Return: matrix<float>
            */
            boost::numeric::ublas::matrix<float> GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T, std::vector<float> grid_fitness);
            /*
            Summary: Adds the given map-representative to the memory kernel and removes the most oldest one if the kernel length is full
            Params:
                matrix<float> evolvedState, the already evolved state
                array kernel, the memory kernel
            Return: array, the memory kernel
            */
            void UpdateMemoryKernel(boost::numeric::ublas::vector<float>& evolvedState, std::vector<boost::numeric::ublas::vector<float>>& kernel);
            /*
            Summary: Sets the values of a matrix to zero only in the given row
            Params: 
                matrix<float> mat
                int rowindex
            Return: matrix<float>, the input matrix
            */
            void SetRowZero(boost::numeric::ublas::matrix<float>& mat, int rowindex);
            /*
            Summary: Evolves the current state according to some procedure that imitates that map rota algorithm
            Params: 
                matrix<float> state
                map<int,matrix<float>> clusters, the map-cluster map containing which state has which neighbours
            Return: matrix<float>, a new state
            */
            boost::numeric::ublas::vector<float> Evolve(boost::numeric::ublas::matrix<float>& state);
            /*
            Returns true if the state difference is either negative or greater than some random float in [0,1)
            */
            bool AcceptMove(float state_difference);
            boost::numeric::ublas::matrix<float> ComparisonState_FromProbabilities(std::vector<float> probabilities);

            std::vector<float> TakeMemoryKernelSum();

            std::vector<float> ProbabilitiesFromMemoryKernel();
    };
};