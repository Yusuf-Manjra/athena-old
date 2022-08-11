# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#
# File: CaloClusterCorrection/python/CaloSwLongWeights_old.py
# Created: Nov 2006, sss
# Purpose: Longitudinal weights corrections, original version.
#
# This is the original version of the longitudinal weight corrections,
# derived by Stathes Paganis and Karina Loureiro.  It was added in
# LArClusterRec-02-05-10, in 8.5.0, and received a few fixes
# in LArClusterRec-02-05-25, in 9.0.3.
#
# Derived from electrons with energies of 20, 50, 100, 150 GeV
# with |eta| < 2.5 with fixed vertex.
#
# There is only one set of constants here, which is used for both electron
# and photons, at all cluster sizes.  See comments below about a multi-size
# issue, though.
#


from CaloClusterCorrection.constants import CALOCORR_COMBINED2

#######################################################################
# The old layer weights corrections, that were used in release 9 + 10.0.0.
CaloSwLongWeights_old = [
               # w0        w3       escale    eoffset
    [0.012500, 0.9184,   1.1895,   1.0309,   0.0125],  
    [0.037500, 0.8539,   0.9963,   1.0320,   0.0375],  
    [0.062500, 0.8524,   1.0739,   1.0305,   0.0625],  
    [0.087500, 0.8722,   1.0842,   1.0322,   0.0875],  
    [0.112500, 0.9170,   1.0852,   1.0332,   0.1125],  
    [0.137500, 0.8829,   1.0667,   1.0352,   0.1375],  
    [0.162500, 0.8180,   1.1028,   1.0354,   0.1625],  
    [0.187500, 0.8793,   1.1396,   1.0352,   0.1875],  
    [0.212500, 0.8787,   1.0558,   1.0358,   0.2125],  
    [0.237500, 0.8732,   0.9990,   1.0366,   0.2375],  
    [0.262500, 0.8945,   0.7700,   1.0382,   0.2625],  
    [0.287500, 0.8835,   0.8898,   1.0374,   0.2875],  
    [0.312500, 0.8424,   0.9188,   1.0385,   0.3125],  
    [0.337500, 0.8460,   0.5436,   1.0403,   0.3375],  
    [0.362500, 0.8891,   0.6496,   1.0403,   0.3625],  
    [0.387500, 0.8845,   0.5783,   1.0394,   0.3875],  
    [0.412500, 0.8838,   0.6480,   1.0391,   0.4125],  
    [0.437500, 0.8518,   0.5489,   1.0398,   0.4375],  
    [0.462500, 0.9279,   0.7136,   1.0395,   0.4625],  
    [0.487500, 0.9169,   0.8641,   1.0377,   0.4875],  
    [0.512500, 0.8887,   0.7498,   1.0395,   0.5125],  
    [0.537500, 0.9180,   0.7701,   1.0372,   0.5375],  
    [0.562500, 0.8483,   0.6088,   1.0401,   0.5625],  
    [0.587500, 0.9369,   0.6797,   1.0382,   0.5875],  
    [0.612500, 0.8862,   0.6738,   1.0388,   0.6125],  
    [0.637500, 0.9037,   0.5376,   1.0378,   0.6375],  
    [0.662500, 0.9048,   0.6425,   1.0367,   0.6625],  
    [0.687500, 0.9084,   0.6749,   1.0356,   0.6875],  
    [0.712500, 0.9254,   0.7304,   1.0345,   0.7125],  
    [0.737500, 0.9214,   0.5986,   1.0374,   0.7375],  
    [0.762500, 0.9580,   0.7591,   1.0370,   0.7625],  
    [0.787500, 0.9625,   0.7187,   1.0440,   0.7875],  
    [0.812500, 0.9621,   1.0624,   1.0383,   0.8125],  
    [0.837500, 0.9390,   0.8696,   1.0343,   0.8375],  
    [0.862500, 0.9237,   0.6808,   1.0347,   0.8625],  
    [0.887500, 0.9367,   0.7999,   1.0336,   0.8875],  
    [0.912500, 0.9192,   0.5430,   1.0360,   0.9125],  
    [0.937500, 0.9238,   0.7012,   1.0350,   0.9375],  
    [0.962500, 0.9427,   0.6160,   1.0377,   0.9625],  
    [0.987500, 0.9446,   0.6267,   1.0380,   0.9875],  
    [1.012500, 0.9395,   0.6647,   1.0384,   1.0125],  
    [1.037500, 0.9546,   0.4892,   1.0411,   1.0375],  
    [1.062500, 0.9455,   0.5777,   1.0422,   1.0625],  
    [1.087500, 0.9422,   0.5512,   1.0435,   1.0875],  
    [1.112500, 0.9464,   0.5608,   1.0447,   1.1125],  
    [1.137500, 0.9591,   0.5350,   1.0460,   1.1375],  
    [1.162500, 0.9451,   0.3708,   1.0484,   1.1625],  
    [1.187500, 0.9482,   0.3379,   1.0520,   1.1875],  
    [1.212500, 0.9511,   0.2735,   1.0556,   1.2125],  
    [1.237500, 0.9763,   0.3651,   1.0513,   1.2375],  
    [1.262500, 0.9797,   0.2695,   1.0518,   1.2625],  
    [1.287500, 0.9776,   0.0000,   1.0566,   1.2875],  
    [1.312500, 0.9797,   0.2224,   1.0569,   1.3125],  
    [1.337500, 0.9743,   0.0635,   1.0622,   1.3375],  
    [1.362500, 0.9693,   0.0000,   1.0635,   1.3625],  
    [1.387500, 0.9667,   0.9448,   1.0798,   1.3875],  
    [1.412500, 0.9755,   1.2780,   1.0927,   1.4125],  
    [1.437500, 0.9890,   0.5688,   1.1800,   1.4375],  
    [1.462500, 0.9845,   1.5194,   1.3279,   1.4625],  
    [1.487500, 1.0103,   1.6738,   1.5319,   1.4875],  
    [1.512500, 1.0576,   0.0000,   1.4285,   1.5125],  
    [1.537500, 1.2602,   0.8485,   1.0674,   1.5375],  
    [1.562500, 1.0878,   0.9894,   1.0141,   1.5625],  
    [1.587500, 0.9365,   0.4089,   1.0134,   1.5875],  
    [1.612500, 1.0035,   0.5964,   1.0141,   1.6125],  
    [1.637500, 1.0867,   0.7597,   1.0192,   1.6375],  
    [1.662500, 1.1186,   0.7015,   1.0200,   1.6625],  
    [1.687500, 1.1758,   0.6681,   1.0172,   1.6875],  
    [1.712500, 1.0180,   0.6487,   1.0178,   1.7125],  
    [1.737500, 0.8115,   0.8384,   1.0220,   1.7375],  
    [1.762500, 0.7456,   0.8645,   1.0213,   1.7625],  
    [1.787500, 0.7509,   0.9944,   1.0228,   1.7875],  
    [1.812500, 0.5444,   0.8145,   1.0310,   1.8125],  
    [1.837500, 0.4712,   0.8456,   1.0265,   1.8375],  
    [1.862500, 0.0000,   0.9558,   1.0222,   1.8625],  
    [1.887500, 0.0000,   0.9651,   1.0226,   1.8875],  
    [1.912500, 2.5000,   0.8825,   1.0227,   1.9125],  
    [1.937500, 2.5000,   0.8621,   1.0241,   1.9375],  
    [1.962500, 2.5000,   0.9450,   1.0239,   1.9625],  
    [1.987500, 2.5000,   0.9200,   1.0269,   1.9875],  
    [2.012500, 2.5000,   0.9359,   1.0275,   2.0125],  
    [2.037500, 2.5000,   0.8121,   1.0266,   2.0375],  
    [2.062500, 2.5000,   0.8888,   1.0297,   2.0625],  
    [2.087500, 2.5000,   0.9147,   1.0269,   2.0875],  
    [2.112500, 2.5000,   0.8688,   1.0255,   2.1125],  
    [2.137500, 2.5000,   0.8608,   1.0262,   2.1375],  
    [2.162500, 2.5000,   0.7335,   1.0275,   2.1625],  
    [2.187500, 2.5000,   1.0163,   1.0223,   2.1875],  
    [2.212500, 2.5000,   0.8506,   1.0269,   2.2125],  
    [2.237500, 2.5000,   0.6934,   1.0286,   2.2375],  
    [2.262500, 2.5000,   0.9199,   1.0256,   2.2625],  
    [2.287500, 2.5048,   0.8158,   1.0229,   2.2875],  
    [2.312500, 2.5045,   0.7018,   1.0216,   2.3125],  
    [2.337500, 2.5052,   0.8360,   1.0189,   2.3375],  
    [2.362500, 2.5054,   0.8671,   1.0194,   2.3625],  
    [2.387500, 2.5051,   0.8499,   1.0222,   2.3875],  
    [2.412500, 2.5054,   0.6835,   1.0222,   2.4125],  
    [2.437500, 2.5059,   0.9177,   1.0154,   2.4375],  
    [2.462500, 2.5063,   0.8476,   1.0163,   2.4625],  
    #[2.487500, 2.5047,   0.8776,   1.0280,   0.0000],
    [2.487500, 2.5047,   0.8776,   1.0280,   2.4625],
    ]



