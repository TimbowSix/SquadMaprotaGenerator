#include <gtest/gtest.h>
#include <vector>

#include "../../generator/utils.hpp"


TEST(Utils_test, sigmoid_test){
    float values[] = {-10, 0, 50, 24, -23, -3.7};
    float expValues[] = {0, 0.999999694097773, 1, 1, 0, 0.0293122};
    float shift = 3;
    float slope = 5;

    for(int i = 0; i < 6; i++){
        EXPECT_NEAR(rota::sigmoid(values[i], slope, shift), expValues[i], 0.01);
    }
}

TEST(Utils_test, weightedChoice){

    std::vector<float> weights = {0.1,0.5,0.2,0.15,0.05};
    int counts[5]= {0};
    int rounds = 0;
    while(rounds < 1000000){
        counts[rota::weightedChoice(&weights)]++;
    }

    for(int i = 0; i < 5; i++){
        EXPECT_NEAR(weights[i], counts[i]/1000000, 0.001);
    }

}