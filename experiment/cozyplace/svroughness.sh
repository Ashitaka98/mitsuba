#! /bin/bash
source ~/.bashrc
source activate
sampleCount=64
for sigmat in 0 ; do
    for color in 1 ; do

        for tex_roughness in 1 4 5 ; do
            # conda activate layeredModel 
            # cp /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_roughness/${tex_roughness}.exr /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_roughness/alpha0.exr
            # cp /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_roughness/${tex_roughness}.exr /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_roughness/alpha1.exr
            # #BRDF model
            # mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229001/cozyplace/roughnessTex_${tex_roughness}
            # python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=2 \
            #     --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229001 \
            #     --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229001/cozyplace/roughnessTex_${tex_roughness} \
            #     --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_roughness \
            #     --surface_type=dielectric --is_sv=1 --filename=-1_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_-1_0_0_-1
            # #BTDF model
            # mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229002/cozyplace/roughnessTex_${tex_roughness}
            # python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=2 \
            #     --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229002 \
            #     --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229002/cozyplace/roughnessTex_${tex_roughness} \
            #     --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_roughness \
            #     --surface_type=dielectric --is_sv=1 --filename=-1_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_-1_0_0_-1
            
            conda activate layeredBsdf
            source setpath.sh
            mkdir -p /home/lzr/Projects/mitsuba/experiment/cozyplace/output/svroughness_withoutEmitter
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/cozyplace/cozyplace.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=1200 \
                -D paramdir1="1229001/cozyplace/roughnessTex_${tex_roughness}" \
                -D paramdir2="1229002/cozyplace/roughnessTex_${tex_roughness}" \
                -D paramstr="-1_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_-1_0_0_-1" \
                -D eta1=1.5 -D texWidth=800 -D texHeight=2000 \
                -o /home/lzr/Projects/mitsuba/experiment/cozyplace/output/svroughness_withoutEmitter/color_new_tex_${tex_roughness}_sigmat_${sigmat}_$sampleCount.exr
        done
    done
done