#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include "RotaOptimizer.hpp"

boost::numeric::ublas::matrix<double> GenerateSeed(){
        boost::numeric::ublas::matrix<double> mat (3, 3);
        for (unsigned i = 0; i < mat.size1 (); ++ i)
            for (unsigned j = 0; j < mat.size2 (); ++ j)
                mat (i, j) = 3 * i + j;
        return mat;
    }

int main(void){
    return 0;
}