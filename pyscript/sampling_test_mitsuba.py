from mitsuba.core import PluginManager, Properties, Vector3, Spectrum, Point2
from mitsuba.render import Intersection, BSDFSamplingRecord, ETransportMode, EMeasure
#import numpy as np
from math import sin, cos


class SVNBRDF_Caller:
    def __init__(self, isSV=False, txwidth=0, txheight=0):
        self.pmgr = PluginManager.getInstance()
        samplerProps = Properties('independent')
        samplerProps['sampleCount'] = 128
        self.sampler = self.pmgr.createObject(samplerProps)
        self.sampler.configure()

        self.isSV = isSV
        self.txwidth = txwidth
        self.txheight = txheight

        if not self.isSV:
            self.layered = self.pmgr.create({
                'type':
                'svneuralbsdf',
                'BRDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/weights',
                'isSV':
                False,
                'isBSDF':
                False,
                'useWhWd':
                True,
                'BRDFDeltaPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/delta_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_r',
                'BRDFDeltaPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/delta_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_g',
                'BRDFDeltaPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/delta_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_b',
                'BRDFPredWeightsPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/predWeights_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_r',
                'BRDFPredWeightsPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/predWeights_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_g',
                'BRDFPredWeightsPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/predWeights_0.3_0_0_1.2_5_0.8_0.8_0.8_0_0.1_0_0_Ag_b',
                'AlphaPath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/sphere_envmap/roughness_0.3_sigmaT_5/alpha.txt'
            })
        else:
            self.layered = self.pmgr.create({
                'type': 'svneuralbsdf',
                'BRDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/weights',
                'isSV': True,
                'isBSDF': False,
                'useWhWd': True,
                'BRDFDeltaTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svdelta_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_r',
                'BRDFDeltaTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svdelta_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_g',
                'BRDFDeltaTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svdelta_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_b',
                'BRDFPredWeightsTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svpredWeights_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_r',
                'BRDFPredWeightsTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svpredWeights_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_g',
                'BRDFPredWeightsTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svpredWeights_0.005_0_0_1.5_-1_0_0_0_0_0.05_0_0_Hg_b',
                'AlphaTexturePath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/globe_lowres/roughness_0.005/svalpha.txt',
                'textureWidth': self.txwidth,
                'textureHeight': self.txheight
            })

    def Log(str):
        print('[SVNBRDF_Caller]: ' + str)

    def eval(self, theta_i, phi_i, theta_o, phi_o, x=0, y=0, debug=False):
        its = Intersection()
        wi_x = cos(theta_i) * sin(phi_i)
        wi_y = sin(theta_i) * sin(phi_i)
        wi_z = cos(phi_i)
        wi = Vector3(wi_x, wi_y, wi_z)
        its.wi = wi

        wo_x = cos(theta_o) * sin(phi_o)
        wo_y = sin(theta_o) * sin(phi_o)
        wo_z = cos(phi_o)
        wo = Vector3(wo_x, wo_y, wo_z)
        bRec = BSDFSamplingRecord(its, self.sampler, ETransportMode.ERadiance)
        bRec.wi = wi
        bRec.wo = wo
        if self.isSV:
            print(x, y, self.txwidth, (x + 0.5) / self.txwidth)
            print(bRec.its.uv)
            # bRec.its.uv.x = (x + 0.5) / self.txwidth
            # bRec.its.uv.y = (y + 0.5) / self.txheight
            bRec.its.uv = Point2((x + 0.5) / self.txwidth,
                                 (y + 0.5) / self.txheight)
            print(bRec.its.uv)

        ret = self.layered.eval(bRec, EMeasure.ESolidAngle)
        SVNBRDF_Caller.Log(
            'Sampling theta_i:{:.2f} phi_i:{:.2f} theta_o:{:.2f}, phi_o:{:.2f} -> BSDF: [{:.8f}, {:.8f}, {:.8f}]'
            .format(theta_i, phi_i, theta_o, phi_o, ret[0], ret[1], ret[2]))

        return ret[0], ret[1], ret[2]


if __name__ == '__main__':
    caller = SVNBRDF_Caller(isSV=False)

    # caller.eval(0, 19, 0.8, 0.8)

    caller2 = SVNBRDF_Caller(isSV=True, txwidth=256, txheight=128)

    caller2.eval(1.0, 0.9, 0.8, 0.7, x=5, y=3)