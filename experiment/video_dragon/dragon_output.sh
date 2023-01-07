source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_dragon/output/ours
sampleCount=64
for sigmat in `seq 0.36 0.02 3`
do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_dragon/dragon_ours.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1280 -D height=720 \
        -D paramdir="1108001/dragon/sigmat_${sigmat}" \
        -D paramstr="0.1_0_0_1.5_${sigmat}_0.03_0.6_0.8_0_0.1_0_0_Au" \
        -o /home/lzr/Projects/mitsuba/experiment/video_dragon/output/ours/sigmat_${sigmat}_${sampleCount}.exr
done