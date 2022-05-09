#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
#include <onnxruntime_cxx_api.h>
#include <misc/MLP.h>
#include <misc/sh_encode.h>
MTS_NAMESPACE_BEGIN
class NeuralBSDF : public BSDF
{

    using BRDFNet = MLP<6, 32, 32, 32, 3>;

public:
    NeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# NeuralBSDF #############");
        m_modelpath = props.getString("modelpath", "");

        Log(EInfo, "NeuralBSDF constructed");
    }
    NeuralBSDF(Stream *stream, InstanceManager *manager) : BSDF(stream, manager)
    {
        Log(EError, " Not implemented.");
    }
    ~NeuralBSDF()
    {
    }
    void serialize(Stream *stream, InstanceManager *manager) const override
    {
        Log(EError, "Not implemented.");
    }
    void configure() override
    {
        float weights[BRDFNet::num_weights];
        FILE *model_file = fopen(m_modelpath.c_str(), "r");
        Log(EInfo, "MLP weights number: %d", BRDFNet::num_weights);
        for (int i = 0; i < BRDFNet::num_weights; i++)
        {
            fscanf(model_file, "%f", &weights[i]);
#ifdef NBSDF_DEBUG
            Log(EInfo, "%d", weights[i]);
#endif
        }
        m_brdf = new BRDFNet(weights);
    }
    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const override
    {
        auto wi = bRec.wi;
        auto wo = bRec.wo;
        auto whwd = SH_Encode::convert_to_rusinkiewicz_xyz(wi.x, wi.y, wi.z, wo.x, wo.y, wo.z);
        float pred[3];
        m_brdf->feedforward(whwd.xyz, pred);
        return Spectrum(pred);
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const override
    {
        Spectrum ret;
        return ret;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        Spectrum ret;
        return ret;
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        return 1;
    }
    Spectrum getDiffuseReflectance(const Intersection &its) const override
    {
        Log(EError, "Not implemented.");
        return Spectrum(0.0);
    }

    Spectrum getSpecularReflectance(const Intersection &its) const override
    {
        Log(EError, "Not implemented.");
        return Spectrum(0.0);
    }

    MTS_DECLARE_CLASS()
private:
    std::string m_modelpath;
    ref<BRDFNet> m_brdf;
};

MTS_IMPLEMENT_CLASS_S(NeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(NeuralBSDF, "Neural Network based BSDF");
MTS_NAMESPACE_END