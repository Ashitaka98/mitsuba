mkdir -p /home/lzr/layeredBsdfData/meta_texture/1202001/shoe/roughness_0.02_sigmat_4; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1202001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1202001/shoe/roughness_0.02_sigmat_4 \
    --sh_dim=81 \
    --surface_type=conductor --is_sv=0 --filename=0.02_0_0_1.9_4_0.9_0.2_0.1_0_0.04_0_0_Ag
