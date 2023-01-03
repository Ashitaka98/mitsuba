# 生成结果图片

# for alpha_1 in 0.005 0.01 0.02 0.05 0.1 0.2 0.25; do
#     for sigmat in 0 1 2 5; do
#         for color in 0.95 0.9 0.8 0.7 0.6; do
#             mkdir -p /home/lzr/Projects/mitsuba/experiment/plane/output/albedo_${color}_${color}_${color}
#             /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/plane/plane.xml \
#                 -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=256 -D height=256 \
#                 -D paramdir1="1219001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
#                 -D paramdir2="1218001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
#                 -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1" \
#                 -D eta1=1.5 \
#                 -o /home/lzr/Projects/mitsuba/experiment/plane/output/albedo_${color}_${color}_${color}/roughness_${alpha_1}_sigmat_${sigmat}_64.exr
#         done
#     done
# done
for alpha_1 in 0.1; do
    for sigmat in 0 1 2 ; do
        for color in 0; do
            mkdir -p /home/lzr/Projects/mitsuba/experiment/plane/output/albedo_${color}_${color}_${color}_
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/plane/plane.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=1024 -D height=1024 \
                -D paramdir1="1219001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
                -D paramdir2="1218001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
                -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1" \
                -D eta1=1.5 \
                -o /home/lzr/Projects/mitsuba/experiment/plane/output/albedo_${color}_${color}_${color}_/roughness_${alpha_1}_sigmat_${sigmat}_64.exr
        done
    done
done
