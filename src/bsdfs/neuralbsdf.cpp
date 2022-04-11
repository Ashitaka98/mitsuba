#include <mitsuba/core/warp.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/fstream.h>
MTS_NAMESPACE_BEGIN

class NeuralBSDF : public BSDF
{
public:
    NeuralBSDF(const Properties &props) : BSDF(props)
    {
        Log(EInfo, "NeuralBSDF constructed");
    }
    NeuralBSDF(Stream *stream, InstanceManager *manager) : BSDF(stream, manager)
    {
        Log(EError, " Not implemented.");
    }
    void serialize(Stream *stream, InstanceManager *manager) const
    {
        Log(EError, "Not implemented.");
    }
    void configure()
    {
    }
    Spectrum eval(const BSDFSamplingRecord &bRec,
                  EMeasure measure = ESolidAngle) const
    {
        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const
    {
        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf,
                    const Point2 &sample) const
    {
        Spectrum ret;
        ret[0] = 1;
        return ret;
    }
    Float pdf(const BSDFSamplingRecord &bRec,
              EMeasure measure = ESolidAngle) const
    {
    }
    Spectrum getDiffuseReflectance(const Intersection &its) const
    {
        Log(EError, "Not implemented.");
        return Spectrum(0.0);
    }

    Spectrum getSpecularReflectance(const Intersection &its) const
    {
        Log(EError, "Not implemented.");
        return Spectrum(0.0);
    }

    MTS_DECLARE_CLASS()
};

MTS_IMPLEMENT_CLASS_S(NeuralBSDF, false, BSDF);
MTS_EXPORT_PLUGIN(NeuralBSDF, "Neural Network based BSDF");
MTS_NAMESPACE_END