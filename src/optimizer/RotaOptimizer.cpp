#include <iostream>
#include <math.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "RotaOptimizer.hpp"

namespace optimizer
{
    double UpdateTemperature(double T0, double s, int i){
        return T0*exp(-s*i);
    }

    boost::numeric::ublas::matrix<double> GenerateSeed(){
        boost::mt19937 gen;
        boost::random::uniform_int_distribution<> dist(0, 1);
        boost::numeric::ublas::matrix<double> mat (3, 3);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = dist(gen);
        return mat;
    }


}