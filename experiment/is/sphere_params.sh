#生成网络参数
for alpha_0 in 0.01 0.02
do
    for alpha_1 in 0.1 0.2
    do
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/sphere/alpha0_${alpha_0}_alpha1_${alpha_1}; \
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=-1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/sphere/alpha0_${alpha_0}_alpha1_${alpha_1} \
            --sh_dim=81 \
            --surface_type=conductor --is_sv=0 --filename=${alpha_0}_0_0_1.9_0_0.6_0.4_0.8_0_${alpha_1}_0_0_Ag
    done
done