#! /bin/bash
source ~/.bashrc
source activate
sampleCount=64
for alpha_1 in 0.02; do
    for sigmat in `seq 0 0.01 2`; do
        conda activate layeredModel
        #BRDF model
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229001/plane/roughness_${alpha_1}_sigmat_${sigmat}
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
            --sh_dim=81 \
            --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_0_0_0_0_${alpha_1}_0_0_-1
        #BTDF model
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1229002/plane/roughness_${alpha_1}_sigmat_${sigmat}
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1229002 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1229002/plane/roughness_${alpha_1}_sigmat_${sigmat} \
            --sh_dim=81 \
            --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_0_0_0_0_${alpha_1}_0_0_-1
        
        conda activate layeredBsdf
        source /home/lzr/Projects/mitsuba/setpath.sh
        mkdir -p /home/lzr/Projects/mitsuba/experiment/video_plane/output/sigmat_new
        /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_plane/plane.xml \
            -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=720 -D height=720 \
            -D paramdir1="1229001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
            -D paramdir2="1229002/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
            -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_0_0_0_0_${alpha_1}_0_0_-1" \
            -D eta1=1.5 \
            -o /home/lzr/Projects/mitsuba/experiment/video_plane/output/sigmat_new/roughness_${alpha_1}_sigmat_${sigmat}_$sampleCount.exr
    done
done