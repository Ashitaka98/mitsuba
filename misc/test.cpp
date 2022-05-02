#include "MLP.hpp"
int main()
{
    float weights[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    MLP<2, 2> mlp0(weights);
    MLP<2, 2, 2> mlp1(weights);
    float input[] = {-0.5, 1.5};
    float output[2];
    mlp0.feedforward(input, output);
    mlp1.feedforward(input, output);
    return 0;
}