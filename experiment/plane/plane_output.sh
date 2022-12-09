# 生成结果图片
mkdir -p /home/lzr/Projects/mitsuba/experiment/plane/output/

for alpha_1 in 0.1; do
    for sigmat in 2; do
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/plane/plane.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=16 -D width=512 -D height=512 \
            -D paramdir1="1114001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
            -D paramdir2="1205001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
            -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_0.9_0.8_0.6_0_0.1_0_0_1.2" \
            -o /home/lzr/Projects/mitsuba/experiment/plane/output/plane_flip.exr
    done
done
