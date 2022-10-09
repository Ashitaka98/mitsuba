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
            outputs[i] = relu(outputs[i]);
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

template <int input_dim, int dim0, int... args>
class MLP
{
public:
    MLP(float *buffer) : _layer0(buffer), _rest(buffer + input_dim * dim0 + dim0)
    {
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(dim0, batch_size)];
        _layer0.feedforward(inputs, buf);
        _rest.feedforward(buf, outputs);
    }
    Layer<input_dim, dim0> _layer0;
    MLP<dim0, args...> _rest;
    static const int num_weights = input_dim * dim0 + dim0 + decltype(_rest)::num_weights;
};

template <int input_dim, int output_dim>
class MLP<input_dim, output_dim>
{
public:
    MLP() : _layer()
    {
    }
    MLP(float *buffer) : _layer(buffer)
    {
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        _layer.feedforward(inputs, outputs);
    }
    static const int num_weights = input_dim * output_dim + output_dim;
    Layer<input_dim, output_dim> _layer;
};

// injected_MLP<168, 32, 64, 64, 96, 96, 128, 128, 160, 160, 192, 192, 224, 1>;
template <int input_dim0, int dim0, int input_dim1, int... args>
class injected_MLP
{
public:
    injected_MLP(float *weights, float *injected_units) : _layer0(weights), _rest(weights + input_dim0 * dim0 + dim0, injected_units + input_dim1 - dim0)
    {
        memcpy(injected, injected_units, sizeof(float) * (input_dim1 - dim0));
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(input_dim1, batch_size)];
        _layer0.feedforward(inputs, buf);
        memcpy(&buf[dim0], injected, sizeof(float) * (input_dim1 - dim0));
        _rest.feedforward(buf, outputs);
    }
    Layer<input_dim0, dim0> _layer0;
    float injected[input_dim1 - dim0];
    injected_MLP<input_dim1, args...> _rest;
    static const int num_weights = input_dim0 * dim0 + dim0 + decltype(_rest)::num_weights;
    static const int num_injected = input_dim1 - dim0 + decltype(_rest)::num_injected;
};

template <int input_dim0, int dim0, int input_dim1, int dim1>
class injected_MLP<input_dim0, dim0, input_dim1, dim1>
{
public:
    injected_MLP() : _layer0()
    {
    }
    injected_MLP(float *weights, float *injected_units) : _layer0(weights), _layer1(weights + input_dim0 * dim0 + dim0)
    {
        memcpy(injected, injected_units, sizeof(float) * (input_dim1 - dim0));
    }
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(input_dim1, batch_size)];
        _layer0.feedforward(inputs, buf);
        memcpy(&buf[dim0], injected, sizeof(float) * (input_dim1 - dim0));
        _layer1.feedforward(buf, outputs);
    }
    Layer<input_dim0, dim0> _layer0;
    float injected[input_dim1 - dim0];
    Layer<input_dim1, dim1> _layer1;
    static const int num_weights = input_dim0 * dim0 + dim0 + input_dim1 * dim1 + dim1;
    static const int num_injected = input_dim1 - dim0;
};

// injected_MLP_v2<168,32,0,0,64,31,32,1,64,31,32,1,32,0,32,1>
template <int input_dim0, int dim0, int input_dim0v2, int dim0v2, int input_dim1, int dim1, int input_dim1v2, int... args>
class injected_MLP_v2
{
public:
    injected_MLP_v2(float *weights, float *injected_units, float *injected_weights) : _layer0(weights), _rest(weights + input_dim0 * dim0 + dim0, injected_units + input_dim1 - dim0 - dim0v2, injected_weights + input_dim0v2 * dim0v2 + dim0v2)
    {
        // static_assert(input_dim1 >= dim0 + dim0v2);
        // static_assert(input_dim1v2 == dim0 + dim0v2);
        if (input_dim1 - dim0 - dim0v2 > 0)
            memcpy(_injected, injected_units, sizeof(float) * (input_dim1 - dim0 - dim0v2));
        if (dim0v2 > 0)
        {
            memcpy(_injected_weights, injected_weights, sizeof(float) * input_dim0v2 * dim0v2);
            memcpy(_injected_bias, injected_weights + input_dim0v2 * dim0v2, sizeof(float) * dim0v2);
        }
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(input_dim1, batch_size)];
        for (int i = 0; i < dim0v2; i++)
        {
            buf[i] = _injected_bias[i];
            for (int j = 0; j < input_dim0v2; j++)
            {
                buf[i] += _injected_weights[i][j] * inputs[j];
            }
        }
        if (dim0 > 0)
            _layer0.feedforward(inputs, &buf[dim0v2]);
        if (input_dim1 - dim0 - dim0v2 > 0)
            memcpy(&buf[dim0v2 + dim0], _injected, sizeof(float) * (input_dim1 - dim0 - dim0v2));
        _rest.feedforward(buf, outputs);
    }
    Layer<input_dim0, dim0> _layer0;

    float _injected[input_dim1 - dim0 - dim0v2];
    float _injected_weights[dim0v2][input_dim0v2];
    float _injected_bias[dim0v2];

    injected_MLP_v2<input_dim1, dim1, input_dim1v2, args...> _rest;
    static const int num_weights = input_dim0 * dim0 + dim0 + decltype(_rest)::num_weights;
    static const int num_injected_weights = input_dim0v2 * dim0v2 + dim0v2 + decltype(_rest)::num_injected_weights;
    static const int num_injected = input_dim1 - dim0 - dim0v2 + decltype(_rest)::num_injected;
};

