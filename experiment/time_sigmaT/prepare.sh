MODEL_ROOT=/home/lzr/layeredBsdfModel
PARAM_ROOT=/home/lzr/layeredBsdfData/meta_texture/1108001/time_sigmaT

# 生成网络参数
for sigmaT_0 in `seq 0 0.5 5`
do
  mkdir -p ${PARAM_ROOT}/roughness_0.1_sigmaT_${sigmaT_0}
  python ${MODEL_ROOT}/meta_texture_maker_batch.py --GPU=-1 \
    --model_dir=${MODEL_ROOT}/checkpoints/1108001 \
    --out_dir=${PARAM_ROOT}/roughness_0.1_sigmaT_${sigmaT_0} \
    --sh_dim=81\
    --surface_type=conductor --is_sv=0 --filename=0.1_0_0_1.5_${sigmaT_0}_0.03_0.6_0.8_0_0.1_0_0_Au
done
