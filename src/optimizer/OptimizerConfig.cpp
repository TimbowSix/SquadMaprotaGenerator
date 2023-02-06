#include "OptimizerConfig.hpp"

namespace optimizer{
    OptimizerConfig::OptimizerConfig(int memoryKernelSize, std::map<int, std::vector<int>> neighbourDictionary, std::vector<float> mapProbabilities){
        this->T0 = 0.03;
        this->iterationMax = 2000;
        this->maxEvolveSteps = 1000;
        if(mapProbabilities.size() - memoryKernelSize > 2){
            this->kernelsize = memoryKernelSize;
        }
        /*
        Edge case Destruction:
        only four maps (effectivley three including neighbours) but mem-colonel of four -> no maps choosen in optimizer
        To avoid this behavoir we simply just set the mem-len to one whenever we have less than 2 maps to choose from during the evolve-step in the optimizer
        */
        else{
            this->kernelsize = 1;
        }
        this->stateBaseSize = mapProbabilities.size();
        this->slope = 0.05;
        this->clusters = neighbourDictionary;
        this->mapProbabilities = mapProbabilities;
    };
    OptimizerConfig::~OptimizerConfig(){};
}