template <int input_dim0, int dim0, int input_dim0v2, int dim0v2, int input_dim1, int dim1, int input_dim1v2, int dim1v2>
class injected_MLP_v2<input_dim0, dim0, input_dim0v2, dim0v2, input_dim1, dim1, input_dim1v2, dim1v2>
{
public:
    injected_MLP_v2(float *weights, float *injected_units, float *injected_weights) : _layer0(weights), _layer1(weights + input_dim0 * dim0 + dim0)
    {
        // static_assert(input_dim1v2 == dim0 + dim0v2);
        // static_assert(input_dim1 >= dim0 + dim0v2);
        if (input_dim1 - dim0 - dim0v2 > 0)
            memcpy(_injected, injected_units, sizeof(float) * (input_dim1 - dim0 - dim0v2));
        if (dim0v2 > 0)
        {
            memcpy(_injected_weights, injected_weights, sizeof(float) * input_dim0v2 * dim0v2);
            memcpy(_injected_bias, injected_weights + input_dim0v2 * dim0v2, sizeof(float) * dim0v2);
            memcpy(_injected_weights2, injected_weights + input_dim0v2 * dim0v2 + dim0v2, sizeof(float) * input_dim1v2 * dim1v2);
            memcpy(_injected_bias2, injected_weights + input_dim0v2 * dim0v2 + dim0v2 + input_dim1v2 * dim1v2, sizeof(float) * dim1v2);
        }
    }
    inline void feedforward(float *inputs, float *outputs) const
    {
        __attribute__((aligned(64))) float buf[RoundUp(input_dim1, batch_size)];
        for (int i = 0; i < dim0v2; i++)
        {
            buf[i] = _injected_bias[i];
            for (int j = 0; j < input_dim0v2; j++)
            {
                buf[i] += _injected_weights[i][j] * inputs[j];
            }
        }
        if (dim0 > 0)
            _layer0.feedforward(inputs, &buf[dim0v2]);
        if (input_dim1 - dim0 - dim0v2 > 0)
            memcpy(&buf[dim0v2 + dim0], _injected, sizeof(float) * (input_dim1 - dim0 - dim0v2));
        for (int i = 0; i < dim1v2; i++)
        {
            outputs[i] = _injected_bias2[i];
            for (int j = 0; j < input_dim1v2; j++)
            {
                outputs[i] += _injected_weights2[i][j] * buf[j];
            }
        }
        if (dim1 > 0)
            _layer1.feedforward(buf, &outputs[dim1v2]);
    }
    Layer<input_dim0, dim0> _layer0;
    float _injected[input_dim1 - dim0 - dim0v2];
    float _injected_weights[dim0v2][input_dim0v2];
    float _injected_bias[dim0v2];
    Layer<input_dim1, dim1> _layer1;

    float _injected_weights2[dim1v2][input_dim1v2];
    float _injected_bias2[dim1v2];

    static const int num_weights = input_dim0 * dim0 + dim0 + input_dim1 * dim1 + dim1;
    static const int num_injected_weights = input_dim0v2 * dim0v2 + dim0v2 + input_dim1v2 * dim1v2 + dim1v2;
    static const int num_injected = input_dim1 - dim0 - dim0v2;
};

#endif /* __MLP_H_ */