#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
#include <misc/sh_encode.h>
#include <misc/MLP.h>
#include <misc/utils.h>

#include "microfacet.h"
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

    // using BRDFNet = injected_MLP<INPUT_DIM, 32, 64, 64, 96, 96, 128, 128, 160, 160, 192, 192, 224, 1>;
    // using BTDFNet = injected_MLP<INPUT_DIM, 32, 64, 64, 96, 96, 128, 128, 160, 160, 192, 192, 224, 1>;
    using BRDFNet = injected_MLP_v2<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;
    using BTDFNet = injected_MLP_v2<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;

public:
    SVNeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# SVNeuralBSDF #############");
        sh_encode = SH_Encode::getInstance();
        m_BRDFWeightsPath = props.getString("BRDFWeightsPath");
        m_flag_isSV = props.getBoolean("isSV", false);
        m_flag_isBSDF = props.getBoolean("isBSDF", false);
        m_flag_useWhWd = props.getBoolean("useWhWd", false);
        if (m_flag_isSV)
        {
            m_alphaTexturePath = props.getString("AlphaTexturePath");
            m_textureWidth = props.getInteger("textureWidth");
            m_textureHeight = props.getInteger("textureHeight");
            m_BRDFDeltaTexturePath_r = props.getString("BRDFDeltaTexturePath_r");
            m_BRDFDeltaTexturePath_g = props.getString("BRDFDeltaTexturePath_g");
            m_BRDFDeltaTexturePath_b = props.getString("BRDFDeltaTexturePath_b");
            m_BRDFPredWeightsTexturePath_r = props.getString("BRDFPredWeightsTexturePath_r");
            m_BRDFPredWeightsTexturePath_g = props.getString("BRDFPredWeightsTexturePath_g");
            m_BRDFPredWeightsTexturePath_b = props.getString("BRDFPredWeightsTexturePath_b");
        }
        else
        {
            m_alphaPath = props.getString("AlphaPath");
            m_BRDFDeltaPath_r = props.getString("BRDFDeltaPath_r");
            m_BRDFDeltaPath_g = props.getString("BRDFDeltaPath_g");
            m_BRDFDeltaPath_b = props.getString("BRDFDeltaPath_b");
            m_BRDFPredWeightsPath_r = props.getString("BRDFPredWeightsPath_r");
            m_BRDFPredWeightsPath_g = props.getString("BRDFPredWeightsPath_g");
            m_BRDFPredWeightsPath_b = props.getString("BRDFPredWeightsPath_b");
        }
        if (m_flag_isBSDF)
        {

            m_BTDFWeightsPath = props.getString("BTDFWeightsPath");
            m_eta = props.getFloat("eta");
            m_invEta = 1 / m_eta;
            if (m_flag_isSV)
            {
                m_BTDFDeltaTexturePath_r = props.getString("BTDFDeltaTexturePath_r");
                m_BTDFDeltaTexturePath_g = props.getString("BTDFDeltaTexturePath_g");
                m_BTDFDeltaTexturePath_b = props.getString("BTDFDeltaTexturePath_b");
                m_BTDFPredWeightsTexturePath_r = props.getString("BTDFPredWeightsTexturePath_r");
                m_BTDFPredWeightsTexturePath_g = props.getString("BTDFPredWeightsTexturePath_g");
                m_BTDFPredWeightsTexturePath_b = props.getString("BTDFPredWeightsTexturePath_b");
            }
            else
            {
                m_BTDFDeltaPath_r = props.getString("BTDFDeltaPath_r");
                m_BTDFDeltaPath_g = props.getString("BTDFDeltaPath_g");
                m_BTDFDeltaPath_b = props.getString("BTDFDeltaPath_b");
                m_BTDFPredWeightsPath_r = props.getString("BTDFPredWeightsPath_r");
                m_BTDFPredWeightsPath_g = props.getString("BTDFPredWeightsPath_g");
                m_BTDFPredWeightsPath_b = props.getString("BTDFPredWeightsPath_b");
            }
        }
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
            float injected_weights[BRDFNet::num_injected_weights];
            FILE *injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_r.c_str(), "r");
            assert(injectedWeights_file);
            Log(EInfo, "BRDFNet injected weights number: %d", BRDFNet::num_injected_weights);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    for (int k = 0; k < BRDFNet::num_injected_weights; k++)
                    {
                        fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                    }
                    auto p1 = aligned_alloc(64, sizeof(BRDFNet));
                    auto p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
                    m_svbrdf_r[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p2);
                }
            }
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);
            injectedUnits_file = fopen(m_BRDFDeltaTexturePath_g.c_str(), "r");
            assert(injectedUnits_file);
            injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_g.c_str(), "r");
            assert(injectedWeights_file);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    for (int k = 0; k < BRDFNet::num_injected_weights; k++)
                    {
                        fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                    }
                    auto p1 = aligned_alloc(64, sizeof(BRDFNet));
                    auto p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
                    m_svbrdf_g[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p2);
                }
            }
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);
            injectedUnits_file = fopen(m_BRDFDeltaTexturePath_b.c_str(), "r");
            assert(injectedUnits_file);
            injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_b.c_str(), "r");
            assert(injectedWeights_file);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    for (int k = 0; k < BRDFNet::num_injected; k++)
                    {
                        fscanf(injectedUnits_file, "%f", &injected_units[k]);
                    }
                    for (int k = 0; k < BRDFNet::num_injected_weights; k++)
                    {
                        fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                    }
                    auto p1 = aligned_alloc(64, sizeof(BRDFNet));
                    auto p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
                    m_svbrdf_b[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p2);
                }
            }
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);
            if (m_flag_isBSDF)
            {
                float injected_units[BTDFNet::num_injected];
                FILE *injectedUnits_file = fopen(m_BTDFDeltaTexturePath_r.c_str(), "r");
                assert(injectedUnits_file);
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                float injected_weights[BTDFNet::num_injected_weights];
                FILE *injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_r.c_str(), "r");
                assert(injectedWeights_file);
                Log(EInfo, "BTDFNet injected weights number: %d", BTDFNet::num_injected_weights);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        for (int k = 0; k < BTDFNet::num_injected_weights; k++)
                        {
                            fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                        }
                        auto p1 = aligned_alloc(64, sizeof(BTDFNet));
                        auto p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                        m_svbtdf_r[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p2);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);

                injectedUnits_file = fopen(m_BTDFDeltaTexturePath_g.c_str(), "r");
                assert(injectedUnits_file);
                injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_g.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        for (int k = 0; k < BTDFNet::num_injected_weights; k++)
                        {
                            fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                        }
                        auto p1 = aligned_alloc(64, sizeof(BTDFNet));
                        auto p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                        m_svbtdf_g[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p2);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);

                injectedUnits_file = fopen(m_BTDFDeltaTexturePath_b.c_str(), "r");
                assert(injectedUnits_file);
                injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_b.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        for (int k = 0; k < BTDFNet::num_injected; k++)
                        {
                            fscanf(injectedUnits_file, "%f", &injected_units[k]);
                        }
                        for (int k = 0; k < BTDFNet::num_injected_weights; k++)
                        {
                            fscanf(injectedWeights_file, "%f", &injected_weights[k]);
                        }
                        auto p1 = aligned_alloc(64, sizeof(BTDFNet));
                        auto p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                        m_svbtdf_b[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p2);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            FILE *svAlphaFile = fopen(m_alphaTexturePath.c_str(), "r");
            assert(svAlphaFile);
            for (int i = 0; i < m_textureHeight; i++)
            {
                for (int j = 0; j < m_textureWidth; j++)
                {
                    float alpha;
                    fscanf(svAlphaFile, "%f", &alpha);
                    m_svalpha[i * m_textureWidth + j] = alpha;
                }
            }
            fclose(svAlphaFile);
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
            float injected_weights[BRDFNet::num_injected_weights];
            FILE *injectedWeights_file = fopen(m_BRDFPredWeightsPath_r.c_str(), "r");
            assert(injectedWeights_file);
            Log(EInfo, "BRDFNet injected weights number: %d", BRDFNet::num_injected_weights);
            for (int i = 0; i < BRDFNet::num_injected_weights; i++)
            {
                fscanf(injectedWeights_file, "%f", &injected_weights[i]);
            }
            auto p1 = aligned_alloc(64, sizeof(BRDFNet));
            auto p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
            m_brdf_r = std::unique_ptr<BRDFNet>(p2);
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);

            injectedUnits_file = fopen(m_BRDFDeltaPath_g.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            injectedWeights_file = fopen(m_BRDFPredWeightsPath_g.c_str(), "r");
            assert(injectedWeights_file);
            for (int i = 0; i < BRDFNet::num_injected_weights; i++)
            {
                fscanf(injectedWeights_file, "%f", &injected_weights[i]);
            }
            p1 = aligned_alloc(64, sizeof(BRDFNet));
            p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
            m_brdf_g = std::unique_ptr<BRDFNet>(p2);
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);

            injectedUnits_file = fopen(m_BRDFDeltaPath_b.c_str(), "r");
            assert(injectedUnits_file);
            for (int i = 0; i < BRDFNet::num_injected; i++)
            {
                fscanf(injectedUnits_file, "%f", &injected_units[i]);
            }
            injectedWeights_file = fopen(m_BRDFPredWeightsPath_b.c_str(), "r");
            assert(injectedWeights_file);
            for (int i = 0; i < BRDFNet::num_injected_weights; i++)
            {
                fscanf(injectedWeights_file, "%f", &injected_weights[i]);
            }
            p1 = aligned_alloc(64, sizeof(BRDFNet));
            p2 = new (p1) BRDFNet(BRDFWeights, injected_units, injected_weights);
            m_brdf_b = std::unique_ptr<BRDFNet>(p2);
            fclose(injectedUnits_file);
            fclose(injectedWeights_file);

            if (m_flag_isBSDF)
            {
                float injected_units[BTDFNet::num_injected];
                FILE *injectedUnits_file = fopen(m_BTDFDeltaPath_r.c_str(), "r");
                assert(injectedUnits_file);
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                float injected_weights[BTDFNet::num_injected_weights];
                FILE *injectedWeights_file = fopen(m_BTDFPredWeightsPath_r.c_str(), "r");
                assert(injectedWeights_file);
                Log(EInfo, "BTDFNet injected weights number: %d", BTDFNet::num_injected_weights);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                for (int i = 0; i < BTDFNet::num_injected_weights; i++)
                {
                    fscanf(injectedWeights_file, "%f", &injected_weights[i]);
                }
                auto p1 = aligned_alloc(64, sizeof(BTDFNet));
                auto p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                m_btdf_r = std::unique_ptr<BTDFNet>(p2);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);

                injectedUnits_file = fopen(m_BTDFDeltaPath_g.c_str(), "r");
                assert(injectedUnits_file);
                injectedWeights_file = fopen(m_BTDFPredWeightsPath_g.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                for (int i = 0; i < BTDFNet::num_injected_weights; i++)
                {
                    fscanf(injectedWeights_file, "%f", &injected_weights[i]);
                }
                p1 = aligned_alloc(64, sizeof(BTDFNet));
                p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                m_btdf_g = std::unique_ptr<BTDFNet>(p2);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);

                injectedUnits_file = fopen(m_BTDFDeltaPath_b.c_str(), "r");
                assert(injectedUnits_file);
                injectedWeights_file = fopen(m_BTDFPredWeightsPath_b.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < BTDFNet::num_injected; i++)
                {
                    fscanf(injectedUnits_file, "%f", &injected_units[i]);
                }
                for (int i = 0; i < BTDFNet::num_injected_weights; i++)
                {
                    fscanf(injectedWeights_file, "%f", &injected_weights[i]);
                }
                p1 = aligned_alloc(64, sizeof(BTDFNet));
                p2 = new (p1) BTDFNet(BTDFWeights, injected_units, injected_weights);
                m_btdf_b = std::unique_ptr<BTDFNet>(p2);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            FILE *alphaFile = fopen(m_alphaPath.c_str(), "r");
            assert(alphaFile);
            fscanf(alphaFile, "%f", &m_alpha);
            fclose(alphaFile);
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
        __attribute__((aligned(64))) float inputs[RoundUp(INPUT_DIM, batch_size)];
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

            wiwo.xyz[2] = wiwo.xyz[2] < -0.9999f ? -0.9999f : wiwo.xyz[2];
            wiwo.xyz[2] = wiwo.xyz[2] > 0.9999f ? 0.9999f : wiwo.xyz[2];
            wiwo.xyz[5] = wiwo.xyz[5] < -0.9999f ? -0.9999f : wiwo.xyz[5];
            wiwo.xyz[5] = wiwo.xyz[5] > 0.9999f ? 0.9999f : wiwo.xyz[5];
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
        // std::cout << pred[0] << " " << pred[1] << " " << pred[2] << "\n";
        // std::cout << wo.z << " " << abs(bRec.wo.z) << " " << Frame::cosTheta(bRec.wo) << " " << abs(Frame::cosTheta(bRec.wo)) << "\n";
        // return Spectrum(pred) * Frame::cosTheta(bRec.wo);
        return Spectrum(pred);
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &_sample) const override
    {
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        float pdf;
        return sample(bRec, pdf, _sample);
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const override
    {
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);
        float g;
        if (m_flag_isSV)
        {
            int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
            x = math::clamp(x, 0, m_textureWidth - 1);
            y = math::clamp(y, 0, m_textureHeight - 1);

            int texel = m_textureWidth * y + x;
            g = m_svalpha[texel];
        }
        else
        {
            g = m_alpha;
        }
        /* Construct the microfacet distribution matching the
           roughness values at the current surface position. */
        MicrofacetDistribution distr(MicrofacetDistribution::EGGX, g);
        if (m_flag_isBSDF)
        {
            bool sampleReflection = true;
            /* Sample M, the microfacet normal */
            Float microfacetPDF;
            const Normal m = distr.sample(math::signum(Frame::cosTheta(bRec.wi)) * bRec.wi, sample, microfacetPDF);
            if (microfacetPDF == 0)
                return Spectrum(0.0f);
            pdf = microfacetPDF;

            Float cosThetaT;
            Float F = fresnelDielectricExt(dot(bRec.wi, m), cosThetaT, m_eta);

            if (bRec.sampler->next1D() > F)
            {
                sampleReflection = false;
                pdf *= 1 - F;
            }
            else
            {
                pdf *= F;
            }

            Float dwh_dwo;
            if (sampleReflection)
            {
                /* Perfect specular reflection based on the microfacet normal */
                bRec.wo = reflect(bRec.wi, m);
                bRec.eta = 1.0f;
                bRec.sampledComponent = 0;
                bRec.sampledType = EGlossyReflection;

                /* Side check */
                if (Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo) <= 0)
                    return Spectrum(0.0f);

                /* Jacobian of the half-direction mapping */
                dwh_dwo = 1.0f / (4.0f * dot(bRec.wo, m));
            }
            else
            {
                if (cosThetaT == 0)
                    return Spectrum(0.0f);

                /* Perfect specular transmission based on the microfacet normal */
                bRec.wo = refract(bRec.wi, m, m_eta, cosThetaT);
                bRec.eta = cosThetaT < 0 ? m_eta : m_invEta;
                bRec.sampledComponent = 1;
                bRec.sampledType = EGlossyTransmission;

                /* Side check */
                if (Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo) >= 0)
                    return Spectrum(0.0f);

                /* Jacobian of the half-direction mapping */
                Float sqrtDenom = dot(bRec.wi, m) + bRec.eta * dot(bRec.wo, m);
                dwh_dwo = (bRec.eta * bRec.eta * dot(bRec.wo, m)) / (sqrtDenom * sqrtDenom);
            }
            pdf *= std::abs(dwh_dwo);
        }
        else
        {
            /* Sample M, the microfacet normal */
            Normal m = distr.sample(bRec.wi, sample, pdf);

            if (pdf == 0)
                return Spectrum(0.0f);

            /* Perfect specular reflection based on the microfacet normal */
            bRec.wo = reflect(bRec.wi, m);
            bRec.eta = 1.0f;
            bRec.sampledComponent = 0;
            bRec.sampledType = EGlossyReflection;

            /* Side check */
            if (Frame::cosTheta(bRec.wo) <= 0)
                return Spectrum(0.0f);

            /* Jacobian of the half-direction mapping */
            pdf /= 4.0f * dot(bRec.wo, m);
        }
        return eval(bRec) / pdf;
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        if (!m_flag_isBSDF && (Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0))
            return 0.0f;
        float g;
        if (m_flag_isSV)
        {
            int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
            x = math::clamp(x, 0, m_textureWidth - 1);
            y = math::clamp(y, 0, m_textureHeight - 1);

            int texel = m_textureWidth * y + x;
            g = m_svalpha[texel];
        }
        else
        {
            g = m_alpha;
        }
        /* Construct the microfacet distribution matching the
           roughness values at the current surface position. */
        MicrofacetDistribution distr(MicrofacetDistribution::EGGX, g);
        if (m_flag_isBSDF)
        {
            bool reflect = Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo) > 0;
            Vector H;
            Float dwh_dwo;

            if (reflect)
            {
                /* Zero probability if this component was not requested */
                if ((bRec.component != -1 && bRec.component != 0) || !(bRec.typeMask & EGlossyReflection))
                    return 0.0f;

                /* Calculate the reflection half-vector */
                H = normalize(bRec.wo + bRec.wi);

                /* Jacobian of the half-direction mapping */
                dwh_dwo = 1.0f / (4.0f * dot(bRec.wo, H));
            }
            else
            {
                /* Zero probability if this component was not requested */
                if ((bRec.component != -1 && bRec.component != 1) || !(bRec.typeMask & EGlossyTransmission))
                    return 0.0f;

                /* Calculate the transmission half-vector */
                Float eta = Frame::cosTheta(bRec.wi) > 0
                                ? m_eta
                                : m_invEta;

                H = normalize(bRec.wi + bRec.wo * eta);

                /* Jacobian of the half-direction mapping */
                Float sqrtDenom = dot(bRec.wi, H) + eta * dot(bRec.wo, H);
                dwh_dwo = (eta * eta * dot(bRec.wo, H)) / (sqrtDenom * sqrtDenom);
            }

            /* Ensure that the half-vector points into the
               same hemisphere as the macrosurface normal */
            H *= math::signum(Frame::cosTheta(H));
            Float prob = distr.pdf(math::signum(Frame::cosTheta(bRec.wi)) * bRec.wi, H);

            Float F = fresnelDielectricExt(dot(bRec.wi, H), m_eta);
            prob *= reflect ? F : (1 - F);

            return std::abs(prob * dwh_dwo);
        }
        else
        {
            /* Calculate the reflection half-vector */
            Vector H = normalize(bRec.wo + bRec.wi);
            return distr.eval(H) * distr.smithG1(bRec.wi, H) / (4.0f * Frame::cosTheta(bRec.wi));
        }
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
    std::string m_BRDFPredWeightsPath_r, m_BRDFPredWeightsPath_g, m_BRDFPredWeightsPath_b;
    std::string m_BRDFPredWeightsTexturePath_r, m_BRDFPredWeightsTexturePath_g, m_BRDFPredWeightsTexturePath_b;
    std::string m_BRDFDeltaPath_r, m_BRDFDeltaPath_g, m_BRDFDeltaPath_b;
    std::string m_BRDFDeltaTexturePath_r, m_BRDFDeltaTexturePath_g, m_BRDFDeltaTexturePath_b;

    std::string m_BTDFWeightsPath;
    std::string m_BTDFPredWeightsPath_r, m_BTDFPredWeightsPath_g, m_BTDFPredWeightsPath_b;
    std::string m_BTDFPredWeightsTexturePath_r, m_BTDFPredWeightsTexturePath_g, m_BTDFPredWeightsTexturePath_b;
    std::string m_BTDFDeltaPath_r, m_BTDFDeltaPath_g, m_BTDFDeltaPath_b;
    std::string m_BTDFDeltaTexturePath_r, m_BTDFDeltaTexturePath_g, m_BTDFDeltaTexturePath_b;

    std::unique_ptr<BRDFNet> m_brdf_r, m_brdf_g, m_brdf_b;
    std::unique_ptr<BTDFNet> m_btdf_r, m_btdf_g, m_btdf_b;

    std::string m_alphaPath;
    std::string m_alphaTexturePath;

    float m_alpha;
    std::vector<float> m_svalpha;

    float m_eta;
    float m_invEta;

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