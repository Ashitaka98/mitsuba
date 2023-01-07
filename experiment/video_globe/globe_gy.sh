source /home/lzr/Projects/layeredbsdf/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_globe/output/gy
sampleCount=64
for alpha_0 in 0.005; do
    for sigmaT in 3; do
        for angle in `seq 0 1 360`; do
        /home/lzr/Projects/layeredbsdf/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_globe/globe_gy.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1024 -D height=1024 -D angle=$angle \
            -D eta_0=1.5 -D alpha_0=${alpha_0} -D sigmaT_0=${sigmaT} -D albedo_0="0 0 0" -D alpha_1=0.05 -D material=Hg \
            -o /home/lzr/Projects/mitsuba/experiment/video_globe/output/gy/angle_${angle}_${sampleCount}_gy.exr
        done
    done
done
