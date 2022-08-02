#pragma once
#ifndef __SH_ENCODE_H_
#define __SH_ENCODE_H_

#include <cmath>
#include <cassert>

#define SH_SAMPLE_RATE 1000
#define SH_TABLE_SIZE (SH_SAMPLE_RATE * SH_SAMPLE_RATE)
#define SH_DIMS 81

static const float pi = acos(-1);

struct Omega_io
{
    float theta1, phi1, theta2, phi2;
};

struct Omega_io_xyz
{
    float xyz[6];
};

template <int dim>
struct SH_basis
{
    float sh[dim];
};

template <int dim1, int dim2>
SH_basis<dim1> cast_sh_basis(const SH_basis<dim2> &src)
{
#ifdef SH_DEBUG
    assert(dim1 < dim2);
#endif
    SH_basis<dim1> ret;
    memcpy(ret.sh, src.sh, dim1 * sizeof(float));
    return ret;
};

class SH_Encode
{

private:
    SH_Encode()
    {
        FILE *sh_table_file = fopen("/home/lzr/Projects/mitsuba/data/sh_table/sh_table.txt", "r");
        assert(sh_table_file);
        for (int i = 0; i < SH_TABLE_SIZE; i++)
        {
            for (int j = 0; j < SH_DIMS; j++)
            {
                int test = fscanf(sh_table_file, "%f", &sh_table[i].sh[j]);
#ifdef SH_DEBUG
                assert(test != EOF);
#endif
            }
        }
#ifdef SH_DEBUG
        float test;
        assert(fscanf(sh_table_file, "%f", &test) == EOF);
#endif
        fclose(sh_table_file);
    }

public:
    static SH_Encode *instance;
    static SH_Encode *getInstance()
    {
        if (!instance)
            instance = new SH_Encode();
        return instance;
    }

    static Omega_io convert_to_rusinkiewicz(Omega_io wiwo)
    {
        float theta_i = wiwo.theta1;
        float phi_i = wiwo.phi1;
        float theta_o = wiwo.theta2;
        float phi_o = wiwo.phi2;
        float x1 = cos(theta_i) * sin(phi_i);
        float y1 = sin(theta_i) * sin(phi_i);
        float z1 = cos(phi_i);
        float x2 = cos(theta_o) * sin(phi_o);
        float y2 = sin(theta_o) * sin(phi_o);
        float z2 = cos(phi_o);
        float hx = x1 + x2;
        float hy = y1 + y2;
        float hz = z1 + z2;
        float norm_h = sqrt(hx * hx + hy * hy + hz * hz + 0.00001f);
        hx /= norm_h;
        hy /= norm_h;
        hz /= norm_h;
        float phi_h = acos(hz);
        float theta_h = atan2(hy, hx);
        if (theta_h < 0)
            theta_h += 2 * pi - 0.00001;

        float wi[3] = {x1, y1, z1};
        double rot_z[3][3] = {
            {cos(-theta_h), -sin(-theta_h), 0.0},
            {sin(-theta_h), cos(-theta_h), 0.0},
            {0.0, 0.0, 1.0}};
        double rot_y[3][3] = {
            {cos(-phi_h), 0.0, sin(-phi_h)},
            {0.0, 1.0, 0.0},
            {-sin(-phi_h), 0.0, cos(-phi_h)}};

        float d_tmp[3];
        for (int i = 0; i < 3; i++)
        {
            d_tmp[i] = 0;
            for (int j = 0; j < 3; j++)
                d_tmp[i] += rot_z[i][j] * wi[j];
        }
        float d[3];
        for (int i = 0; i < 3; i++)
        {
            d[i] = 0;
            for (int j = 0; j < 3; j++)
                d[i] += rot_y[i][j] * d_tmp[j];
        }
        d[2] = d[2] < -0.9999 ? -0.9999 : d[2];
        d[2] = d[2] > 0.9999 ? 0.9999 : d[2];
        float phi_d = acos(d[2]);
        float theta_d = atan2(d[1], d[0]);
        if (theta_d < 0)
            theta_d += 2 * pi - 0.00001;

#ifdef SH_DEBUG
        assert(theta_h >= 0 && theta_h <= 2 * pi);
        assert(theta_d >= 0 && theta_d <= 2 * pi);
        assert(phi_h >= 0 && phi_h <= pi);
        assert(phi_d >= 0 && phi_d <= pi);
#endif

        Omega_io whwd = (Omega_io){theta_h, phi_h, theta_d, phi_d};
        return whwd;
    }

