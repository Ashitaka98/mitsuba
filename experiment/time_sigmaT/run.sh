MTS_PROJECT_ROOT=/home/lzr/Projects/mitsuba
PARAM_ROOT=/home/lzr/layeredBsdfData/meta_texture/1108001/time_sigmaT

source ${MTS_PROJECT_ROOT}/setpath.sh

function log() {
  echo -ne "$1" >> run.log
}

log "######################################################################\n"
log "## Experiment started at $(date)\n"
log "######################################################################\n\n"

function run_case() {
  paramdir=${PARAM_ROOT}/roughness_0.1_sigmaT_$1
  paramstr=0.1_0_0_1.5_${sigmaT_0}_0.03_0.6_0.8_0_0.1_0_0_Au

  ${MTS_PROJECT_ROOT}/dist/mitsuba dragon_ours.xml \
    -D paramdir=${paramdir} \
    -D paramstr=${paramstr} \
    -o spp64_box_bin/dragon_ours_$1.exr
}

for sigmaT_0 in `seq 0 0.5 5`
do
  log "Timing for sigmaT_0=${sigmaT_0} ... "
  mts_output="$(run_case ${sigmaT_0})"
  log "$(echo ${mts_output} | grep -Po 'Render time: \K.*? ')\n\n"
done
