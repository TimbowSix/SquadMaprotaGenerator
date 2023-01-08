#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"


int main(void){
    optimizer::RotaOptimizer opt;

    boost::numeric::ublas::matrix<double> seed1 = opt.GenerateSeed(3);

    return 0;
}