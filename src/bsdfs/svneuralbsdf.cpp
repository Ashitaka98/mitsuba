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
    using BTDFNet = injected_MLP<INPUT_DIM, 32, 64, 64, 96, 96, 128, 128, 160, 160, 192, 192, 224, 1>;

public:
    SVNeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# SVNeuralBSDF #############");
        sh_encode = SH_Encode::getInstance();
        m_BRDFWeightsPath = props.getString("BRDFWeightsPath");
        m_BTDFWeightsPath = props.getString("BTDFWeightsPath", "");
        m_flag_isSV = props.getBoolean("isSV", false);
        m_flag_isBSDF = props.getBoolean("isBSDF", false);
        m_flag_useWhWd = props.getBoolean("useWhWd", false);
        m_BRDFDeltaPath_r = props.getString("BRDFDeltaPath_r", "");
        m_BRDFDeltaPath_g = props.getString("BRDFDeltaPath_g", "");
        m_BRDFDeltaPath_b = props.getString("BRDFDeltaPath_b", "");
        m_BRDFDeltaTexturePath_r = props.getString("BRDFDeltaTexturePath_r", "");
        m_BRDFDeltaTexturePath_g = props.getString("BRDFDeltaTexturePath_g", "");
        m_BRDFDeltaTexturePath_b = props.getString("BRDFDeltaTexturePath_b", "");
        m_BTDFDeltaPath_r = props.getString("BTDFDeltaPath_r", "");
        m_BTDFDeltaPath_g = props.getString("BTDFDeltaPath_g", "");
        m_BTDFDeltaPath_b = props.getString("BTDFDeltaPath_b", "");
        m_BTDFDeltaTexturePath_r = props.getString("BTDFDeltaTexturePath_r", "");
        m_BTDFDeltaTexturePath_g = props.getString("BTDFDeltaTexturePath_g", "");
        m_BTDFDeltaTexturePath_b = props.getString("BTDFDeltaTexturePath_b", "");
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
        m_components.push_back(EGlossyReflection | EFrontSide);
        if (m_flag_isBSDF)
        {
            m_components.push_back(EGlossyTransmission | EBackSide);
        }

        float BRDFWeights[BRDFNet::num_weights];
        FILE *model_file = fopen(m_BRDFWeightsPath.c_str(), "r");
        assert(model_file);
        Log(EInfo, "BRDFNet weights number: %d", BRDFNet::num_weights);
        for (int i = 0; i < BRDFNet::num_weights; i++)
        {
            fscanf(model_file, "%f", &BRDFWeights[i]);
        }
        fclose(model_file);

        float BTDFWeights[BTDFNet::num_weights];
        if (m_flag_isBSDF)
        {
            model_file = fopen(m_BTDFWeightsPath.c_str(), "r");
            assert(model_file);
            Log(EInfo, "BTDFNet weights number: %d", BTDFNet::num_weights);
            for (int i = 0; i < BTDFNet::num_weights; i++)
            {
                fscanf(model_file, "%f", &BTDFWeights[i]);
            }
            fclose(model_file);
        }

        if (m_flag_isSV)
        {
            m_svbrdf_r.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_g.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_b.resize(m_textureWidth * m_textureHeight);
            if (m_flag_isBSDF)
            {
                m_svbtdf_r.resize(m_textureWidth * m_textureHeight);
                m_svbtdf_g.resize(m_textureWidth * m_textureHeight);
                m_svbtdf_b.resize(m_textureWidth * m_textureHeight);
            }
            float injected_units[BRDFNet::num_injected];
            FILE *injectedUnits_file = fopen(m_BRDFDeltaTexturePath_r.c_str(), "r");
            assert(injectedUnits_file);
            Log(EInfo, "BRDFNet injected units number: %d", BRDFNet::num_injected);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_r[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
                }
            }
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_BRDFDeltaTexturePath_g.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_g[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
                }
            }
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_BRDFDeltaTexturePath_b.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    m_svbrdf_b[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
                }
            }
            fclose(injectedUnits_file);
            if (m_flag_isBSDF)
            {
                float injected_units[BTDFNet::num_injected];
                FILE *injectedUnits_file = fopen(m_BTDFDeltaTexturePath_r.c_str(), "r");
                assert(injectedUnits_file);
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        m_svbtdf_r[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                    }
                }
                fclose(injectedUnits_file);
                injectedUnits_file = fopen(m_BTDFDeltaTexturePath_g.c_str(), "r");
                assert(injectedUnits_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        m_svbtdf_g[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                    }
                }
                fclose(injectedUnits_file);
                injectedUnits_file = fopen(m_BTDFDeltaTexturePath_b.c_str(), "r");
                assert(injectedUnits_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        m_svbtdf_b[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                    }
                }
            }
        }
        else
        {
            float injected_units[BRDFNet::num_injected];
            FILE *injectedUnits_file = fopen(m_BRDFDeltaPath_r.c_str(), "r");
            assert(injectedUnits_file);
            Log(EInfo, "BRDFNet injected units number: %d", BRDFNet::num_injected);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_r = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_BRDFDeltaPath_g.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_g = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
            fclose(injectedUnits_file);
            injectedUnits_file = fopen(m_BRDFDeltaPath_b.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            m_brdf_b = std::unique_ptr<BRDFNet>(new BRDFNet(BRDFWeights, injected_units));
            fclose(injectedUnits_file);
            if (m_flag_isBSDF)
            {
                float injected_units[BTDFNet::num_injected];
                FILE *injectedUnits_file = fopen(m_BTDFDeltaPath_r.c_str(), "r");
                assert(injectedUnits_file);
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                m_btdf_r = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                fclose(injectedUnits_file);
                injectedUnits_file = fopen(m_BTDFDeltaPath_g.c_str(), "r");
                assert(injectedUnits_file);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                m_btdf_g = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                fclose(injectedUnits_file);
                injectedUnits_file = fopen(m_BTDFDeltaPath_b.c_str(), "r");
                assert(injectedUnits_file);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                m_btdf_b = std::unique_ptr<BTDFNet>(new BTDFNet(BTDFWeights, injected_units));
                fclose(injectedUnits_file);
            }
        }

        Log(EInfo, "SVNeuralBSDF configured");
        BSDF::configure();
    }
    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const override
    {
        if (measure != ESolidAngle)
            return Spectrum(0.0);
        if (!m_flag_isBSDF && (Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0))
            return Spectrum(0.0);
        Vector wi;
        Vector wo;
        if (bRec.wi.z < 0)
        {
            if (bRec.wo.z > 0)
            {
                // reciprocity
                wi = bRec.wo;
                wo = bRec.wi;
            }
            else
            {
                wi = -bRec.wi;
                wo = -bRec.wo;
            }
        }
        else
        {
            wi = bRec.wi;
            wo = bRec.wo;
        }
        float inputs[INPUT_DIM];
        float pred[3];
        Omega_io_xyz wiwo{wi.x, wi.y, wi.z, wo.x, wo.y, wo.z};
        Omega_io_xyz whwd;
        SH_basis<SH_DIMS> sh_i, sh_o;
        if (wo.z > 0 && m_flag_useWhWd)
        {
            // convert to rusinkiewicz coordinate frame
            whwd = SH_Encode::convert_to_rusinkiewicz_xyz(wi.x, wi.y, wi.z, wo.x, wo.y, wo.z);
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
        if (wo.z > 0)
        {
            // feedforward BRDFNet
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
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
        }
        else
        {
            // feedforward BTDFNet
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
                y = math::clamp(y, 0, m_textureHeight - 1);

                int texel = m_textureWidth * y + x;
                m_svbtdf_r[texel]->feedforward(inputs, &pred[0]);
                m_svbtdf_g[texel]->feedforward(inputs, &pred[1]);
                m_svbtdf_b[texel]->feedforward(inputs, &pred[2]);
            }
            else
            {
                m_btdf_r->feedforward(inputs, &pred[0]);
                m_btdf_g->feedforward(inputs, &pred[1]);
                m_btdf_b->feedforward(inputs, &pred[2]);
            }
        }
        inverse_miu_law_compression(MIU, pred, 3);
        return Spectrum(pred) * abs(Frame::cosTheta(bRec.wo));
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const override
    {
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        float pdf;
        if (m_flag_isBSDF)
        {
            bRec.wo = warp::squareToUniformSphere(sample);
            pdf = warp::squareToUniformSpherePdf();
        }
        else
        {
            bRec.wo = warp::squareToUniformHemisphere(sample);
            pdf = warp::squareToUniformHemispherePdf();
        }
        return eval(bRec) / pdf;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        if (m_flag_isBSDF)
        {
            bRec.wo = warp::squareToUniformSphere(sample);
            pdf = warp::squareToUniformSpherePdf();
        }
        else
        {
            bRec.wo = warp::squareToUniformHemisphere(sample);
            pdf = warp::squareToUniformHemispherePdf();
        }
        return eval(bRec);
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return 0;
        float pdf;
        if (m_flag_isBSDF)
        {
            pdf = warp::squareToUniformSpherePdf();
        }
        else
        {
            pdf = warp::squareToUniformHemispherePdf();
        }
        return pdf;
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
    std::string m_BRDFWeightsPath;
    std::string m_BRDFDeltaPath_r, m_BRDFDeltaPath_g, m_BRDFDeltaPath_b;
    std::string m_BRDFDeltaTexturePath_r, m_BRDFDeltaTexturePath_g, m_BRDFDeltaTexturePath_b;

    std::string m_BTDFWeightsPath;
    std::string m_BTDFDeltaPath_r, m_BTDFDeltaPath_g, m_BTDFDeltaPath_b;
    std::string m_BTDFDeltaTexturePath_r, m_BTDFDeltaTexturePath_g, m_BTDFDeltaTexturePath_b;

    std::unique_ptr<BRDFNet> m_brdf_r, m_brdf_g, m_brdf_b;
    std::unique_ptr<BTDFNet> m_btdf_r, m_btdf_g, m_btdf_b;

    std::vector<std::unique_ptr<BRDFNet>> m_svbrdf_r, m_svbrdf_g, m_svbrdf_b;
    std::vector<std::unique_ptr<BTDFNet>> m_svbtdf_r, m_svbtdf_g, m_svbtdf_b;
    bool m_flag_isSV;
    bool m_flag_isBSDF;
    bool m_flag_useWhWd;
    SH_Encode *sh_encode;
    int m_textureWidth, m_textureHeight;
};

MTS_IMPLEMENT_CLASS_S(SVNeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(SVNeuralBSDF, "Spatial Variant Neural Network Based BSDF");
MTS_NAMESPACE_END