#######################################################################


class CaloSwLongWeights_old_parms:
    eta_start_crack = 1.375
    eta_end_crack = 1.525
    etamax = 2.5
    use_raw_eta = False

    # This is used to help paper over an awkwardness in the gap region.
    # The gap correction adds scintillator energy to the cluster.
    # The C++ version of the gap correction does not include scintillator
    # energy in the cluster layer energies.  However, the python version
    # of the correction scales all the layer energies when the scintillator
    # is added (so that the layer sum is the same as the total energy).
    #
    # The layer weights correction used to throw away the scintillator
    # information entirely.  This is mostly OK, since the LW correction
    # is applied outside the gap.  However, there is a small overlap region
    # where it matters.
    #
    # It turns out that when the corrections were derived, for the 5x5
    # clusters, the python version of the gap correction was used upstream,
    # while for the other sizes, the C++ version was used.  Thus, there's
    # a systematic difference in the overlap region.  We tell the layer weights
    # correction to try to preserve any offset for the 5x5 clusters, but
    # not for other sizes.
    #
    # This is cleaned up in later versions of the correction.
    preserve_offset = {'ele55' : True,
                       'ele35' : False,
                       'ele37' : False,
                       'ele57' : False,
                       'ele77' : False,
                       'ele33' : False,
                       'gam55' : True,
                       'gam35' : False,
                       'gam37' : False,
                       'gam57' : False,
                       'gam77' : False,
                       'gam33' : False,
                       }
                       

    region = CALOCORR_COMBINED2
    degree = 3
    correction = CaloSwLongWeights_old
