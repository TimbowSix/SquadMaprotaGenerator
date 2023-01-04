#include <vector>
#include <stdexcept>
#include <numeric>
#include <random>

namespace rota
{
    int weightedChoice(std::vector<float> *weights){
        float weightSum = std::reduce(weights->begin(), weights->end());
        weightSum = roundf(weightSum * 100) / 100; // round to 2 decimal places to account floating point error
        if (weightSum != 1){
            throw std::invalid_argument("Weights do not sum to 1");
        }
        float randomValue = (float)rand()/RAND_MAX;
        float currentValue = 0;
        for(int i=0; i<weights->size(); i++){
            currentValue += weights->at(i);
            if(randomValue <= currentValue){
                return i;
            }
        }
        return -1;
    }

} // namespace rota
