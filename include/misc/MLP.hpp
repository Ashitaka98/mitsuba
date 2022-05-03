#ifdef DEBUG
#include <iostream>
#endif

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
    inline void feedforward(float *inputs, float *outputs)
    {
        for (int i = 0; i < output_dim; i++)
        {
            outputs[i] = _bias[i];
            for (int j = 0; j < input_dim; j++)
            {
                outputs[i] += _weights[i][j] * inputs[j];
            }
        }
#ifdef DEBUG
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
    inline void feedforward(float *inputs, float *outputs)
    {
        float buf[dim0];
        _layer0.feedforward(inputs, buf);
        _rest.feedforward(buf, outputs);
    }
    Layer<input_dim, dim0> _layer0;
    MLP<dim0, args...> _rest;
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
    inline void feedforward(float *inputs, float *outputs)
    {
        _layer.feedforward(inputs, outputs);
    }
    Layer<input_dim, output_dim> _layer;
};