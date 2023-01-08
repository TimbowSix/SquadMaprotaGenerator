#pragma once

#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <random>

namespace optimizer
{
    /// @brief Optimizer Class
    class RotaOptimizer
    {
        private:
            float T0;
            int kernelSize;
            int stateBaseSize;
            int maxEvolveSteps;
            int iterationMax;
            float slope;
            boost::numeric::ublas::matrix<float> comparisonState;
            std::mt19937 generator;
            std::mt19937* generator_ptr;
        public:
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
            Summary: Calculates the fitness value of the given state.
            @param : 
            matrix<float> matrix, the state to be evaluated
            Return: float
            */
            void Fitness(boost::numeric::ublas::matrix<float> matrix);
            /*
            Summary: Metric on the space of states. Returns the distance between two states. Usually called energy
            Params: 
                matrix<float> state1 
                matrix<float> state2
            Return: float
            */
            void StateDifference(boost::numeric::ublas::matrix<float> state1, boost::numeric::ublas::matrix<float> state2);
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
            void GenerateNeighbour(boost::numeric::ublas::matrix<float> state, float s, float T);
            /*
            Summary: Transforms a matrix consisting of un-normalised wheights into a probability matrix where each column sums up to one
            Params: 
                matrix<float> transitionmatrix
            Return: matrix<float>
            */
            void ProbabilityFromTransitionMatrix(boost::numeric::ublas::matrix<float> transitionmatrix);
            /*
            Summary: Adds the given map-representative to the memory kernel and removes the most oldest one if the kernel length is full
            Params:
                matrix<float> evolvedState, the already evolved state
                array kernel, the memory kernel
            Return: array, the memory kernel
            */
            void UpdateMemoryKernel(boost::numeric::ublas::matrix<float> evolvedState, boost::numeric::ublas::matrix<float> kernel);//REPLACE KERNEL WITH ARRAY
            /*
            Summary: Sets the values of a matrix to zero only in the given row
            Params: 
                matrix<float> mat
                int rowindex
            Return: matrix<float>, the input matrix
            */
            void SetRowZero(boost::numeric::ublas::matrix<float> mat, int rowindex);
            /*
            Summary: Normalizes the columns of a matrix
            Params:
                matrix<float> mat
            Return: matrix<float>, the input matrix
            */
            void NormalizeMat(boost::numeric::ublas::matrix<float> mat);
            /*
            Summary: Evolves the current state according to some procedure that imitates that map rota algorithm
            Params: 
                matrix<float> state
                map<int,matrix<float>> clusters, the map-cluster map containing which state has which neighbours
            Return: matrix<float>, a new state
            */
            void Evolve();
    };
};