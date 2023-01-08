#include <iostream>
#include <math.h>
#include <random>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    double RotaOptimizer::UpdateTemperature(double T0, double s, int i){
        return T0*exp(-s*i);
    }

    boost::numeric::ublas::matrix<double> RotaOptimizer::GenerateSeed(int dim){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        std::mt19937 generator(seed);           // the generator seeded with the random device
        std::uniform_real_distribution<> distribute(0,1);   //1 is excluded!
        boost::numeric::ublas::matrix<double> mat (dim, dim);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = distribute(generator);
        return mat;
    }

    RotaOptimizer::RotaOptimizer(){

    };
    RotaOptimizer::~RotaOptimizer(){

    };

}