for sigmat in 1; do
    mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/alarm/sigmat_${sigmat}; \
    python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
        --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
        --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/alarm/sigmat_${sigmat} \
        --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/alarm/tex_temp \
        --surface_type=conductor --is_sv=1 --filename=-1_0_0_1.5_${sigmat}_1_1_1_0_0.05_0_0_Hg
done
