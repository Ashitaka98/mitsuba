# 生成结果图片
mkdir -p /home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/output/1108001/

/home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/teaser.xml \
    -D maxDepth=100 -D rrDepth=20 -D sampleCount=256 -D width=960 -D height=540 \
    -D paramdir="1108001/teaser" \
    -D paramstr1="0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Na_palik" \
    -D paramstr2="0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Al" \
    -D paramstr3="0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Hg" \
    -D paramstr4="0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Ag" \
    -D paramstr5="0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Ag" \
    -D paramstrTable="0.1_0_0_1.5_1_1_1_1_0_0.1_0_0_Ag" \
    -o /home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/output/1108001/teaser_ours.exr
  
