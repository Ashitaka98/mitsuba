#生成网络参数
for alpha_1 in 0.1
do
    for sigmat in 2
    do
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/matpreview/roughness_${alpha_1}_sigmat_${sigmat}; \
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/matpreview/roughness_${alpha_1}_sigmat_${sigmat} \
            --sh_dim=81 \
            --surface_type=conductor --is_sv=0 --filename=${alpha_1}_0_0_1.9_${sigmat}_0.6_0.4_0.8_0_0.1_0_0_Ag
    done
done