#!/bin/sh
#
# art-description: MC21-style simulation using FullG4MT_QS
# art-include: 22.0/Athena
# art-include: master/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

unset ATHENA_CORE_NUMBER

# RUN3 setup
# ATLAS-R3S-2021-02-00-00 and OFLCOND-MC21-SDR-RUN3-05
    Sim_tf.py \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'default:PyJobTransforms/UseFrontier.py' \
        --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
        --DataRunNumber '410000' \
        --geometryVersion 'default:ATLAS-R3S-2021-02-00-00_VALIDATION' \
        --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
        --outputHITSFile "test.CG.HITS.pool.root" \
        --maxEvents 4 \
        --imf False \
        --athenaopts '"--config-only=ConfigSimCG.pkl"'

    Sim_tf.py \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'default:PyJobTransforms/UseFrontier.py' \
        --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
        --DataRunNumber '410000' \
        --geometryVersion 'default:ATLAS-R3S-2021-02-00-00_VALIDATION' \
        --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
        --outputHITSFile "test.CG.HITS.pool.root" \
        --maxEvents 4 \
        --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo  "art-result: $rc simCG"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    Sim_tf.py \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'default:PyJobTransforms/UseFrontier.py' \
        --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
        --DataRunNumber '410000' \
        --geometryVersion 'default:ATLAS-R3S-2021-02-00-00_VALIDATION' \
        --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
        --outputHITSFile "test.CG.HITS.pool.root" \
        --maxEvents 4 \
        --DBRelease '400.0.445' \
        --imf False \
        --athenaopts '"--config-only=ConfigSimCGDB.pkl"'

    Sim_tf.py \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'default:PyJobTransforms/UseFrontier.py' \
        --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
        --DataRunNumber '410000' \
        --geometryVersion 'default:ATLAS-R3S-2021-02-00-00_VALIDATION' \
        --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
        --outputHITSFile "test.CGDB.HITS.pool.root" \
        --maxEvents 4 \
        --DBRelease '400.0.445' \
        --imf False

    rc2=$?
    status=$rc2
    mv log.EVNTtoHITS log.EVNTtoHITS.CGDB
fi
echo "art-result: $rc2 simOLD_DB"

rc3=-9999
if [ $rc2 -eq 0 ]
then
  # Compare the outputs
  acmd.py diff-root test.CG.HITS.pool.root test.CGDB.HITS.pool.root \
    --error-mode resilient \
    --mode semi-detailed \
    --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
  rc3=$?
  status=$rc3
fi

echo "art-result: $rc3 OLDvsOLD_DB"
rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
