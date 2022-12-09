#生成网络参数
for alpha_0 in 0.02 0.05 0.1 0.2
do
    for alpha_1 in 0.02 0.05 0.1 0.2
    do
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/utah_teapot/alpha0_${alpha_0}_alphpa1_${alpha_1}; \
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/utah_teapot/alpha0_${alpha_0}_alpha1_${alpha_1} \
            --sh_dim=81 \
            --surface_type=conductor --is_sv=0 --filename=${alpha_0}_0_0_1.5_0_0.9_0.8_0.6_0_${alpha_1}_0_0_Hg
    done
done