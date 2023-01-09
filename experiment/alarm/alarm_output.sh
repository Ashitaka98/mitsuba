source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/alarm/output/
sampleCount=64
for sigmat in 1; do
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/alarm/alarm.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1024 -D height=1024 \
        -D paramdir="1108001/alarm/sigmat_${sigmat}" \
        -D paramstr="-1_0_0_1.5_${sigmat}_1_1_1_0_0.05_0_0_Hg" \
        -o /home/lzr/Projects/mitsuba/experiment/alarm/output/sigmat_${sigmat}_${sampleCount}.exr
done
