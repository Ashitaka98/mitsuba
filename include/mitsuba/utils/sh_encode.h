#pragma once
#include <mitsuba/core/platform.h>
#include <cmath>
#include <cassert>
#include <atomic>

#define SH_DEBUG

const int SH_SAMPLE_RATE = 1000;
const int SH_TABLE_SIZE = SH_SAMPLE_RATE * SH_SAMPLE_RATE;
const int SH_DIMS = 81;

const float pi = acos(-1);

MTS_NAMESPACE_BEGIN

struct Omega_io
{
    float theta1, phi1, theta2, phi2;
};

class SH_Encode
{

private:
    SH_Encode()
    {
        FILE *sh_table_file = fopen("/home/lzr/Projects/mitsuba/data/sh_table/sh_table.txt", "r");
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
    static SH_Encode *instance;
    public : SH_Encode *getInstance()
    {

        if (instance)
            return instance;
        else
            instance = new SH_Encode();
    }
    struct SH_bases
    {
        float sh[SH_DIMS];
    };

    struct SH_io
    {
        SH_bases sh_val_h, sh_val_d;
    };

    Omega_io convert_to_rusinkiewicz(Omega_io wiwo)
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
        float norm_h = sqrt(hx * hx + hy * hy + hz * hz);
        hx /= norm_h;
        hy /= norm_h;
        hz /= norm_h;
        float phi_h = acos(hz);
        float theta_h = atan2(hy, hx);
        if (theta_h < 0)
            theta_h += 2 * pi;

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

        float phi_d = acos(d[2]);
        float theta_d = atan2(d[1], d[0]);
        if (theta_d < 0)
            theta_d += 2 * pi;

#ifdef SH_DEBUG
        assert(theta_h >= 0 && theta_h <= 2 * pi);
        assert(theta_d >= 0 && theta_d <= 2 * pi);
        assert(phi_h >= 0 && phi_h <= pi);
        assert(phi_d >= 0 && phi_d <= pi);
#endif

        Omega_io whwd = (Omega_io){theta_h, phi_h, theta_d, phi_d};
        return whwd;
    }

    SH_bases eval_sh_bases(float theta, float phi)
    {
        int idx_theta = SH_SAMPLE_RATE * theta / (2 * pi);
        int idx_phi = SH_SAMPLE_RATE * theta / pi;
        int idx = idx_theta * SH_SAMPLE_RATE + idx_phi;
        return sh_table[idx];
    }

    SH_io convert_to_sh_enc(Omega_io whwd)
    {
        SH_io ret;
        ret.sh_val_h = eval_sh_bases(whwd.theta1, whwd.phi1);
        ret.sh_val_d = eval_sh_bases(whwd.theta2, whwd.phi2);
        return ret;
    }

private:
    SH_bases sh_table[SH_TABLE_SIZE];
};

MTS_NAMESPACE_END