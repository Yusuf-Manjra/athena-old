#!/bin/sh

# art-include: master/AthDerivation
# art-include: master/Athena
# art-description: DAOD building IDTR2 mc20
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt
# art-output: checkIndexRefs.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13167/AOD.27162646._000001.pool.root.1 --outputDAODFile art.pool.root --reductionConf IDTR2 --maxEvents -1 --preExec 'from AthenaCommon.DetFlags import DetFlags; DetFlags.detdescr.all_setOff(); DetFlags.BField_setOn(); DetFlags.digitize.all_setOff(); DetFlags.detdescr.Calo_setOn(); DetFlags.simulate.all_setOff(); DetFlags.pileup.all_setOff(); DetFlags.overlay.all_setOff(); DetFlags.detdescr.pixel_setOn(); DetFlags.detdescr.SCT_setOn(); from InDetRecExample.InDetJobProperties import InDetFlags; InDetFlags.doR3LargeD0.set_Value_and_Lock(True);'  --passThrough True 

echo "art-result: $? reco"

checkFile.py DAOD_IDTR2.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_IDTR2.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_IDTR2.art.pool.root > checkIndexRefs.txt

echo "art-result: $?  checkIndexRefs"
