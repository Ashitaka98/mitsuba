

#vase 1
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_1/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_1/ \
    --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/textures/vase_1 \
    --surface_type=conductor --is_sv=1 --filename=0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Na_palik

#vase 2
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_2/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_2/ \
    --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/textures/vase_2 \
    --surface_type=conductor --is_sv=1 --filename=0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Al

#vase 3
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_3/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_3/ \
    --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/textures/vase_3 \
    --surface_type=conductor --is_sv=1 --filename=0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Hg

#vase 4
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_4/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_4/ \
    --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/textures/vase_4 \
    --surface_type=conductor --is_sv=1 --filename=0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Ag

#vase 5
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_5/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/vase_5/ \
    --sh_dim=81 --texture_dir=/home/lzr/Projects/mitsuba/experiment/teaser/teaser_ours/textures/vase_5 \
    --surface_type=conductor --is_sv=1 --filename=0.1_0_0_1.5_1_0_0_0_0_0.1_0_0_Ag

#table top(not sv)
mkdir -p /home/lzr/layeredBsdfData/meta_texture/1108001/teaser/table_top/; \
python /home/lzr/layeredBsdfModel/meta_texture_maker_batch.py --GPU=1 \
    --model_dir=/home/lzr/layeredBsdfModel/checkpoints/1108001 \
    --out_dir=/home/lzr/layeredBsdfData/meta_texture/1108001/teaser/table_top/ \
    --sh_dim=81 \
    --surface_type=conductor --is_sv=0 --filename=0.1_0_0_1.5_1_1_1_1_0_0.1_0_0_Ag
