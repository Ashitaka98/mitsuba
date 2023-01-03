#生成网络参数
# for alpha_1 in 0.005 0.01 0.02 0.05 0.1 0.2 0.25; do
#     for sigmat in 0 1 2 5; do
#         for color in 0.95 0.9 0.8 0.7 0.6; do
#             #BRDF model
#             mkdir -p /home/lzr/layeredBsdfData/meta_texture/1219001/plane/roughness_${alpha_1}_sigmat_${sigmat}
#             python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
#                 --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1219001 \
#                 --out_dir=/home/lzr/layeredBsdfData/meta_texture/1219001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
#                 --sh_dim=81 \
#                 --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1
#             #BTDF model
#             mkdir -p /home/lzr/layeredBsdfData/meta_texture/1218001/plane/roughness_${alpha_1}_sigmat_${sigmat}
#             python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
#                 --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1218001 \
#                 --out_dir=/home/lzr/layeredBsdfData/meta_texture/1218001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
#                 --sh_dim=81 \
#                 --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1
#             done
#     done
# done
for alpha_1 in 0.1; do
    for sigmat in `seq 0 0.5 5`; do
        for color in 0; do
            #BRDF model
            mkdir -p /home/lzr/layeredBsdfData/meta_texture/1219001/plane/roughness_${alpha_1}_sigmat_${sigmat}
            python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=2 \
                --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1219001 \
                --out_dir=/home/lzr/layeredBsdfData/meta_texture/1219001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
                --sh_dim=81 \
                --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1
            #BTDF model
            mkdir -p /home/lzr/layeredBsdfData/meta_texture/1218001/plane/roughness_${alpha_1}_sigmat_${sigmat}
            python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=2 \
                --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1218001 \
                --out_dir=/home/lzr/layeredBsdfData/meta_texture/1218001/plane/roughness_${alpha_1}_sigmat_${sigmat} \
                --sh_dim=81 \
                --surface_type=dielectric --is_sv=0 --filename=${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1
            done
    done
done