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
    float weights[] = {12};
    float injected_units[] = {12};
    float injected_weights[] = {12};
    injected_MLP_v2<3, 1, 0, 0, 3, 2, 1, 1, 3, 0, 3, 1> mlp(weights, injected_units, injected_weights);
    float input[] = {12};
    float output[1];
    mlp.feedforward(input, output);
    return 0;
}