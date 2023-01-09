for sigmat in 0 1 2 3
do
    mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/dragon/sigmat_${sigmat}; \
    python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
        --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
        --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/dragon/sigmat_${sigmat} \
        --sh_dim=81 \
        --surface_type=conductor --is_sv=0 --filename=0.05_0_0_1.5_${sigmat}_0.03_0.6_0.8_0_0.05_0_0_Au
done