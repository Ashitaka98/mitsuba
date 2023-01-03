#! /bin/bash
source ~/.bashrc
source activate
sampleCount=64
for sigmat in 1 ; do
    for alpha0 in 0.01 ; do
        for tex_albedo in 2 4 5 6 7; do
            # conda activate layeredModel 
            # cp /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_albedo/${tex_albedo}.exr /home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_albedo/albedo.exr
            # #BRDF model
            # mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229001/cozyplace/albedoTex_${tex_albedo}
            # python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            #     --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229001 \
            #     --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229001/cozyplace/albedoTex_${tex_albedo} \
            #     --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_albedo \
            #     --surface_type=dielectric --is_sv=1 --filename=${alpha0}_0_0_1.5_${sigmat}_1_1_1_0_${alpha0}_0_0_-1
            # #BTDF model
            # mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229002/cozyplace/albedoTex_${tex_albedo}
            # python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            #     --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229002 \
            #     --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229002/cozyplace/albedoTex_${tex_albedo} \
            #     --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/cozyplace/tex_temp_albedo \
            #     --surface_type=dielectric --is_sv=1 --filename=${alpha0}_0_0_1.5_${sigmat}_1_1_1_0_${alpha0}_0_0_-1

            conda activate layeredBsdf
            source setpath.sh
            mkdir -p /home/lzr/Projects/mitsuba/experiment/cozyplace/output/svalbedo
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/cozyplace/cozyplace.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=1200 \
                -D paramdir1="1229001/cozyplace/albedoTex_${tex_albedo}" \
                -D paramdir2="1229002/cozyplace/albedoTex_${tex_albedo}" \
                -D paramstr="${alpha0}_0_0_1.5_${sigmat}_1_1_1_0_${alpha0}_0_0_-1" \
                -D eta1=1.5 -D texWidth=700 -D texHeight=1500 \
                -o /home/lzr/Projects/mitsuba/experiment/cozyplace/output/svalbedo/tex_${tex_albedo}_roughness_${alpha0}_sigmat_${sigmat}_$sampleCount.exr

            
        done
    done
done