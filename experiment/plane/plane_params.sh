#生成网络参数
for alpha_1 in 0.1; do
    for sigmat in 2; do
        #BRDF model
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1114001/plane/roughness_${alpha_1}_sigmat_${sigmat}
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1114001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1114001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
            --sh_dim=81 \
            --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_0.9_0.8_0.6_0_0.1_0_0_1.2
        #BTDF model
        mkdir -p /home/lzr/layeredBsdfData/meta_texture/1205001/plane/roughness_${alpha_1}_sigmat_${sigmat}
        python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
            --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1205001 \
            --out_dir=/home/lzr/layeredBsdfData/meta_texture/1205001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
            --sh_dim=81 \
            --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_0.9_0.8_0.6_0_0.1_0_0_1.2
    done
done
