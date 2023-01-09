source /home/lzr/Projects/layeredbsdf/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_shoe_2/output/gy
sampleCount=1
for angle in `seq 0 2 360`; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_shoe_2/shoe_gy.xml \
        -D maxDepth=2 -D rrDepth=2 -D sampleCount=$sampleCount -D width=640 -D height=640 -D angle=$angle \
        -o /home/lzr/Projects/mitsuba/experiment/video_shoe_2/output/gy/angle_${angle}_${sampleCount}_gy.exr
done
