#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
#include <misc/MLP.h>
#include <misc/sh_encode.h>
#include <misc/utils.h>
#ifndef MIU
#define MIU 32
#endif
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
        m_components.push_back(EDiffuseReflection | EFrontSide);
        float weights[BRDFNet::num_weights];
        FILE *model_file = fopen(m_modelpath.c_str(), "r");
        Log(EInfo, "MLP weights number: %d", BRDFNet::num_weights);
        for (int i = 0; i < BRDFNet::num_weights; i++)
        {
            fscanf(model_file, "%f", &weights[i]);
        }
        m_brdf = std::unique_ptr<BRDFNet>(new BRDFNet(weights));
        BSDF::configure();
    }

    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const override
    {
        if (measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0)
            return Spectrum(0.0);
        Vector wi = bRec.wi;
        Vector wo = bRec.wo;
        Omega_io_xyz wiwo{wi.x, wi.y, wi.z, wo.x, wo.y, wo.z};
        float pred[3];
        m_brdf->feedforward(wiwo.xyz, pred);
        inverse_miu_law_compression(MIU, pred, 3);
        return Spectrum(pred) * Frame::cosTheta(bRec.wo);
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const override
    {
        if (Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        Vector wi = bRec.wi;
        Vector wo = warp::squareToUniformHemisphere(sample);
        bRec.wo = wo;
        Omega_io_xyz wiwo{wi.x, wi.y, wi.z, wo.x, wo.y, wo.z};
        float pred[3];
        m_brdf->feedforward(wiwo.xyz, pred);
        inverse_miu_law_compression(MIU, pred, 3);
        return Spectrum(pred) * Frame::cosTheta(bRec.wo) / warp::squareToUniformHemispherePdf();
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        if (Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        Vector wi = bRec.wi;
        Vector wo = warp::squareToUniformHemisphere(sample);
        bRec.wo = wo;
        Omega_io_xyz wiwo{wi.x, wi.y, wi.z, wo.x, wo.y, wo.z};
        float pred[3];
        m_brdf->feedforward(wiwo.xyz, pred);
        inverse_miu_law_compression(MIU, pred, 3);
        pdf = warp::squareToUniformHemispherePdf();
        return Spectrum(pred) * Frame::cosTheta(bRec.wo) / pdf;
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        if (measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;
        return warp::squareToUniformHemispherePdf();
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
    std::unique_ptr<BRDFNet> m_brdf;
};

MTS_IMPLEMENT_CLASS_S(NeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(NeuralBSDF, "Neural Network based BSDF");
MTS_NAMESPACE_END