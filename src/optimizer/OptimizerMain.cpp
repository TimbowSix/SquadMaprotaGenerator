#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.hpp"


int main(void){
    optimizer::RotaOptimizer opt;

    boost::numeric::ublas::matrix<float> mat = opt.GenerateSeed(3);

    return 0;
}