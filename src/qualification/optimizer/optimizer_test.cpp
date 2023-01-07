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
    optimizer::RotaOptimizer opt;
    ublas::matrix<double> seed1 = opt.GenerateSeed(3);
    ublas::matrix<double> seed2 = opt.GenerateSeed(3);
    bool elementsMatch = true;

    for (unsigned i = 0; i < seed1.size1 (); ++ i)
            for (unsigned j = 0; j < seed1.size2 (); ++ j)
                elementsMatch = seed1(i,j) == seed2(i,j) && elementsMatch;

    ASSERT_FALSE(elementsMatch);
};