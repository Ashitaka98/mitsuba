source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours_rusty_tex5
sampleCount=512
for angle in `seq 0 1 210`; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_kettle/kettle_rusty.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=960 -D Angle=$angle \
        -D paramdir2="1108001/kettle/roughness_0.05_sigmat_2_rusty_texname_5_Al" \
        -D paramstr2="0_0_0_1.49_2_0.7_0.529_0.4_0_0.05_0_0_Al" -D pdfMode=two_lobe -D eta0=1.49  \
        -o /home/lzr/Projects/mitsuba/experiment/video_kettle/output/ours_rusty_tex5/angle_${angle}_${sampleCount}.exr
done