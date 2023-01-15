#! /bin/bash
source /home/lzr/Projects/layeredbsdf/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_plane/output/gy_sigmat_new
sampleCount=64           
for alpha_1 in 0.02; do
    for sigmat in `seq 0 0.01 2`; do
        /home/lzr/Projects/layeredbsdf/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_plane/plane_gy.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=720 -D height=720 \
            -D eta_0=1.5 -D alpha_0=${alpha_1} -D sigmaT_0=${sigmat} -D albedo_0="0 0 0" \
            -o /home/lzr/Projects/mitsuba/experiment/video_plane/output/gy_sigmat_new/roughness_${alpha_1}_sigmat_${sigmat}_${sampleCount}_gy.exr
    done
done