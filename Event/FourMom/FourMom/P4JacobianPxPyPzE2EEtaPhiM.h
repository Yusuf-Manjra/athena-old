/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef P4JacobianPxPyPzE2EEtaPhiM_H
#define P4JacobianPxPyPzE2EEtaPhiM_H

#include "CLHEP/Matrix/Matrix.h"

class P4JacobianPxPyPzE2EEtaPhiM : public CLHEP::HepMatrix {
public:

  P4JacobianPxPyPzE2EEtaPhiM( double px, double py, double pz, double E);

};

#endif
