#include <iostream>
#include <math.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    double RotaOptimizer::UpdateTemperature(double T0, double s, int i){
        return T0*exp(-s*i);
    }

    boost::numeric::ublas::matrix<double> RotaOptimizer::GenerateSeed(int dim){
        int dist_max = 10000;
        boost::mt19937 gen(time(0));
        boost::random::uniform_int_distribution<> dist(0, dist_max);
        boost::numeric::ublas::matrix<double> mat (dim, dim);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = dist(gen)/((double)dist_max);
        return mat;
    }

    RotaOptimizer::RotaOptimizer(){

    };
    RotaOptimizer::~RotaOptimizer(){

    };

}