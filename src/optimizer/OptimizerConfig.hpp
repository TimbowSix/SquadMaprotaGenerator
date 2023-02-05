#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace optimizer{
    class OptimizerConfig{
        private:
        public:
            OptimizerConfig(int memoryKernelSize, std::map<int, std::vector<int>> clusters, std::vector<float> mapProbabilities);
            ~OptimizerConfig();
            float T0;
            int iterationMax;
            int maxEvolveSteps;
            int kernelsize;
            int stateBaseSize;
            float slope;
            std::map<int, std::vector<int>> clusters;
            std::vector<float> mapProbabilities;
    };
}