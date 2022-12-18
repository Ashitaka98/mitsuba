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
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/weights',
                'isSV':
                False,
                'isBSDF':
                False,
                'useWhWd':
                True,
                'eta1':
                1.5,
                'BRDFDeltaPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/delta_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_r',
                'BRDFDeltaPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/delta_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_g',
                'BRDFDeltaPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/delta_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_b',
                'BRDFPredWeightsPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/predWeights_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_r',
                'BRDFPredWeightsPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/predWeights_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_g',
                'BRDFPredWeightsPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/predWeights_0.01_0_0_1.5_0_0.1_0.7_0.5_0_0.1_0_0_Cu_b',
                'AlphaPath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/teapot/roughness_0.01_sigmat_0/alpha.txt'
            })
        else:
            self.layered = self.pmgr.create({
                'type': 'svneuralbsdf',
                'BRDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/weights',
                'isSV': True,
                'isBSDF': False,
                'useWhWd': True,
                'eta1': 1.9,
                'BRDFDeltaTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svdelta_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_r',
                'BRDFDeltaTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svdelta_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_g',
                'BRDFDeltaTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svdelta_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_b',
                'BRDFPredWeightsTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svpredWeights_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_r',
                'BRDFPredWeightsTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svpredWeights_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_g',
                'BRDFPredWeightsTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svpredWeights_0.2_0_0_1.9_2_1_1_1_0_0.04_0_0_Ag_b',
                'AlphaTexturePath':
                '/home/lzr/layeredBsdfData/meta_texture/1108001/shoe/roughness_0.2_sigmat_2/svalpha.txt',
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
    # caller = SVNBRDF_Caller(isSV=False)

    # caller.eval(0, 1, 0.6, 0.8)

    caller2 = SVNBRDF_Caller(isSV=True, txwidth=780, txheight=521)

    caller2.eval(1.0, 0.9, 0.8, 0.7, x=5, y=3)