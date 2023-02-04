#include "OptimizerConfig.hpp"

namespace optimizer{
    OptimizerConfig::OptimizerConfig(int numberOfMaps, int memoryKernelSize, std::map<int, std::vector<int>> neighbourDictionary, std::vector<float> mapProbabilities){
        this->T0 = 0.03;
        this->iterationMax = 1000;
        this->maxEvolveSteps = 1000;
        this->kernelsize = memoryKernelSize;
        this->stateBaseSize = numberOfMaps;
        this->slope = 0.05;
        this->clusters = neighbourDictionary;
        this->mapProbabilities = mapProbabilities;
    };
}