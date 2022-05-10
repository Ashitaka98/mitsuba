#include <cmath>
#include <random>

#include <mitsuba/core/vector.h>
inline void inverse_miu_law_compression(int miu, float src[], int size)
{
    for (int i = 0; i < size; i++)
    {
        src[i] = (std::exp(src[i] * std::log(1.0 + miu)) - 1.0) / miu;
    }
}

inline mitsuba::Vector sample_uniform_direction()
{
    static std::default_random_engine e(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_real_distribution<> u_x(-1, 1);
    static std::uniform_real_distribution<> u_y(-1, 1);
    static std::uniform_real_distribution<> u_z(0, 1);
    float x = u_x(e);
    float y = u_y(e);
    float z = u_z(e);
    float norm = x * x + y * y + z * z;
    while (norm > 1)
    {
        x = u_x(e);
        y = u_y(e);
        z = u_z(e);
        norm = x * x + y * y + z * z;
    }
    if (EXPECT_NOT_TAKEN(z == 0))
        z = 1e-10f;
    float root_norm = sqrt(norm);
    return mitsuba::Vector(x / root_norm, y / root_norm, z / root_norm);
}