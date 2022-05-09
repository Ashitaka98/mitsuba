#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
#include <onnxruntime_cxx_api.h>
#include <misc/sh_encode.h>
#include <misc/MLP.h>
MTS_NAMESPACE_BEGIN
#ifndef SH_ENCODING_DIM
#define SH_ENCODING_DIM 49
#endif
class SVNeuralBSDF : public BSDF
{

    using BRDFNet = MLP<2 * SH_ENCODING_DIM, 32, 32, 32, 3>;

public:
    SVNeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# SVNeuralBSDF #############");
        m_modelpath = props.getString("modelpath", "");
        m_alpha0 = props.getFloat("alpha0", 0.1);
        m_alpha1 = props.getFloat("alpha1", 0.1);
        m_eta0 = props.getFloat("eta0", 1.2);
        m_eta1 = props.getFloat("eta1", 1.2);
        m_normal0 = props.getVector("normal0", Vector(0.0, 0.0, 1.0));
        m_normal1 = props.getVector("normal1", Vector(0.0, 0.0, 1.0));
        m_sigmaT = props.getFloat("sigmaT", 1.0);
        m_albedo = props.getSpectrum("albedo", Spectrum(1.0));
        m_flag_roughnessmap0 = false;
        m_flag_roughnessmap1 = false;
        m_flag_normalmap0 = false;
        m_flag_normalmap1 = false;
        m_flag_sigmaTmap = false;
        m_flag_albedomap = false;
        m_flag_isSVBSDF = false;
        Log(EDebug, "alpha0:%f alpha1:%f eta0:%f eta1:%f\n\
            normal0: (%f, %f, %f) normal1: (%f, %f, %f)\n\
            sigmaT: %f albedo: (%f, %f, %f)",
            m_alpha0, m_alpha1, m_eta0, m_eta1,
            m_normal0[0], m_normal0[1], m_normal0[2], m_normal1[0], m_normal1[1], m_normal1[2],
            m_sigmaT, m_albedo[0], m_albedo[1], m_albedo[2]);

        Log(EInfo, "SVNeuralBSDF constructed");
    }
    SVNeuralBSDF(Stream *stream, InstanceManager *manager) : BSDF(stream, manager)
    {
        Log(EError, " Not implemented.");
    }
    ~SVNeuralBSDF()
    {
    }
    void serialize(Stream *stream, InstanceManager *manager) const override
    {
        Log(EError, "Not implemented.");
    }
    void addChild(const std::string &name, ConfigurableObject *child) override
    {
        if (child->getClass()->derivesFrom(MTS_CLASS(Texture2D)))
        {
            if (name == "normal_tex_0")
            {
                Log(EInfo, "[NeuralBSDF]: surface 0 :: normal(texture)");
                m_normalmap0 = static_cast<Texture2D *>(child);
                m_flag_normalmap0 = true;
                m_flag_isSVBSDF = true;
            }
            else if (name == "normal_tex_1")
            {
                Log(EInfo, "[NeuralBSDF]: surface 1 :: normal(texture)");
                m_normalmap1 = static_cast<Texture2D *>(child);
                m_flag_normalmap1 = true;
                m_flag_isSVBSDF = true;
            }
            else if (name == "sigmaT_tex")
            {
                Log(EInfo, "[NeuralBSDF]:  medium :: sigmaT(texture)");
                m_sigmaTmap = static_cast<Texture2D *>(child);
                m_flag_sigmaTmap = true;
                m_flag_isSVBSDF = true;
            }
            else if (name == "albedo_tex")
            {
                Log(EInfo, "[NeuralBSDF]:  medium :: albedo(texture)");
                m_albedomap = static_cast<Texture2D *>(child);
                m_flag_albedomap = true;
                m_flag_isSVBSDF = true;
            }
            else if (name == "roughness_tex_0")
            {
                Log(EInfo, "[NeuralBSDF]: surface 0 :: roughness(texture)");
                m_roughnessmap0 = static_cast<Texture2D *>(child);
                m_flag_roughnessmap0 = true;
                m_flag_isSVBSDF = true;
            }
            else if (name == "roughness_tex_1")
            {
                Log(EInfo, "[NeuralBSDF]: surface 1 :: roughness(texture)");
                m_roughnessmap1 = static_cast<Texture2D *>(child);
                m_flag_roughnessmap1 = true;
                m_flag_isSVBSDF = true;
            }
            else
            {
                BSDF::addChild(name, child);
            }
        }
        else
        {
            BSDF::addChild(name, child);
        }
    }
    void configure() override
    {

        /** Load .onnx network **/
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        Ort::Env env;
        Ort::Session session(env, m_modelpath.c_str(), session_options);
        Log(EInfo, "NeuralBSDF configured");
    }
    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const override
    {

        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const override
    {
        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        return 1.0;
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

    float m_alpha0;
    float m_alpha1;
    bool m_flag_roughnessmap0;
    bool m_flag_roughnessmap1;
    ref<Texture2D> m_roughnessmap0;
    ref<Texture2D> m_roughnessmap1;

    float m_eta0;
    float m_eta1;

    float m_sigmaT;
    bool m_flag_sigmaTmap;
    ref<Texture2D> m_sigmaTmap;

    Spectrum m_albedo;
    bool m_flag_albedomap;
    ref<Texture2D> m_albedomap;

    const float g = 0;

    Vector m_normal0;
    Vector m_normal1;
    bool m_flag_normalmap0;
    bool m_flag_normalmap1;
    ref<Texture2D> m_normalmap0;
    ref<Texture2D> m_normalmap1;

    ref<BRDFNet> m_brdf;
    bool m_flag_isSVBSDF;
    ref_vector<BRDFNet> m_svbrdf;
};

MTS_IMPLEMENT_CLASS_S(SVNeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(SVNeuralBSDF, "Spatial Variant Neural Network Based BSDF");
MTS_NAMESPACE_END