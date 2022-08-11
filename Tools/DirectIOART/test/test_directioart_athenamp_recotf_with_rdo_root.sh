#!/bin/bash

# art-description: DirectIOART AthenaMP Reco_tf.py inputFile:RDO protocol=ROOT
# art-type: grid
# art-output: *.pool.root
# art-include: 21.0/Athena
# art-athena-mt: 2

set -e

unset ATHENA_PROC_NUMBER; Reco_tf.py --AMI q221 --athenaopts="--nprocs=2 --threads=1" --inputRDOFile root://lcg-lrz-rootd.grid.lrz.de:1094/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc16_13TeV/90/96/RDO.11373415._000001.pool.root.1 --outputRDO_TRIGFile art.pool.root

echo "art-result: $? DirectIOART_AthenaMP_RecoTF_inputRDO_protocol_ROOT"
