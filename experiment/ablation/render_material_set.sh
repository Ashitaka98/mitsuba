#! /bin/bash
source ~/.bashrc
source activate

function run_case(){
    echo model is $model
    conda activate layeredModel
    echo 'prepare meta texture'
    for matname in `cat /home/lzr/Projects/mitsuba/experiment/ablation/material_set.txt`
    do
        path="/home/lzr/layeredBsdfData/meta_texture/$model/ablation/$matname"
        if [ ! -d "$path" ]; then
            mkdir -p $path; 
            python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=$GPU \
                --model_dir=/home/lzr/layeredBsdfModel/checkpoints/$model \
                --out_dir=$path\
                --sh_dim=81 \
                --surface_type=conductor --is_sv=0 --filename=$matname
        fi
    done

    echo 'prepare done'

    echo 'rendering'
    conda activate layeredBsdf
    source /home/lzr/Projects/mitsuba/setpath.sh

    for matname in `cat /home/lzr/Projects/mitsuba/experiment/ablation/material_set.txt`
    do
        path="/home/lzr/Projects/mitsuba/experiment/ablation/$model/sphere/$matname"
        
        if [ ! -d "$path" ]; then
            echo $matname
            arr=(${matname//_/ })
            eta0=${arr[3]}
            echo eta0 is $eta0
            mkdir -p $path; 
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/ablation/sphere/sphere.xml \
                -D maxDepth=$maxDepth -D rrDepth=$rrDepth -D sampleCount=$sampleCount -D width=$width -D height=$height \
                -D paramdir="$model/ablation/$matname" \
                -D paramstr="$matname" \
                -D pdfMode=two_lobe  -D eta_0=$eta0 \
                -o $path/${matname}_twolobe.exr
        fi
    done
    echo 'rendering done'
}

function run_gt(){
    echo 'rendering gt'
    conda activate layeredBsdf
    source /home/lzr/Projects/layeredbsdf/setpath.sh
    for matname in `cat /home/lzr/Projects/mitsuba/experiment/ablation/material_set.txt`
    do
        path="/home/lzr/Projects/mitsuba/experiment/ablation/gy/sphere/$matname"
        
        if [ ! -d "$path" ]; then
            echo $matname
            arr=(${matname//_/ })
            alpha0=${arr[0]}
            eta0=${arr[3]}
            sigmat=${arr[4]}
            albedo0=${arr[5]}
            albedo1=${arr[6]}
            albedo2=${arr[7]}
            alpha1=${arr[9]}
            material=${arr[12]}
            if [ ${#arr[@]} -eq "14" ]; then
                material=${material}_${arr[13]}
            fi
            echo alpha0 is $alpha0
            echo eta0 is $eta0
            echo sigmat is $sigmat
            echo albedo is $albedo0,$albedo1,$albedo2
            echo alpha1 is $alpha1
            echo material is $material
            mkdir -p $path; 
            /home/lzr/Projects/layeredbsdf/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/ablation/sphere/sphere_gy.xml \
                -D maxDepth=$maxDepth -D rrDepth=$rrDepth -D sampleCount=$sampleCount_gy -D width=$width -D height=$height \
                -D eta_0=$eta0 -D alpha_0=${alpha0} -D sigmaT_0=${sigmat} -D albedo_0="${albedo0} ${albedo1} ${albedo2}" -D alpha_1=${alpha1} -D material=${material} \
                -o $path/${matname}_gy.exr
        fi
    done
    echo 'rendering gt done'
}

sampleCount=16
sampleCount_gy=256
width=256
height=256
maxDepth=100
rrDepth=20
GPU=0

run_gt

for model in 1108001 1209001 1209002
do
    run_case
done



