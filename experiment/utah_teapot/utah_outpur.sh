# 生成结果图片
mkdir -p /home/lzr/Projects/mitsuba/experiment/utah_teapot/output/fov30_640/

for alpha_0 in 0.1
do
    for alpha_1 in 0.1
    do
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/utah_teapot/utah_teapot.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=640 -D height=360 \
            -D paramdir="1108001/utah_teapot/alpha0_${alpha_0}_alpha1_${alpha_1}" \
            -D paramstr="${alpha_0}_0_0_1.5_0_0.9_0.8_0.6_0_${alpha_1}_0_0_Hg" \
            -D pdfMode=two_lobe -D eta_0=1.5 \
            -o /home/lzr/Projects/mitsuba/experiment/utah_teapot/output/fov30_640/alpha0_${alpha_0}_alpha1_${alpha_1}_64.exr
    done
done
