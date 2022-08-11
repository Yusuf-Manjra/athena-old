#!/bin/sh

# art-description: MC+MC Overlay chain for MC20e, ttbar, full reco chain, 1000 events, AthenaMT
# art-type: grid
# art-include: master/Athena/x86_64-centos7-gcc11-opt
# art-include: master/Athena/x86_64-centos7-clang13-opt
# art-include: 22.0-mc20/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8

events=1000
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"

Reco_tf.py \
--multithreaded='True' \
--autoConfiguration everything \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02 \
--geometryVersion default:ATLAS-R2-2016-01-00-01 \
--maxEvents ${events} --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--preInclude "all:Campaigns/MC20e.py" \
--postInclude "default:PyJobTransforms/UseFrontier.py" \
--runNumber 410470 \
--steering "doOverlay" "doRDO_TRIG" "doTRIGtoALL" \
--triggerConfig "RDOtoRDOTrigger=MCRECO:DBF:TRIGGERDBMC:2233,87,314" --asetup "RDOtoRDOTrigger:Athena,21.0,latest" \
--outputAODFile MC_plus_MC.AOD.pool.root \
--postExec 'FPEAuditor.NStacktracesOnFPE=10' \
--imf False

rc1=$?
echo "art-result: ${rc1} Reco_tf_overlay_fullchain_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
exit $rc1
