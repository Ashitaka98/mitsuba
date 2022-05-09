#include <cmath>
static inline void inverse_miu_law_compression(int miu, float src[], int size)
{
    for (int i = 0; i < size; i++)
    {
        src[i] = (std::exp(src[i] * std::log(1.0 + miu)) - 1.0) / miu;
    }
}