source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_shoe_100/output/ours
sampleCount=1024
for angle in `seq 0 2 360`; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_shoe_100/shoe_ours.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=640 -D height=640 -D angle=$angle \
        -D paramdir="1202001/shoe/roughness_0.1_sigmat_4" \
        -D paramstr="0.1_0_0_1.9_4_0.9_0.2_0.1_0_0.04_0_0_Ag" \
        -o /home/lzr/Projects/mitsuba/experiment/video_shoe_100/output/ours/angle_${angle}_${sampleCount}.exr
done
