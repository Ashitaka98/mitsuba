function log() {
  echo -ne "$1" >> run.log
}

log "######################################################################\n"
log "## Experiment started at $(date)\n"
log "######################################################################\n\n"

function run_case() {
    mkdir -p /home/lzr/Projects/mitsuba/experiment/plane/output/time_sigmat
    /home/lzr/Projects/mitsuba/dist/mitsuba /home/lzr/Projects/mitsuba/experiment/plane/plane.xml \
        -D maxDepth=100 -D rrDepth=20 -D sampleCount=64 -D width=1024 -D height=1024 \
        -D paramdir1="1219001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
        -D paramdir2="1218001/plane/roughness_${alpha_1}_sigmat_${sigmat}" \
        -D paramstr="${alpha_1}_0_0_1.5_${sigmat}_${color}_${color}_${color}_0_${alpha_1}_0_0_-1" \
        -D eta1=1.5 \
        -o /home/lzr/Projects/mitsuba/experiment/plane/output/time_sigmat/roughness_${alpha_1}_sigmat_${sigmat}_64.exr
}

alpha_1=0.1
color=0
for sigmat in `seq 0 0.5 5`
do
  log "Timing for sigmat=${sigmat} ... "
  mts_output="$(run_case ${sigmat})"
  log "$(echo ${mts_output} | grep -Po 'Render time: \K.*? ')\n\n"
done
