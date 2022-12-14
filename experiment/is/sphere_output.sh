# 生成结果图片
mkdir -p /home/lzr/Projects/mitsuba/experiment/is/output/sphere/

for alpha_0 in 0.02
do
    for alpha_1 in 0.05
    do
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/is/sphere/sphere.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=512 -D height=512 \
            -D paramdir="1108001/sphere/alpha0_${alpha_0}_alpha1_${alpha_1}" \
            -D paramstr="${alpha_0}_0_0_1.9_0_0.6_0.4_0.8_0_${alpha_1}_0_0_Ag" \
            -D pdfMode=single_lobe -D eta_0=1.9 \
            -o /home/lzr/Projects/mitsuba/experiment/is/output/sphere/alpha0_${alpha_0}_alpha1_${alpha_1}_onelobe.exr
    done
done
