source /home/lzr/Projects/layeredbsdf/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_dragon/output/gy
sampleCount=64
for sigmat in `seq 0 0.02 3`
do
    /home/lzr/Projects/layeredbsdf/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_dragon/dragon_gy.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1280 -D height=720 \
            -D sigmat=${sigmat}  \
            -o /home/lzr/Projects/mitsuba/experiment/video_dragon/output/gy/sigmat_${sigmat}_${sampleCount}_gy.exr
done