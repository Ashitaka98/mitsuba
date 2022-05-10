from cmath import pi
from mitsuba.core import PluginManager, Properties, Vector3, Spectrum
from mitsuba.render import Intersection, BSDFSamplingRecord, ETransportMode, EMeasure
from math import sin, cos

pmgr = PluginManager.getInstance()
samplerProps = Properties('independent')
samplerProps['sampleCount'] = 128
sampler = pmgr.createObject(samplerProps)
sampler.configure()
bsdf = pmgr.create({
    'type':
    'neuralbsdf',
    'modelpath':
    '/home/lzr/layeredBsdfData/checkpoints/0509001/weights.txt'
})

samplerProps = Properties('independent')
samplerProps['sampleCount'] = 128
sampler = pmgr.createObject(samplerProps)
theta_i = 0.3
phi_i = 0.3
theta_o = 0.5
phi_o = 0.5
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
bRec = BSDFSamplingRecord(its, sampler, ETransportMode.ERadiance)
bRec.wi = wi
bRec.wo = wo
ret = bsdf.eval(bRec, EMeasure.ESolidAngle)
print(ret)