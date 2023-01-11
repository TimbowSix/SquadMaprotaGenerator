#include <gtest/gtest.h>
#include <vector>

#include "../../generator/utils.hpp"

TEST(Utils_test, sigmoid_test) {
    float values[] = {-10, 0, 50, 24, -23, -3.7};
    float expValues[] = {0, 0.999999694097773, 1, 1, 0, 0.0293122};
    float shift = 3;
    float slope = 5;

    for (int i = 0; i < 6; i++) {
        EXPECT_NEAR(rota::sigmoid(values[i], slope, shift), expValues[i], 0.01);
    }
}

TEST(Utils_test, weightedChoice_normal) {

    srand(time(0));

    std::vector<float> weights = {0.1, 0.5, 0.2, 0.15, 0.05, 0.0};
    int counts[6] = {0, 0, 0, 0, 0, 0};
    int rounds = 0;
    while (rounds < 1000000) {
        int val = rota::weightedChoice(&weights);
        ASSERT_GE(val, 0);
        ASSERT_LE(val, 4);
        counts[val]++;
        rounds++;
    }

    for (int i = 0; i < weights.size(); i++) {
        //std::cout << counts[i] << std::endl;
        EXPECT_NEAR(weights[i], (float)counts[i] / 1000000, 0.002);
    }
}

TEST(Utils_test, choice) {

    srand(time(0));

    int counts[4] = {0, 0, 0, 0};
    for(int i=0; i<1000000; i++){
        int val = rota::choice(4);
        ASSERT_GE(val, 0);
        ASSERT_LE(val, 3);
        counts[val]++;
    }

    for (int i : counts) {
        EXPECT_NEAR(1/i, 1/4, 0.002);
    }
}

TEST(Utils_test, normalize) {
    std::vector<float> arr = {100, 12, 44, 43, 6, 0, 1};
    std::vector<float> expected = {0.5, 0.06, 0.22, 0.215, 0.03, 0.0, 0.005};

    std::sort(expected.begin(), expected.end());

    std::vector<float> withOutSum = arr;
    rota::normalize(&withOutSum, NULL);
    std::sort(withOutSum.begin(), withOutSum.end());

    std::vector<float> withSum = arr;
    float sum = 200;
    rota::normalize(&withSum, &sum);
    std::sort(withSum.begin(), withSum.end());

    for (int i = 0; i < expected.size(); i++) {
        ASSERT_NEAR(withOutSum[i], expected[i], 0.02);
        ASSERT_NEAR(withSum[i], expected[i], 0.02);
    }
}

TEST(Utils_test, normalize_zero) {
    std::vector<float> arr = {0, 0, 0, 0, 0};
    rota::normalize(&arr, NULL);
    for (int i = 0; i < arr.size(); i++) {
        ASSERT_EQ(arr[i], 0);
    }
}
