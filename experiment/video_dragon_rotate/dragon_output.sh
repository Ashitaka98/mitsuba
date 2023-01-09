source /home/lzr/Projects/mitsuba/setpath.sh
sampleCount=64
intensity=(15 20 25 30)
for angle in `seq 0 2 160`; do
    for sigmat in 0 1 2 3; do
        mkdir -p /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/output/$sigmat/ours_new
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/dragon_ours.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1280 -D height=720 -D Angle=$angle -D intensity=${intensity[$sigmat]} \
            -D paramdir="1108001/dragon/sigmat_${sigmat}" \
            -D paramstr="0.05_0_0_1.5_${sigmat}_0.03_0.6_0.8_0_0.05_0_0_Au" \
            -o /home/lzr/Projects/mitsuba/experiment/video_dragon_rotate/output/$sigmat/ours_new/angle_${angle}_${sampleCount}.exr
    done
done