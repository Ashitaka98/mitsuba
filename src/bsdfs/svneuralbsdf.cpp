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

enum EpdfType
{
    /// two lobe ggx
    ETwolobe = 0,

    /// single lobe ggx
    ESingleGGX = 1,

    /// diffuse
    EDiffuse = 2
};
class SVNeuralBSDF : public BSDF
{
    using BRDFNet = BSDFNet<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;
    using BRDFNet_SharedWeights = SharedWeights<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;
    using BTDFNet = BSDFNet<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;
    using BTDFNet_SharedWeights = SharedWeights<INPUT_DIM, 32, 0, 0, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 64, 31, 32, 1, 32, 1, 32, 0>;

public:
    SVNeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "############# SVNeuralBSDF #############");
        sh_encode = SH_Encode::getInstance();
        m_BRDFWeightsPath = props.getString("BRDFWeightsPath");
        m_flag_isSV = props.getBoolean("isSV", false);
        m_flag_isBSDF = props.getBoolean("isBSDF", false);
        m_flag_useWhWd = props.getBoolean("useWhWd", false);
        std::string pdfMode = props.getString("pdfMode", "two_lobe");
        if (pdfMode == "two_lobe")
        {
            m_pdfMode = EpdfType::ETwolobe;
        }
        else if (pdfMode == "single_lobe")
        {
            m_pdfMode = EpdfType::ESingleGGX;
        }
        else if (pdfMode == "diffuse")
        {
            m_pdfMode = EpdfType::EDiffuse;
        }
        else
        {
            Log(EError, "illegal pdfMode, only two_lobe, single_lobe, diffuse is supported");
        }
        if (m_flag_isSV)
        {
            if (m_pdfMode != EpdfType::EDiffuse)
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
            if (m_pdfMode != EpdfType::EDiffuse)
                m_alphaPath = props.getString("AlphaPath");
            m_BRDFDeltaPath_r = props.getString("BRDFDeltaPath_r");
            m_BRDFDeltaPath_g = props.getString("BRDFDeltaPath_g");
            m_BRDFDeltaPath_b = props.getString("BRDFDeltaPath_b");
            m_BRDFPredWeightsPath_r = props.getString("BRDFPredWeightsPath_r");
            m_BRDFPredWeightsPath_g = props.getString("BRDFPredWeightsPath_g");
            m_BRDFPredWeightsPath_b = props.getString("BRDFPredWeightsPath_b");
        }
        m_eta1 = props.getFloat("eta1");
        Log(EInfo, "eta1:%f", m_eta1);
        if (m_flag_isBSDF)
        {

            m_BTDFWeightsPath = props.getString("BTDFWeightsPath");
            m_eta2 = props.getFloat("eta2");
            Log(EInfo, "eta2:%f", m_eta2);
            // m_invEta = 1 / m_eta;
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
        auto start = time(nullptr);
        m_components.push_back(EGlossyReflection | EFrontSide);
        if (m_flag_isBSDF)
        {
            m_components.push_back(EGlossyTransmission | EBackSide);
        }

        {
            float BRDFWeights[BRDFNet_SharedWeights::num_weights];
            FILE *model_file = fopen(m_BRDFWeightsPath.c_str(), "r");
            assert(model_file);
            Log(EInfo, "BRDFNet weights number: %d", BRDFNet_SharedWeights::num_weights);
            assert(fread(BRDFWeights, sizeof(float), BRDFNet_SharedWeights::num_weights, model_file) == BRDFNet_SharedWeights::num_weights);
            auto p = aligned_alloc(64, sizeof(BRDFNet_SharedWeights));
            p_brdf_shared = std::unique_ptr<BRDFNet_SharedWeights>(new (p) BRDFNet_SharedWeights(BRDFWeights));
            fclose(model_file);
        }

        if (m_flag_isBSDF)
        {
            float BTDFWeights[BTDFNet_SharedWeights::num_weights];
            FILE *model_file = fopen(m_BTDFWeightsPath.c_str(), "r");
            assert(model_file);
            Log(EInfo, "BTDFNet weights number: %d", BTDFNet_SharedWeights::num_weights);
            assert(fread(BTDFWeights, sizeof(float), BTDFNet_SharedWeights::num_weights, model_file) == BTDFNet_SharedWeights::num_weights);
            auto p = aligned_alloc(64, sizeof(BTDFNet_SharedWeights));
            p_btdf_shared = std::unique_ptr<BTDFNet_SharedWeights>(new (p) BTDFNet_SharedWeights(BTDFWeights));
            fclose(model_file);
        }

        if (m_flag_isSV)
        {
            m_svbrdf_r.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_g.resize(m_textureWidth * m_textureHeight);
            m_svbrdf_b.resize(m_textureWidth * m_textureHeight);
            m_svalpha1.resize(m_textureWidth * m_textureHeight);
            m_svalpha2.resize(m_textureWidth * m_textureHeight);
            if (m_flag_isBSDF)
            {
                m_svbtdf_r.resize(m_textureWidth * m_textureHeight);
                m_svbtdf_g.resize(m_textureWidth * m_textureHeight);
                m_svbtdf_b.resize(m_textureWidth * m_textureHeight);
            }
            Log(EInfo, "BRDFNet injected units number: %d", BRDFNet::num_injected);
            Log(EInfo, "BRDFNet injected weights number: %d", BRDFNet::num_injected_weights);
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];
                FILE *injectedUnits_file = fopen(m_BRDFDeltaTexturePath_r.c_str(), "r");
                assert(injectedUnits_file);
                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_r.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);
                        assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);
                        auto p = new BRDFNet(injected_units, injected_weights);
                        p->set_shared(p_brdf_shared.get());
                        m_svbrdf_r[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];
                FILE *injectedUnits_file = fopen(m_BRDFDeltaTexturePath_g.c_str(), "r");
                assert(injectedUnits_file);
                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_g.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);
                        assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);
                        auto p = new BRDFNet(injected_units, injected_weights);
                        p->set_shared(p_brdf_shared.get());
                        m_svbrdf_g[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];
                FILE *injectedUnits_file = fopen(m_BRDFDeltaTexturePath_b.c_str(), "r");
                assert(injectedUnits_file);
                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsTexturePath_b.c_str(), "r");
                assert(injectedWeights_file);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);
                        assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);
                        auto p = new BRDFNet(injected_units, injected_weights);
                        p->set_shared(p_brdf_shared.get());
                        m_svbrdf_b[i * m_textureWidth + j] = std::unique_ptr<BRDFNet>(p);
                    }
                }
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            if (m_flag_isBSDF)
            {
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                Log(EInfo, "BTDFNet injected weights number: %d", BTDFNet::num_injected_weights);
                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];
                    FILE *injectedUnits_file = fopen(m_BTDFDeltaTexturePath_r.c_str(), "r");
                    assert(injectedUnits_file);
                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_r.c_str(), "r");
                    assert(injectedWeights_file);
                    for (int i = 0; i < m_textureHeight; i++)
                    {
                        for (int j = 0; j < m_textureWidth; j++)
                        {
                            assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);
                            assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);
                            auto p = new BTDFNet(injected_units, injected_weights);
                            p->set_shared(p_btdf_shared.get());
                            m_svbtdf_r[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p);
                        }
                    }
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }

                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];
                    FILE *injectedUnits_file = fopen(m_BTDFDeltaTexturePath_g.c_str(), "r");
                    assert(injectedUnits_file);
                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_g.c_str(), "r");
                    assert(injectedWeights_file);
                    for (int i = 0; i < m_textureHeight; i++)
                    {
                        for (int j = 0; j < m_textureWidth; j++)
                        {
                            assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);
                            assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);
                            auto p = new BTDFNet(injected_units, injected_weights);
                            p->set_shared(p_btdf_shared.get());
                            m_svbtdf_g[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p);
                        }
                    }
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }

                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];
                    FILE *injectedUnits_file = fopen(m_BTDFDeltaTexturePath_b.c_str(), "r");
                    assert(injectedUnits_file);
                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsTexturePath_b.c_str(), "r");
                    assert(injectedWeights_file);
                    for (int i = 0; i < m_textureHeight; i++)
                    {
                        for (int j = 0; j < m_textureWidth; j++)
                        {
                            assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);
                            assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);
                            auto p = new BTDFNet(injected_units, injected_weights);
                            p->set_shared(p_btdf_shared.get());
                            m_svbtdf_b[i * m_textureWidth + j] = std::unique_ptr<BTDFNet>(p);
                        }
                    }
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }
            }
            if (m_pdfMode != EpdfType::EDiffuse)
            {
                FILE *svAlphaFile = fopen(m_alphaTexturePath.c_str(), "r");
                assert(svAlphaFile);
                for (int i = 0; i < m_textureHeight; i++)
                {
                    for (int j = 0; j < m_textureWidth; j++)
                    {
                        float alpha1, alpha2;
                        assert(fscanf(svAlphaFile, "%f", &alpha1) == 1);
                        m_svalpha1[i * m_textureWidth + j] = alpha1;
                        if (m_pdfMode == EpdfType::ETwolobe)
                        {
                            assert(fscanf(svAlphaFile, "%f", &alpha2) == 1);
                            m_svalpha2[i * m_textureWidth + j] = alpha2;
                        }
                    }
                }
                fclose(svAlphaFile);
            }
        }
        else
        {
            Log(EInfo, "BRDFNet injected units number: %d", BRDFNet::num_injected);
            Log(EInfo, "BRDFNet injected weights number: %d", BRDFNet::num_injected_weights);
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];

                FILE *injectedUnits_file = fopen(m_BRDFDeltaPath_r.c_str(), "r");
                assert(injectedUnits_file);
                assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);

                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsPath_r.c_str(), "r");
                assert(injectedWeights_file);
                assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);

                auto p = new BRDFNet(injected_units, injected_weights);
                p->set_shared(p_brdf_shared.get());
                m_brdf_r = std::unique_ptr<BRDFNet>(p);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];

                FILE *injectedUnits_file = fopen(m_BRDFDeltaPath_g.c_str(), "r");
                assert(injectedUnits_file);
                assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);

                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsPath_g.c_str(), "r");
                assert(injectedWeights_file);
                assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);

                auto p = new BRDFNet(injected_units, injected_weights);
                p->set_shared(p_brdf_shared.get());
                m_brdf_g = std::unique_ptr<BRDFNet>(p);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            {
                float injected_units[BRDFNet::num_injected];
                float injected_weights[BRDFNet::num_injected_weights];

                FILE *injectedUnits_file = fopen(m_BRDFDeltaPath_b.c_str(), "r");
                assert(injectedUnits_file);
                assert(fread(injected_units, sizeof(float), BRDFNet::num_injected, injectedUnits_file) == BRDFNet::num_injected);

                FILE *injectedWeights_file = fopen(m_BRDFPredWeightsPath_b.c_str(), "r");
                assert(injectedWeights_file);
                assert(fread(injected_weights, sizeof(float), BRDFNet::num_injected_weights, injectedWeights_file) == BRDFNet::num_injected_weights);

                auto p = new BRDFNet(injected_units, injected_weights);
                p->set_shared(p_brdf_shared.get());
                m_brdf_b = std::unique_ptr<BRDFNet>(p);
                fclose(injectedUnits_file);
                fclose(injectedWeights_file);
            }
            if (m_flag_isBSDF)
            {
                Log(EInfo, "BTDFNet injected units number: %d", BTDFNet::num_injected);
                Log(EInfo, "BTDFNet injected weights number: %d", BTDFNet::num_injected_weights);
                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];

                    FILE *injectedUnits_file = fopen(m_BTDFDeltaPath_r.c_str(), "r");
                    assert(injectedUnits_file);
                    assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);

                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsPath_r.c_str(), "r");
                    assert(injectedWeights_file);
                    assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);

                    auto p = new BTDFNet(injected_units, injected_weights);
                    p->set_shared(p_btdf_shared.get());
                    m_btdf_r = std::unique_ptr<BTDFNet>(p);
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }
                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];

                    FILE *injectedUnits_file = fopen(m_BTDFDeltaPath_g.c_str(), "r");
                    assert(injectedUnits_file);
                    assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);

                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsPath_g.c_str(), "r");
                    assert(injectedWeights_file);
                    assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);

                    auto p = new BTDFNet(injected_units, injected_weights);
                    p->set_shared(p_btdf_shared.get());
                    m_btdf_g = std::unique_ptr<BTDFNet>(p);
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }
                {
                    float injected_units[BTDFNet::num_injected];
                    float injected_weights[BTDFNet::num_injected_weights];

                    FILE *injectedUnits_file = fopen(m_BTDFDeltaPath_b.c_str(), "r");
                    assert(injectedUnits_file);
                    assert(fread(injected_units, sizeof(float), BTDFNet::num_injected, injectedUnits_file) == BTDFNet::num_injected);

                    FILE *injectedWeights_file = fopen(m_BTDFPredWeightsPath_b.c_str(), "r");
                    assert(injectedWeights_file);
                    assert(fread(injected_weights, sizeof(float), BTDFNet::num_injected_weights, injectedWeights_file) == BTDFNet::num_injected_weights);

                    auto p = new BTDFNet(injected_units, injected_weights);
                    p->set_shared(p_btdf_shared.get());
                    m_btdf_b = std::unique_ptr<BTDFNet>(p);
                    fclose(injectedUnits_file);
                    fclose(injectedWeights_file);
                }
            }
            if (m_pdfMode != EpdfType::EDiffuse)
            {
                FILE *alphaFile = fopen(m_alphaPath.c_str(), "r");
                assert(alphaFile);
                assert(fscanf(alphaFile, "%f", &m_alpha1) == 1);
                Log(EInfo, "m_alpha1:%f", m_alpha1);
                if (m_pdfMode == EpdfType::ETwolobe)
                {
                    assert(fscanf(alphaFile, "%f", &m_alpha2) == 1);
                    Log(EInfo, "m_alpha2:%f", m_alpha2);
                }
                fclose(alphaFile);
            }
        }

        Log(EInfo, "SVNeuralBSDF configured");
        BSDF::configure();
        auto end = time(nullptr);
        std::cout << "configure costs " << difftime(end, start) << "s\n";
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
            Log(EError, "wi.z<0 not implemented");
            // if (bRec.wo.z > 0)
            // {
            //     // reciprocity
            //     wi = bRec.wo;
            //     wo = bRec.wi;
            // }
            // else
            // {
            //     wi = -bRec.wi;
            //     wo = -bRec.wo;
            // }
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
        assert(bRec.sampler);
        if (!m_flag_isBSDF && Frame::cosTheta(bRec.wi) <= 0)
            return Spectrum(0.0);

        if (m_pdfMode == EpdfType::EDiffuse)
        {
            bRec.wo = warp::squareToCosineHemisphere(sample);
            pdf = warp::squareToCosineHemispherePdf(bRec.wo);
            return eval(bRec) / pdf;
        }
        else if (m_pdfMode == EpdfType::ESingleGGX)
        {
            float g;
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
                y = math::clamp(y, 0, m_textureHeight - 1);

                int texel = m_textureWidth * y + x;
                g = m_svalpha1[texel];
            }
            else
            {
                g = m_alpha1;
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
        else
        {

            float g1, g2;
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
                y = math::clamp(y, 0, m_textureHeight - 1);

                int texel = m_textureWidth * y + x;
                g1 = m_svalpha1[texel];
                g2 = m_svalpha2[texel];
            }
            else
            {
                g1 = m_alpha1;
                g2 = m_alpha2;
            }
            /* Construct the microfacet distribution matching the
               roughness values at the current surface position. */
            MicrofacetDistribution distr1(MicrofacetDistribution::EGGX, g1);
            MicrofacetDistribution distr2(MicrofacetDistribution::EGGX, g2);
            MicrofacetDistribution *distr = &distr1;
            pdf = 1;
            if (m_flag_isBSDF)
            {
                Log(EError, "bsdf sample not implemented");
                bool sampleReflection = true;
                /* Sample M, the microfacet normal */
                Float microfacetPDF;
                const Normal m = distr->sample(math::signum(Frame::cosTheta(bRec.wi)) * bRec.wi, sample, microfacetPDF);
                if (microfacetPDF == 0)
                    return Spectrum(0.0f);
                pdf = microfacetPDF;

                Float cosThetaT;
                Float F = fresnelDielectricExt(dot(bRec.wi, m), cosThetaT, m_eta1);

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
            else // BRDF (two lobe)
            {
                // decide which lobe to sample
                Float cosThetaT;
                Float F = fresnelDielectricExt(Frame::cosTheta(bRec.wi), cosThetaT, m_eta1);
                if (bRec.sampler->next1D() > F)
                {
                    // sample distr2
                    distr = &distr2;
                }
                else
                {
                    // sample distr1
                    distr = &distr1;
                }
                /* Sample M, the microfacet normal */
                Float microfacetPDF;
                Normal m = distr->sample(bRec.wi, sample, microfacetPDF);
                if (microfacetPDF == 0)
                    return Spectrum(0.0f);
                pdf *= microfacetPDF;

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
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const override
    {
        assert(bRec.sampler);
        if (!m_flag_isBSDF && (Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0))
            return 0.0f;
        if (m_pdfMode == EpdfType::EDiffuse)
        {
            return warp::squareToCosineHemispherePdf(bRec.wo);
        }
        else if (m_pdfMode == EpdfType::ESingleGGX)
        {
            float g;
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
                y = math::clamp(y, 0, m_textureHeight - 1);

                int texel = m_textureWidth * y + x;
                g = m_svalpha1[texel];
            }
            else
            {
                g = m_alpha1;
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
        else
        {
            float g1, g2;
            if (m_flag_isSV)
            {
                int x = math::roundToInt(bRec.its.uv.x * m_textureWidth - 0.5), y = math::roundToInt(bRec.its.uv.y * m_textureHeight - 0.5);
                x = math::clamp(x, 0, m_textureWidth - 1);
                y = math::clamp(y, 0, m_textureHeight - 1);

                int texel = m_textureWidth * y + x;
                g1 = m_svalpha1[texel];
                g2 = m_svalpha2[texel];
            }
            else
            {
                g1 = m_alpha1;
                g2 = m_alpha2;
            }
            /* Construct the microfacet distribution matching the
               roughness values at the current surface position. */
            MicrofacetDistribution distr1(MicrofacetDistribution::EGGX, g1);
            MicrofacetDistribution distr2(MicrofacetDistribution::EGGX, g2);
            MicrofacetDistribution *distr = nullptr;
            if (m_flag_isBSDF)
            {

                Log(EError, "bsdf sample not implemented");
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
                Float prob = distr->pdf(math::signum(Frame::cosTheta(bRec.wi)) * bRec.wi, H);

                Float F = fresnelDielectricExt(dot(bRec.wi, H), m_eta);
                prob *= reflect ? F : (1 - F);

                return std::abs(prob * dwh_dwo);
            }
            else
            {
                float pdf, pdf1, pdf2;
                Float cosThetaT;
                Float F = fresnelDielectricExt(Frame::cosTheta(bRec.wi), cosThetaT, m_eta1);
                /* Calculate the reflection half-vector */
                Vector H = normalize(bRec.wo + bRec.wi);
                pdf1 = distr1.eval(H) * distr1.smithG1(bRec.wi, H) / (4.0f * Frame::cosTheta(bRec.wi));
                pdf2 = distr2.eval(H) * distr2.smithG1(bRec.wi, H) / (4.0f * Frame::cosTheta(bRec.wi));
                pdf = (1 - F) * pdf2 + F * pdf1;
                return pdf;
            }
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

    float m_alpha1, m_alpha2;
    std::vector<float> m_svalpha1;
    std::vector<float> m_svalpha2;

    EpdfType m_pdfMode = EpdfType::ETwolobe;

    float m_eta1, m_eta2, m_eta;
    float m_invEta;

    std::vector<std::unique_ptr<BRDFNet>> m_svbrdf_r, m_svbrdf_g, m_svbrdf_b;
    std::vector<std::unique_ptr<BTDFNet>> m_svbtdf_r, m_svbtdf_g, m_svbtdf_b;
    bool m_flag_isSV;
    bool m_flag_isBSDF;
    bool m_flag_useWhWd;
    SH_Encode *sh_encode;
    int m_textureWidth, m_textureHeight;

    std::unique_ptr<BRDFNet_SharedWeights> p_brdf_shared;
    std::unique_ptr<BTDFNet_SharedWeights> p_btdf_shared;
};

MTS_IMPLEMENT_CLASS_S(SVNeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(SVNeuralBSDF, "Spatial Variant Neural Network Based BSDF");
MTS_NAMESPACE_END