#! /bin/bash
source ~/.bashrc
source activate
sampleCount=256

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

Angles=("2" "11" "-11")
# Angles=("-19" "-2" "7" "28")
for tex in 4; do
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
            mkdir -p /home/lzr/Projects/mitsuba/experiment/video_cozyplace/output/svroughness_${tex}
            /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/video_cozyplace/cozyplace.xml \
                -D maxDepth=100 -D rrDepth=20 -D sampleCount=$sampleCount -D width=800 -D height=1200 \
                -D paramdir1="1229001/cozyplace/roughnessTex_$tex" \
                -D paramdir2="1229002/cozyplace/roughnessTex_$tex" \
                -D paramstr="-1_0_0_1.5_0_1_1_1_0_-1_0_0_-1" \
                -D eta1=1.5 -D texWidth=800 -D texHeight=2000 -D origin="$x $y $z" \
                -o /home/lzr/Projects/mitsuba/experiment/video_cozyplace/output/svroughness_${tex}/angle_${angle}_$sampleCount.exr
        fi
        let angle++
    done
done