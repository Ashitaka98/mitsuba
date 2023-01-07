source /home/lzr/Projects/mitsuba/setpath.sh
mkdir -p /home/lzr/Projects/mitsuba/experiment/video_globe/output/ours
sampleCount=434
for alpha_1 in 0.005; do
    for sigmat in 3; do
        for angle in `seq 0 1 360`; do
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_globe/globe.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=1024 -D height=1024 -D angle=$angle \
                -D paramdir="1123001/globe/roughness_${alpha_1}" \
                -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_0_0_0_0_0.05_0_0_Hg" \
                -o /home/lzr/Projects/mitsuba/experiment/video_globe/output/ours/angle_${angle}_${sampleCount}.exr
        done
    done
done
