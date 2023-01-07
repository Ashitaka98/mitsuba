for alpha_1 in 0.005; do
    for sigmat in 3; do
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1123001/globe/roughness_${alpha_1}; \
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1123001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1123001/globe/roughness_${alpha_1} \
            --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/video_globe/textures \
            --surface_type=conductor --is_sv=1 --filename=${alpha_1}_0_0_1.5_${sigmat}_0_0_0_0_0.05_0_0_Hg
    done
done
