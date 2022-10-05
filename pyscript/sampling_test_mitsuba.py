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
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/weights.txt',
                'BTDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/weights.txt',
                'isSV':
                False,
                'isBSDF':
                True,
                'useWhWd':
                True,
                'BRDFDeltaPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_r.txt',
                'BRDFDeltaPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_g.txt',
                'BRDFDeltaPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_b.txt',
                'BTDFDeltaPath_r':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_r.txt',
                'BTDFDeltaPath_g':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_g.txt',
                'BTDFDeltaPath_b':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_b.txt',
            })
        else:
            self.layered = self.pmgr.create({
                'type': 'svneuralbsdf',
                'BRDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/weights.txt',
                'BTDFWeightsPath':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/weights.txt',
                'isSV': True,
                'isBSDF': True,
                'useWhWd': True,
                'BRDFDeltaTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_r.txt',
                'BRDFDeltaTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_g.txt',
                'BRDFDeltaTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/0721001-3/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_b.txt',
                'BTDFDeltaTexturePath_r':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_r.txt',
                'BTDFDeltaTexturePath_g':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_g.txt',
                'BTDFDeltaTexturePath_b':
                '/home/lzr/layeredBsdfData/meta_texture/0809001-epoch133/delta_0.11_0_0_1.08_0_0.89_0.98_0.98_0_0.5_0_0_1.2_b.txt',
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
            # bRec.its.uv.x = (x + 0.5) / self.txwidth
            # bRec.its.uv.y = (y + 0.5) / self.txheight
            bRec.its.uv = Point2((x + 0.5) / self.txwidth,
                                 (y + 0.5) / self.txheight)
            print(bRec.its.uv.x, bRec.its.uv.y)

        ret = self.layered.eval(bRec, EMeasure.ESolidAngle)
        if wo_z != 0:
            ret /= abs(wo_z)
        SVNBRDF_Caller.Log(
            'Sampling theta_i:{:.2f} phi_i:{:.2f} theta_o:{:.2f}, phi_o:{:.2f} -> BSDF: [{:.8f}, {:.8f}, {:.8f}]'
            .format(theta_i, phi_i, theta_o, phi_o, ret[0], ret[1], ret[2]))

        return ret[0], ret[1], ret[2]


if __name__ == '__main__':
    caller = SVNBRDF_Caller(isSV=False)

    caller.eval(1.0, 0.9, 5, 2.5)
    caller.eval(1.0, 0.9, 5, 0.2)

    # caller2 = SVNBRDF_Caller(isSV=True, txwidth=9, txheight=5)

    # caller2.eval(1.0, 0.9, 0.8, 0.7, x=5, y=3)