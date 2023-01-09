#include <gtest/gtest.h>
#include "../../optimizer/RotaOptimizer.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace ublas = boost::numeric::ublas;

class Optimizer_Fixture : public ::testing::Test
{
    protected:
        double d;
};

// Checks for randomness and output dimension
TEST_F(Optimizer_Fixture, GenerateSeedTest){
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
}

TEST_F(Optimizer_Fixture, UpdateTemperatureTest){
    optimizer::RotaOptimizer opt;
    float T0 = 3.8;
    float s = 0.5;
    float values[] = {0, 1, 5, 11, 47, 53};
    float expValues[] = {   3.8, 
                            2.304816506908, 
                            0.3119229947708154,
                            0.015529731466163453, 
                            2.365174956705E-10, 
                            1.1775512727142936E-11};
    for( unsigned i = 0; i < 6; i++)
        ASSERT_NEAR(expValues[i], opt.UpdateTemperature(T0, s, values[i]), 0.001);
}

TEST_F(Optimizer_Fixture, StateDifferenceTest){
    optimizer::RotaOptimizer opt;

    float expValue = 37.0;

    boost::numeric::ublas::matrix<float> mat1(4,4);
    boost::numeric::ublas::matrix<float> mat2(4,4);

    mat1(0,0) = 1.0;
    mat1(1,0) = 3.0;
    mat1(2,0) = 2.0;
    mat1(3,0) = 4.0;

    mat2(0,0) = 5.0;
    mat2(1,0) = 2.0;
    mat2(2,0) = 4.0;
    mat2(3,0) = 0.0;

    ASSERT_NEAR(expValue, opt.StateDifference(mat1, mat2), 0.001);
}

TEST_F(Optimizer_Fixture, MatrixToProbabilityMatrixTest){
    optimizer::RotaOptimizer opt;
    boost::numeric::ublas::matrix<float> mat = opt.GenerateSeed(4);
    float expValue = 1.0;
    float sum;
    for(unsigned j=0; j<mat.size2(); j++){
        sum = 0.0;
        for(unsigned i=0; i<mat.size1(); i++)
            sum += mat(i,j);
        ASSERT_NEAR(expValue, sum, 0.001);
    }
}