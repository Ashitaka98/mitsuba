#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
#include <misc/sh_encode.h>
#include <misc/MLP.h>
#include <misc/utils.h>
MTS_NAMESPACE_BEGIN
#ifndef MIU
#define MIU 32
#endif
#ifndef SH_ENCODING_DIM
#define SH_ENCODING_DIM 81
#endif
#define INPUT_DIM (2 * SH_ENCODING_DIM + 6)
class SVNeuralBSDF : public BSDF
{

    using BRDFNet = injected_MLP<INPUT_DIM, 32, 64, 64, 96, 96, 128, 128, 160, 160, 192, 192, 224, 1>;

public:
    SVNeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# SVNeuralBSDF #############");
        sh_encode = SH_Encode::getInstance();
        m_weightsPath = props.getString("weightspath", "");
        m_flag_isSVBSDF = props.getBoolean("isSV", false);
        m_flag_useWhWd = props.getBoolean("useWhWd", false);
        m_injectedUnitsPath_r = props.getString("unitsPath_r", "");
        m_injectedUnitsPath_g = props.getString("unitsPath_g", "");
        m_injectedUnitsPath_b = props.getString("unitsPath_b", "");
        m_injectedUnitsTexturePath_r = props.getString("unitsTexturePath_r", "");
        m_injectedUnitsTexturePath_g = props.getString("unitsTexturePath_g", "");
        m_injectedUnitsTexturePath_b = props.getString("unitsTexturePath_b", "");
        m_textureWidth = props.getInteger("textureWidth", 0);
        m_textureHeight = props.getInteger("textureHeight", 0);
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
    void configure() override
    {
        m_components.push_back(EDiffuseReflection | EFrontSide);
        float weights[BRDFNet::num_weights];
        FILE *model_file = fopen(m_weightsPath.c_str(), "r");
        Log(EInfo, "MLP weights number: %d", BRDFNet::num_weights);
        for (int i = 0; i < BRDFNet::num_weights; i++)
        {
            fscanf(model_file, "%f", &weights[i]);
        }
        fclose(model_file);
        if (m_flag_isSVBSDF)
        {
            m_svbrdf_r.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_g.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_b.resize(m_textureWidth * m_textureHeight);
            float injected_units[BRDFNet::num_injected];
            FILE *injectedUnits_file = fopen(m_injectedUnitsTexturePath_r.c_str(), "r");
            Log(EInfo, "MLP injected units number: %d", BRDFNet::num_injected);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_r[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
                }
            }
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_injectedUnitsTexturePath_g.c_str(), "r");
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_g[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
                }
            }
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_injectedUnitsTexturePath_b.c_str(), "r");
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_b[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
                }
            }
            fclose(injectedUnits_file);
        }
        else
        {
            float injected_units[BRDFNet::num_injected];
            FILE *injectedUnits_file = fopen(m_injectedUnitsPath_r.c_str(), "r");
            Log(EInfo, "MLP injected units number: %d", BRDFNet::num_injected);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_r = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_injectedUnitsPath_g.c_str(), "r");
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_g = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_injectedUnitsPath_b.c_str(), "r");
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_b = std::unique_ptr<BRDFNet>(new BRDFNet(weights, injected_units));
            fclose(injectedUnits_file);
        }

        Log(EInfo, "SVNeuralBSDF configured");
        BSDF::configure();
    }
    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const override
    {
        if (measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0)
            return Spectrum(0.0);
        Vector wi = bRec.wi;
        Vector wo = bRec.wo;
        float inputs[INPUT_DIM];
        float pred[3];
        Omega_io_xyz wiwo{wi.x, wi.y, wi.z, wo.x, wo.y, wo.z};
        Omega_io_xyz whwd = SH_Encode::convert_to_rusinkiewicz_xyz(wi.x, wi.y, wi.z, wo.x, wo.y, wo.z);
        SH_basis<SH_DIMS> sh_i, sh_o;
        if (m_flag_useWhWd)
        {
            sh_i = sh_encode->eval_sh_basis<SH_DIMS>(whwd.xyz[0], whwd.xyz[1], whwd.xyz[2]);
            sh_o = sh_encode->eval_sh_basis<SH_DIMS>(whwd.xyz[3], whwd.xyz[4], whwd.xyz[5]);
            memcpy(inputs, whwd.xyz, sizeof(float) * 6);
        }
        else
        {
            sh_i = sh_encode->eval_sh_basis<SH_DIMS>(wiwo.xyz[0], wiwo.xyz[1], wiwo.xyz[2]);
            sh_o = sh_encode->eval_sh_basis<SH_DIMS>(wiwo.xyz[3], wiwo.xyz[4], wiwo.xyz[5]);
            memcpy(inputs, wiwo.xyz, sizeof(float) * 6);
        }
        memcpy(&inputs[6], sh_i.sh, sizeof(float) * SH_DIMS);
        memcpy(&inputs[6 + SH_DIMS], sh_o.sh, sizeof(float) * SH_DIMS);
        if (m_flag_isSVBSDF)
        {
            int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
            x = math::clamp(x, 0, m_textureWidth);
            y = math::clamp(y, 0, m_textureHeight - 1);

            int texel = m_textureWidth * y + x;
            m_svbrdf_r[texel]->feedforward(inputs, &pred[0]);
            m_svbrdf_g[texel]->feedforward(inputs, &pred[1]);
            m_svbrdf_b[texel]->feedforward(inputs, &pred[2]);
        }
        else
        {
            m_brdf_r->feedforward(inputs, &pred[0]);
            m_brdf_g->feedforward(inputs, &pred[1]);
            m_brdf_b->feedforward(inputs, &pred[2]);
        }
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
        return eval(bRec);
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        if (Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        Vector wi = bRec.wi;
        Vector wo = warp::squareToUniformHemisphere(sample);
        bRec.wo = wo;
        pdf = warp::squareToUniformHemispherePdf();
        return eval(bRec);
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
    std::string m_weightsPath;
    std::string m_injectedUnitsPath_r, m_injectedUnitsPath_g, m_injectedUnitsPath_b;
    std::string m_injectedUnitsTexturePath_r, m_injectedUnitsTexturePath_g, m_injectedUnitsTexturePath_b;

    std::unique_ptr<BRDFNet> m_brdf_r, m_brdf_g, m_brdf_b;

    std::vector<std::unique_ptr<BRDFNet>> m_svbrdf_r, m_svbrdf_g, m_svbrdf_b;
    bool m_flag_isSVBSDF;
    bool m_flag_useWhWd;
    SH_Encode *sh_encode;
    int m_textureWidth, m_textureHeight;
};

MTS_IMPLEMENT_CLASS_S(SVNeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(SVNeuralBSDF, "Spatial Variant Neural Network Based BSDF");
MTS_NAMESPACE_END