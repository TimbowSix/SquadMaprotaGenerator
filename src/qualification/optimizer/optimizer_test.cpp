#include <gtest/gtest.h>
#include "../../optimizer/RotaOptimizer.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace ublas = boost::numeric::ublas;

class Optimizer_Fixture : public ::testing::Test
{
    protected:
        double d;
};

TEST_F(Optimizer_Fixture, OptimizerTest){
    int matrix_dimension = 5;
    optimizer::RotaOptimizer opt;
    ublas::matrix<double> seed1 = opt.GenerateSeed(matrix_dimension);
    ublas::matrix<double> seed2 = opt.GenerateSeed(matrix_dimension);
    bool elementsMatch = true;
    bool hasCorrectDimension = false;
    for (unsigned i = 0; i < seed1.size1 (); ++ i)
            for (unsigned j = 0; j < seed1.size2 (); ++ j)
                elementsMatch = seed1(i,j) == seed2(i,j) && elementsMatch;

    ASSERT_TRUE(matrix_dimension == seed1.size1() &&
                matrix_dimension == seed1.size2());

    ASSERT_FALSE(elementsMatch);
};