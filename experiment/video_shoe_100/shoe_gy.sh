source /home/lzr/Projects/layeredbsdf/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_shoe_100/output/gy
sampleCount=128
for angle in `seq 0 2 360`; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_shoe_100/shoe_gy.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=640 -D height=640 -D angle=$angle \
        -o /home/lzr/Projects/mitsuba/experiment/video_shoe_100/output/gy/angle_${angle}_${sampleCount}_gy.exr
done
