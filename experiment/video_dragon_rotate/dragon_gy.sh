source /home/lzr/Projects/layeredbsdf/setpath.sh
sampleCount=64
intensity=(15 20 25 30)
for angle in `seq 0 2 160`; do
    for sigmat in 0 1 2 3; do
        mkdir -p /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/output/$sigmat/gy
        /home/lzr/Projects/layeredbsdf/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/dragon_gy.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1280 -D height=720 \
                -D sigmat=${sigmat} -D Angle=$angle -D intensity=${intensity[$sigmat]} \
                -o /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/output/$sigmat/gy/angle_${angle}_${sampleCount}_gy.exr
    done
done