    static Omega_io convert_to_rusinkiewicz(float x1, float y1, float z1, float x2, float y2, float z2)
    {
        float hx = x1 + x2;
        float hy = y1 + y2;
        float hz = z1 + z2;
        float norm_h = sqrt(hx * hx + hy * hy + hz * hz + 0.00001f);
        hx /= norm_h;
        hy /= norm_h;
        hz /= norm_h;
        float phi_h = acos(hz);
        float theta_h = atan2(hy, hx);
        if (theta_h < 0)
            theta_h += 2 * pi - 0.00001;

        float wi[3] = {x1, y1, z1};
        double rot_z[3][3] = {
            {cos(-theta_h), -sin(-theta_h), 0.0},
            {sin(-theta_h), cos(-theta_h), 0.0},
            {0.0, 0.0, 1.0}};
        double rot_y[3][3] = {
            {cos(-phi_h), 0.0, sin(-phi_h)},
            {0.0, 1.0, 0.0},
            {-sin(-phi_h), 0.0, cos(-phi_h)}};

        float d_tmp[3];
        for (int i = 0; i < 3; i++)
        {
            d_tmp[i] = 0;
            for (int j = 0; j < 3; j++)
                d_tmp[i] += rot_z[i][j] * wi[j];
        }
        float d[3];
        for (int i = 0; i < 3; i++)
        {
            d[i] = 0;
            for (int j = 0; j < 3; j++)
                d[i] += rot_y[i][j] * d_tmp[j];
        }
        d[2] = d[2] < -0.9999 ? -0.9999 : d[2];
        d[2] = d[2] > 0.9999 ? 0.9999 : d[2];

        float phi_d = acos(d[2]);
        float theta_d = atan2(d[1], d[0]);
        if (theta_d < 0)
            theta_d += 2 * pi - 0.00001;

#ifdef SH_DEBUG
        assert(theta_h >= 0 && theta_h <= 2 * pi);
        assert(theta_d >= 0 && theta_d <= 2 * pi);
        assert(phi_h >= 0 && phi_h <= pi);
        assert(phi_d >= 0 && phi_d <= pi);
#endif

        Omega_io whwd = (Omega_io){theta_h, phi_h, theta_d, phi_d};
        return whwd;
    }

    static Omega_io_xyz convert_to_rusinkiewicz_xyz(float x1, float y1, float z1, float x2, float y2, float z2)
    {
        float hx = x1 + x2;
        float hy = y1 + y2;
        float hz = z1 + z2;
        float norm_h = sqrt(hx * hx + hy * hy + hz * hz + 0.00001f);
        hx /= norm_h;
        hy /= norm_h;
        hz /= norm_h;
        float phi_h = acos(hz);
        float theta_h = atan2(hy, hx);
        if (theta_h < 0)
            theta_h += 2 * pi - 0.00001;

        float wi[3] = {x1, y1, z1};
        double rot_z[3][3] = {
            {cos(-theta_h), -sin(-theta_h), 0.0},
            {sin(-theta_h), cos(-theta_h), 0.0},
            {0.0, 0.0, 1.0}};
        double rot_y[3][3] = {
            {cos(-phi_h), 0.0, sin(-phi_h)},
            {0.0, 1.0, 0.0},
            {-sin(-phi_h), 0.0, cos(-phi_h)}};

        float d_tmp[3];
        for (int i = 0; i < 3; i++)
        {
            d_tmp[i] = 0;
            for (int j = 0; j < 3; j++)
                d_tmp[i] += rot_z[i][j] * wi[j];
        }
        float d[3];
        for (int i = 0; i < 3; i++)
        {
            d[i] = 0;
            for (int j = 0; j < 3; j++)
                d[i] += rot_y[i][j] * d_tmp[j];
        }

        Omega_io_xyz whwd;
        whwd.xyz[0] = hx;
        whwd.xyz[1] = hy;
        whwd.xyz[2] = hz;
        whwd.xyz[3] = d[0];
        whwd.xyz[4] = d[1];
        whwd.xyz[5] = d[2];
        return whwd;
    }

    template <int dim>
    SH_basis<dim> eval_sh_basis(float theta, float phi)
    {
        int idx_theta = floor(SH_SAMPLE_RATE * theta / (2 * pi));
        int idx_phi = floor(SH_SAMPLE_RATE * theta / pi);
        int idx = idx_theta * SH_SAMPLE_RATE + idx_phi;
        return cast_sh_basis<dim, SH_DIMS>(sh_table[idx]);
    }
    template <int dim>
    SH_basis<dim> eval_sh_basis(float x, float y, float z)
    {
        float phi = acos(z);
        float theta = atan2(y, x);
        if (theta < 0)
            theta += 2 * pi - 0.00001;
        int idx_theta = floor(SH_SAMPLE_RATE * theta / (2 * pi));
        int idx_phi = floor(SH_SAMPLE_RATE * theta / pi);
        int idx = idx_theta * SH_SAMPLE_RATE + idx_phi;
        return cast_sh_basis<dim, SH_DIMS>(sh_table[idx]);
    }

private:
    SH_basis<SH_DIMS> sh_table[SH_TABLE_SIZE];
};
SH_Encode *SH_Encode::instance = nullptr;
#endif /* __SH_ENCODE_H_ */