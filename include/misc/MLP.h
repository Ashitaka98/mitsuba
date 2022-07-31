#pragma once
#ifndef __MLP_H_
#define __MLP_H_
#include <string.h>
#ifdef MLP_DEBUG
#include <iostream>
#endif
inline float relu(float x)
{
    return x > 0 ? x : 0;
}

template <int input_dim, int output_dim>
class Layer
{
public:
    Layer()
    {
        for (int i = 0; i < output_dim; i++)
        {
            _bias[i] = 0;
            for (int j = 0; j < input_dim; j++)
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
        }
        for (int i = 0; i < output_dim; i++)
        {
            _bias[i] = *p;
            p++;
        }
    }
    inline void feedforward(float *inputs, float *outputs) const
    {
        for (int i = 0; i < output_dim; i++)
        {
            outputs[i] = _bias[i];
            for (int j = 0; j < input_dim; j++)
            {
                outputs[i] += _weights[i][j] * inputs[j];
            }
            outputs[i] = relu(outputs[i]);
        }
#ifdef MLP_DEBUG
        for (int i = 0; i < output_dim; i++)
            std::cout << outputs[i] << " ";
        std::cout << std::endl;
#endif
    }
    float _weights[output_dim][input_dim];
    float _bias[output_dim];
};

template <int input_dim, int dim0, int... args>
class MLP
{
public:
    MLP() : _layer0()
    {
    }
    MLP(float *buffer) : _layer0(buffer), _rest(buffer + input_dim * dim0 + dim0)
    {
    }
    /*
        feedforward should be thread-safe
    */
    inline void feedforward(float *inputs, float *outputs) const
    {
        float buf[dim0];
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
    inline void feedforward(float *inputs, float *outputs) const
    {
        _layer.feedforward(inputs, outputs);
    }
    static const int num_weights = input_dim * output_dim + output_dim;
    Layer<input_dim, output_dim> _layer;
};

template <int input_dim0, int dim0, int input_dim1, int... args>
class injected_MLP
{
public:
    injected_MLP() : _layer0()
    {
    }
    injected_MLP(float *weights, float *injected_units) : _layer0(weights), _rest(weights + input_dim0 * dim0 + dim0, injected_units + input_dim1 - dim0)
    {
        memcpy(injected, injected_units, sizeof(float) * (input_dim1 - dim0));
    }
    inline void feedforward(float *inputs, float *outputs) const
    {
        float buf[input_dim1];
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

template <int input_dim, int output_dim>
class injected_MLP
{
public:
    injected_MLP() : _layer0()
    {
    }
    injected_MLP(float *weights, float *injected_units) : _layer(weights)
    {
    }
    inline void feedforward(float *inputs, float *outputs) const
    {
        _layer.feedforward(inputs, outputs);
    }
    static const int num_weights = input_dim * output_dim + output_dim;
    static const int num_injected = 0;
    Layer<input_dim, output_dim> _layer;
};
#endif /* __MLP_H_ */