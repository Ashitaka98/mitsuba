# 生成结果图片
mkdir -p /home/lzr/Projects/mitsuba/experiment/is/output/layered/

for alpha_1 in 0.1
do
    for sigmat in 2
    do
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/is/matpreview/matpreview.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=512 -D height=512 \
            -D paramdir="1108001/matpreview/roughness_${alpha_1}_sigmat_${sigmat}" \
            -D paramstr="${alpha_1}_0_0_1.9_${sigmat}_0.6_0.4_0.8_0_0.1_0_0_Ag" \
            -o /home/lzr/Projects/mitsuba/experiment/is/output/layered/twolobe.exr
    done
done
