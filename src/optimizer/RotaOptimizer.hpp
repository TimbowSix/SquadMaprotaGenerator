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
            double T0;
            int kernelSize;
            int stateBaseSize;
            int maxEvolveSteps;
            int iterationMax;
            double slope;
            boost::numeric::ublas::matrix<double> comparisonState;
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
            Return: Matrix<double>
            */
            boost::numeric::ublas::matrix<double> GenerateSeed(int dim);

            /*
            Summary: Calculates the fitness value of the given state.
            @param : 
            matrix<double> matrix, the state to be evaluated
            Return: double
            */
            void Fitness(boost::numeric::ublas::matrix<double> matrix);
            /*
            Summary: Metric on the space of states. Returns the distance between two states. Usually called energy
            Params: 
                matrix<double> state1 
                matrix<double> state2
            Return: double
            */
            void StateDifference(boost::numeric::ublas::matrix<double> state1, boost::numeric::ublas::matrix<double> state2);
            /*
            Summary: Calculates the probability of accepting a state with positive energy difference.
            Params: 
                matrix<double> state1, current state
                matrix<double> state2, compare state
                double T, temperature;
            Return: double
            */
            void TransitionProbability(boost::numeric::ublas::matrix<double> state1, boost::numeric::ublas::matrix<double> state2, double T);
            /*
            Summary: Decreases the temperature by some defined temperature profile
            Params: 
                double T0, initial temperature
                double s, slope
                int i, step starting at zero
            Return: double
            */
            double UpdateTemperature(double T0, double s, int i);
            /*
            Summary: Generates the next state
            Params:
                matrix<double> state, current state
                double s, slope
                double T, temperature
            Return: matrix<double>
            */
            void GenerateNeighbour(boost::numeric::ublas::matrix<double> state, double s, double T);
            /*
            Summary: Transforms a matrix consisting of un-normalised wheights into a probability matrix where each column sums up to one
            Params: 
                matrix<double> transitionmatrix
            Return: matrix<double>
            */
            void ProbabilityFromTransitionMatrix(boost::numeric::ublas::matrix<double> transitionmatrix);
            /*
            Summary: Adds the given map-representative to the memory kernel and removes the most oldest one if the kernel length is full
            Params:
                matrix<double> evolvedState, the already evolved state
                array kernel, the memory kernel
            Return: array, the memory kernel
            */
            void UpdateMemoryKernel(boost::numeric::ublas::matrix<double> evolvedState, boost::numeric::ublas::matrix<double> kernel);//REPLACE KERNEL WITH ARRAY
            /*
            Summary: Sets the values of a matrix to zero only in the given row
            Params: 
                matrix<double> mat
                int rowindex
            Return: matrix<double>, the input matrix
            */
            void SetRowZero(boost::numeric::ublas::matrix<double> mat, int rowindex);
            /*
            Summary: Normalizes the columns of a matrix
            Params:
                matrix<double> mat
            Return: matrix<double>, the input matrix
            */
            void NormalizeMat(boost::numeric::ublas::matrix<double> mat);
            /*
            Summary: Evolves the current state according to some procedure that imitates that map rota algorithm
            Params: 
                matrix<double> state
                map<int,matrix<double>> clusters, the map-cluster map containing which state has which neighbours
            Return: matrix<double>, a new state
            */
            void Evolve();
    };
};