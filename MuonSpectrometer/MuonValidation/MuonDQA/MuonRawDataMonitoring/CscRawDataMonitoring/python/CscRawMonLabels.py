#
#	Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

labels_occupancy_signal_EA=['','','','','','L01:1', 'L01:2', 'L01:3', 'L01:4', '', 'S02:1', 'S02:2', 'S02:3', 'S02:4', '', 'L03:1', 'L03:2', 'L03:3', 'L03:4', '', 'S04:1', 'S04:2', 'S04:3', 'S04:4', '', 'L05:1', 'L05:2', 'L05:3', 'L05:4', '', 'S06:1', 'S06:2', 'S06:3', 'S06:4', '', 'L07:1', 'L07:2', 'L07:3', 'L07:4', '', 'S08:1', 'S08:2', 'S08:3', 'S08:4', '', 'L09:1', 'L09:2', 'L09:3', 'L09:4', '', 'S10:1', 'S10:2', 'S10:3', 'S10:4', '', 'L11:1', 'L11:2', 'L11:3', 'L11:4', '', 'S12:1', 'S12:2', 'S12:3', 'S12:4', '', 'L13:1', 'L13:2', 'L13:3', 'L13:4', '', 'S14:1', 'S14:2', 'S14:3', 'S14:4', '', 'L15:1', 'L15:2', 'L15:3', 'L15:4', '', 'S16:1', 'S16:2', 'S16:3', 'S16:4', '','','','','','']
labels_occupancy_signal_EC=['','','','','','S16:1', 'S16:2', 'S16:3', 'S16:4', '', 'L15:1', 'L15:2', 'L15:3', 'L15:4', '', 'S14:1', 'S14:2', 'S14:3', 'S14:4', '', 'L13:1', 'L13:2', 'L13:3', 'L13:4', '', 'S12:1', 'S12:2', 'S12:3', 'S12:4', '', 'L11:1', 'L11:2', 'L11:3', 'L11:4', '', 'S10:1', 'S10:2', 'S10:3', 'S10:4', '', 'L09:1', 'L09:2', 'L09:3', 'L09:4', '', 'S08:1', 'S08:2', 'S08:3', 'S08:4', '', 'L07:1', 'L07:2', 'L07:3', 'L07:4', '', 'S06:1', 'S06:2', 'S06:3', 'S06:4', '', 'L05:1', 'L05:2', 'L05:3', 'L05:4', '', 'S04:1', 'S04:2', 'S04:3', 'S04:4', '', 'L03:1', 'L03:2', 'L03:3', 'L03:4', '', 'S02:1', 'S02:2', 'S02:3', 'S02:4', '', 'L01:1', 'L01:2', 'L01:3', 'L01:4', '']
labels_sgm_status = [ "Unspoiled", "Simple", "Edge", "MultiPeak", "Narrow", "Wide", "Skewed", "QRatInc", "StripFitFailed", "SplitUnspoiled", "SplitSimple", "SplitEdge", "SplitMultiPeak", "SplitNarrow", "SplitWide", "SplitSkewed", "SplitQRatInc", "SplitStripFitFailed", "Undefined" ]
labels_sgm_clusCounts = []
for i in range(50):
	if(i == 5):
		labels_sgm_clusCounts.append('234')
	elif(i == 15):
		labels_sgm_clusCounts.append('134')
	elif(i == 25):
		labels_sgm_clusCounts.append('124')
	elif(i == 35):
		labels_sgm_clusCounts.append('123')
	elif(i == 45):
		labels_sgm_clusCounts.append('All')
	else:
		labels_sgm_clusCounts.append('')
