for alpha0 in 0.02 0.05 0.1; do
    mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/kettle/roughness_$alpha0; \
    python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
        --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
        --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/kettle/roughness_$alpha0 \
        --sh_dim=81 \
        --surface_type=conductor --is_sv=0 --filename=${alpha0}_0_0_1.49_0_0.7_0.529_0.4_0_0.005_0_0_Cu
done