#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

#include "RotaOptimizer.cpp"

int main(void){
    optimizer::RotaOptimizer opt;

    boost::numeric::ublas::matrix<double> seed = opt.GenerateSeed(3);
    boost::numeric::ublas::matrix<double> seed2 = opt.GenerateSeed(3);
    for (unsigned i = 0; i < seed2.size1 (); ++ i)
            for (unsigned j = 0; j < seed2.size2 (); ++ j)
                std::cout << seed2 (i, j);

    return 0;
}