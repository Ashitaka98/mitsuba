source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours
sampleCount=512
for angle in `seq 0 1 360`; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_kettle/kettle_brushed.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=960 -D Angle=$angle \
        -D paramdir2="1108001/kettle/roughness_0.005_sigmat_0_brushed_texname_7_Cu" \
        -D paramstr2="0_0_0_1.49_0_0.7_0.529_0.4_0_0.005_0_0_Cu" -D pdfMode=two_lobe -D eta0=1.49  \
        -o /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours/angle_${angle}_${sampleCount}.exr
done