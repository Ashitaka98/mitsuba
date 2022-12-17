#pragma once
#ifndef __MLP_H_
#define __MLP_H_
#include <immintrin.h>
#include <string.h>
#ifdef MLP_DEBUG
#include <iostream>
#endif

#define RoundUp(X, base) (static_cast<int>((X - 1) / base) * base + base)

inline float relu(float x)
{
    return x > 0 ? x : 0;
}

const int batch_size = sizeof(__m512) / sizeof(float); // 16
template <int input_dim, int output_dim>
class Layer
{
public:
    Layer()
    {
        for (int i = 0; i < output_dim; i++)
        {
            _bias[i] = 0;
            for (int j = 0; j < RoundUp(input_dim, batch_size); j++)
                _weights[i][j] = 0;
        }
    }
    Layer(float *buffer)
    {
        set(buffer);
    }

    void set(float *buffer)
    {
        float *p = buffer;
        for (int i = 0; i < output_dim; i++)
        {
            for (int j = 0; j < input_dim; j++)
            {
                _weights[i][j] = *p;
                p++;
            }
            for (int j = input_dim; j < RoundUp(input_dim, batch_size); j++)
                _weights[i][j] = 0;
        }
        for (int i = 0; i < output_dim; i++)
        {
            _bias[i] = *p;
            p++;
        }
    }

    /*
    inputs must be aligned(64)
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        __m512 v, v1, v2, v3;
        __attribute__((aligned(64))) float tmp[batch_size];
        for (int i = 0; i < output_dim; i++)
        {
            outputs[i] = _bias[i];
            v = _mm512_setzero_ps();
            for (int j = 0; j < input_dim; j += batch_size)
            {
                v1 = _mm512_load_ps(&_weights[i][j]);
                v2 = _mm512_load_ps(&inputs[j]);
                v3 = _mm512_mul_ps(v1, v2);
                v = _mm512_add_ps(v, v3);
            }
            _mm512_store_ps(tmp, v);
            for (int j = 0; j < batch_size; j++)
                outputs[i] += tmp[j];
            // activation is execute outside
        }
#ifdef MLP_DEBUG
        for (int i = 0; i < output_dim; i++)
            std::cout << outputs[i] << " ";
        std::cout << std::endl;
#endif
    }
    __attribute__((aligned(64))) float _weights[output_dim][RoundUp(input_dim, batch_size)];
    float _bias[output_dim];
};

// SharedWeights<168,32,32,31,32,31,32,31,32,31,32,31,32,1,1>
// should be aligned
template <int input_dim0, int dim0, int input_dim1, int... args>
class SharedWeights
{
public:
    SharedWeights(float *weights) : _layer(weights), _rest(weights + input_dim0 * dim0 + dim0) {}
    Layer<input_dim0, dim0> _layer __attribute__((aligned(64)));
    SharedWeights<input_dim1, args...> _rest __attribute__((aligned(64)));
    static const int num_weights = input_dim0 * dim0 + dim0 + decltype(_rest)::num_weights;
};
template <int input_dim0, int dim0, int input_dim1>
class SharedWeights<input_dim0, dim0, input_dim1>
{
public:
    SharedWeights(float *weights) : _layer(weights) {}
    Layer<input_dim0, dim0> _layer __attribute__((aligned(64)));
    static const int num_weights = input_dim0 * dim0 + dim0;
};

// BSDFNet<168,32,32,31,32,31,32,31,32,31,32,31,32,1,1>
template <int input_dim0, int dim0, int input_dim1, int... args>
class BSDFNet
{
public:
    BSDFNet(float *biasDelta, float *predParams) : _rest(biasDelta + dim0, predParams + (input_dim0 + 1) * (input_dim1 - dim0))
    {
        static_assert(input_dim1 >= dim0, "input_dim1 must be greater than dim0");
        if (dim0 > 0)
            memcpy(bias, biasDelta, sizeof(float) * dim0);
        if (input_dim1 - dim0 > 0)
        {
            memcpy(predWeights, predParams, sizeof(float) * input_dim0 * (input_dim1 - dim0));
            memcpy(predBias, predParams + input_dim0 * (input_dim1 - dim0), sizeof(float) * (input_dim1 - dim0));
        }
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(input_dim1, batch_size)];
        for (int i = 0; i < input_dim1 - dim0; i++)
        {
            buf[i] = predBias[i];
            for (int j = 0; j < input_dim0; j++)
            {
                buf[i] += predWeights[i][j] * inputs[j];
            }
            // pred Node has no relu activation except output layer
        }
        if (dim0 > 0)
        {
            p_layer->feedforward(inputs, &buf[input_dim1 - dim0]);
            for (int i = 0; i < dim0; i++)
            {
                buf[input_dim1 - dim0 + i] = relu(buf[input_dim1 - dim0 + i] + bias[i]);
            }
        }

        _rest.feedforward(buf, outputs);
    }

    void set_shared(SharedWeights<input_dim0, dim0, input_dim1, args...> *shared)
    {
        p_layer = &(shared->_layer);
        _rest.set_shared(&(shared->_rest));
    }

    Layer<input_dim0, dim0> *p_layer;

    float bias[dim0];
    float predWeights[input_dim1 - dim0][input_dim0];
    float predBias[input_dim1 - dim0];

    BSDFNet<input_dim1, args...> _rest;
    static const int num_pred = (input_dim1 - dim0) * (input_dim0 + 1) + decltype(_rest)::num_pred;
    static const int num_bias = dim0 + decltype(_rest)::num_bias;
};

// input_dim1 should be 1.
// BSDFNet<32,1,1> or BSDFNet<32,0,1>
template <int input_dim0, int dim0, int input_dim1>
class BSDFNet<input_dim0, dim0, input_dim1>
{
public:
    BSDFNet(float *biasDelta, float *predParams)
    {
        static_assert(dim0 <= input_dim1, "input_dim1 must be greater than dim0");
        static_assert(input_dim1 == 1, "output channel should be 1");
        if (dim0 > 0)
            memcpy(bias, biasDelta, sizeof(float) * dim0);
        if (input_dim1 - dim0 > 0)
        {
            memcpy(predWeights, predParams, sizeof(float) * input_dim0 * (input_dim1 - dim0));
            memcpy(predBias, predParams + input_dim0 * (input_dim1 - dim0), sizeof(float) * (input_dim1 - dim0));
        }
    }
    inline void feedforward(float *inputs, float *outputs) const
    {

        for (int i = 0; i < input_dim1 - dim0; i++)
        {
            outputs[i] = predBias[i];
            for (int j = 0; j < input_dim0; j++)
            {
                outputs[i] += predWeights[i][j] * inputs[j];
            }
            outputs[i] = relu(outputs[i]); // only output layer need a relu
        }
        if (dim0 > 0)
        {
            p_layer->feedforward(inputs, &outputs[input_dim1 - dim0]);
            for (int i = 0; i < dim0; i++)
            {
                outputs[input_dim1 - dim0 + i] = relu(outputs[input_dim1 - dim0 + i] + bias[i]);
            }
        }
    }

    void set_shared(SharedWeights<input_dim0, dim0, input_dim1> *shared)
    {
        p_layer = &(shared->_layer);
    }

    Layer<input_dim0, dim0> *p_layer;

    float bias[dim0];
    float predWeights[input_dim1 - dim0][input_dim0];
    float predBias[input_dim1 - dim0];

    static const int num_pred = (input_dim1 - dim0) * (input_dim0 + 1);
    static const int num_bias = dim0;
};

#endif /* __MLP_H_ */