#include <iostream>
#include <math.h>
#include <random>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    float RotaOptimizer::UpdateTemperature(float T0, float s, int i){
        return T0*exp(-s*i);
    }

    boost::numeric::ublas::matrix<float> RotaOptimizer::GenerateSeed(int dim){
        std::uniform_real_distribution<> distribute(0,1);   //uniform-dist wrapper for rng, 1 is excluded
        boost::numeric::ublas::matrix<float> mat (dim, dim);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = distribute(this->generator);
        return mat;
    }

    RotaOptimizer::RotaOptimizer(){
        std::random_device os_seed;             // seed used by the mersenne-twister-engine
        const uint_least32_t seed = os_seed();  

        generator = std::mt19937(seed);            // the generator seeded with the random device

    };
    RotaOptimizer::~RotaOptimizer(){

    };

}