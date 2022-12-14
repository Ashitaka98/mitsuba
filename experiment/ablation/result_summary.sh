
for matname in `cat /home/lzr/Projects/mitsuba/experiment/ablation/material_set_selected.txt`
do
    for model in 1108001 1209001 1209002
    do
        cp /home/lzr/Projects/mitsuba/experiment/ablation/$model/sphere/$matname/${matname}_twolobe.exr /home/lzr/Projects/mitsuba/experiment/ablation/selected/$model/${matname}_$model.exr
    done
    cp /home/lzr/Projects/mitsuba/experiment/ablation/gy/sphere/$matname/${matname}_gy.exr /home/lzr/Projects/mitsuba/experiment/ablation/selected/gy/${matname}_gy.exr
done