#include "MLP.h"
int main()
{
    // float weights[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    // MLP<2, 2> mlp0(weights);
    // MLP<2, 2, 2> mlp1(weights);
    // float input[] = {-0.5, 1.5};
    // float output[2];
    // mlp0.feedforward(input, output);
    // mlp1.feedforward(input, output);
    float weights[] = {1, 1, 1, 0, 2, 2, 2, 3, 3, 3, 0, 0};
    float injected_units[] = {12, 13};
    float injected_weights[] = {0.1, 0, 0.2, 0.3, 0.4, 0};
    __attribute__((aligned(64))) injected_MLP_v2<3, 1, 0, 0, 3, 2, 1, 1, 3, 0, 3, 1> mlp(weights, injected_units, injected_weights);
    __attribute__((aligned(64))) float input[] = {1, 2, 3};
    float output[1];
    mlp.feedforward(input, output);
    std::cout << output[0];
    return 0;
}