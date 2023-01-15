#! /bin/bash
source ~/.bashrc
source activate
sampleCount=1024

function contains() {
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [ "${!i}" == "${value}" ]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

# Angles=("-29" "-18" "-12" "-6" "-2" "3" "7" "10" "14" "21" "24")

Angles=("7")

for tex in 3; do
    angle=-30
    for pos in `cat /home/lzr/Projects/mitsuba/experiment/video_cozyplace/camera_pos.txt`; do
        if [ $(contains "${Angles[@]}" "$angle") == "y" ] ; then
            echo $angle
            echo $pos
            arr=(${pos//_/ })
            x=${arr[0]}
            y=${arr[1]}
            z=${arr[2]}
            echo x is $x
            echo y is $y
            echo z is $z
            conda activate layeredBsdf
            source setpath.sh
            mkdir -p /home/lzr/Projects/mitsuba/experiment/video_cozyplace/output/svalbedo_${tex}_test
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_cozyplace/cozyplace_albedo.xml \
                -D maxDepth=5 -D rrDepth=5 -D sampleCount=$sampleCount -D width=800 -D height=1200 \
                -D paramdir1="1229001/cozyplace/albedoTex_$tex" \
                -D paramdir2="1229002/cozyplace/albedoTex_$tex" \
                -D paramstr="0.01_0_0_1.5_0.5_1_1_1_0_0.01_0_0_-1" \
                -D eta1=1.5 -D texWidth=800 -D texHeight=2000 -D origin="$x $y $z" \
                -o /home/lzr/Projects/mitsuba/experiment/video_cozyplace/output/svalbedo_${tex}_test/angle_${angle}_${sampleCount}.exr
        fi
        let angle++
    done
done