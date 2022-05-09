from mitsuba.core import PluginManager, Properties

pmgr = PluginManager.getInstance()
samplerProps = Properties('independent')
samplerProps['sampleCount'] = 128
sampler = pmgr.createObject(samplerProps)
sampler.configure()
bsdf = pmgr.create({'type': 'svneuralbsdf'})