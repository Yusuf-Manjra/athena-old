#!/bin/sh

# art-include: 21.6/AthGeneration
# art-include: master/AthGeneration
# art-description: Herwig7 ART test dijet dipole
# art-type: grid
# art-output: EVNT.root
# art-output: MyOutput.root
# art-output: MyOutput.yoda.gz
# art-output: dcube
# art-html: dcube

Gen_tf.py --ecmEnergy=13000. --maxEvents=10000 --randomSeed=123456 --jobConfig=421401 --outputEVNTFile=EVNT.root --outputYODAFile=MyOutput.yoda.gz --postInclude=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Herwig7_i/MC_JO/MC_test_01_dijet_dipole.py 

echo "art-result: $? Gen_tf"

asetup 22.6.1,AthGeneration
source setupRivet

python /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Pythia8i/rootconvert.py MyOutput.yoda.gz

echo "art-result: $? Plot"

dcubeName="Herwig7"
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Herwig7_i/config_file/test_01_config.xml"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Herwig7_i/master_branch/reference/test_01_output.root"

bash /cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube $dcubeName MyOutput.root $dcubeXml $dcubeRef

echo  "art-result: $? DCube"


