source /home/lzr/Projects/mitsuba/setpath.sh
sampleCount=512
for alpha0 in 0.05; do
    mkdir -p /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours_homo/$alpha0
    for angle in `seq 0 1 360`; do
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_kettle/kettle_homo.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=960 -D Angle=$angle \
            -D paramdir2="1108001/kettle/roughness_$alpha0" \
            -D paramstr2="${alpha0}_0_0_1.49_0_0.7_0.529_0.4_0_0.005_0_0_Cu" -D pdfMode=two_lobe -D eta0=1.49  \
            -o /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours_homo/$alpha0/angle_${angle}_${sampleCount}.exr
    done
done