/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


///////////////////////////////////////////////////////////////////
// TRT_DriftFunctionTool.h
//   Header file for class TRT_DriftFunctionTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used to go from drift time to drift distance
///////////////////////////////////////////////////////////////////

#ifndef TRT_DRIFTFUNCTIONTOOL
#define TRT_DRIFTFUNCTIONTOOL

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "TRT_DriftFunctionTool/ITRT_DriftFunctionTool.h"
#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
class TRT_ID;


#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "GaudiKernel/ToolHandle.h"
/**
 @class TRT_DriftFunctionTool
  
 * Provides DriftCircle data from RDO info
 * ie transforms a raw drifttime to a calibrated drift radius

*/
class TRT_DriftFunctionTool: public extends<AthAlgTool, ITRT_DriftFunctionTool>{

public:
  /** Constructor                           */
  TRT_DriftFunctionTool(const std::string& type,
		    const std::string& name,
		    const IInterface* parent);

  /** Destructor                            */	
  virtual ~TRT_DriftFunctionTool();

  /** Retrieves needed services             */
  virtual StatusCode initialize() override;

  /** Finalize                              */
  virtual StatusCode finalize() override;

  /** Returns True for drifttimes between -10 and 75ns */
  virtual bool isValidTime(double drifttime) const override;

  /** Returns True for CTB real data */
  virtual bool isTestBeamData() const override;

  /** Returns center of leading edge bin in ns for bin number = tdcvalue.
    * note, that the binwidth can vary with run type. */
  virtual double rawTime(int tdcvalue) const override;


  /** Returns drift radius in mm and t0 in ns
   *  The radius is truncated so it belongs to [0,2]mm.
   *  isOK is false if there is no t0 or the drifttime is non-valid */
  virtual double driftRadius(double rawtime, Identifier id, double& t0, bool& isOK, unsigned int word=0) const override;

  /** Returns drift radius for MC.
   *  the inpout time in ns has t0 subtracted */
  virtual double driftRadius(double drifttime) const override;

  /** Returns approximate drift time (t0 subtracted) */
  virtual double approxDriftTime(double driftradius) const override;

  /** Time-dependent error of drift radius in mm */
  virtual double errorOfDriftRadius(double drifttime, Identifier id, float mu = -10, unsigned int word=0) const override;

  /** Returns time over threshold correction to the drift time (ns) */
  virtual double driftTimeToTCorrection(double tot, Identifier id, bool isArgonStraw=false) const override;
  
  /** Returns high threshold correction to the drift time (ns) */
  virtual double driftTimeHTCorrection(Identifier id, bool isArgonStraw=false) const override;
  
  /** Initialise Rt relation in data */
  void setupRtRelationData();
  /** Initialise Rt relation in MC */
  void setupRtRelationMC();


private:
  
  /** Tool to fetch data from database */
  ToolHandle< ITRT_CalDbTool >   m_TRTCalDbTool;
  ToolHandle< ITRT_CalDbTool >   m_TRTCalDbTool2;

  /** DetectorManager and helper */
  const InDetDD::TRT_DetectorManager* m_manager{};
  const TRT_ID* m_trtid{};


  double m_drifttimeperbin;            //!< 3.125ns
  double m_error;                      //!< universal error
  
  enum ETimeBins { MaxTimeBin = 50 } ; //!< number of time bins
  double m_radius[MaxTimeBin]{};         //!< most probable radius in each bin
  double m_errors[MaxTimeBin]{};         //!< width of radius dist in each bin         
  double m_drifttimeperhalfbin;        //!< 1.5625ns

  bool m_isdata;                       //!< flag for real data
  bool m_ismc;                         //!< flag for mc
  bool m_isoverlay;                    //!< flag for overlay
  bool m_istestbeam;                   //!< flag for CTB data or mc

  bool m_allow_data_mc_override;       //!< treat mc as data or vice versa
  bool m_forcedata;                    //!< treat mc as data

  bool m_dummy;                        //!< flag for ignoring drift time info

  double m_err_fudge;                  //!< fudge_factor for error scaling

  bool  m_allow_digi_version_override; //!< flag for using constants for 
  int m_forced_digiversion;            //!< this digi version

  bool m_override_simcal;              //!< flag for reading constants from
  bool m_force_universal_errors;       //!< use one universal error
  double m_uni_error;                  //!< namely this one

  std::string m_inputfile;             //!< file overriding MC constants
  std::string m_key;                   //!< GeoModel version key
  std::string m_trt_mgr_location;      //!< Name of TRT detector manager
  double m_t0_barrel[3]{};               //!< t0 for the 3 barrel rings
  double m_t0_endcap[18]{};              //!< t0 for the 14(18) endcap wheels
  double m_t0_shift;                   //!< digiversion dependent t0 shift
  double m_ht_correction_barrel_Xe;    //!< HT correction for Xe straws in barrel
  double m_ht_correction_endcap_Xe;    //!< HT correction for Xe straws in barrel
  double m_ht_correction_barrel_Ar;    //!< HT correction for Ar straws in barrel
  double m_ht_correction_endcap_Ar;    //!< HT correction for Ar straws in barrel
  std::vector<double> m_tot_corrections_barrel_Xe; //!< ToT corrections for 20 ToT bins in Xe barrel straws
  std::vector<double> m_tot_corrections_endcap_Xe; //!< ToT corrections for 20 ToT bins in Xe endcap straws
  std::vector<double> m_tot_corrections_barrel_Ar; //!< ToT corrections for 20 ToT bins in Ar barrel straws
  std::vector<double> m_tot_corrections_endcap_Ar; //!< ToT corrections for 20 ToT bins in Ar endcap straws
};

inline bool TRT_DriftFunctionTool::isValidTime(double drifttime) const
{ return (drifttime>-10. && drifttime<75.); }

inline bool TRT_DriftFunctionTool::isTestBeamData() const
{ return (m_isdata && m_istestbeam); }

inline double TRT_DriftFunctionTool::rawTime(int tdcvalue) const
{
   return  (m_isdata && m_istestbeam) ?
    (tdcvalue*m_drifttimeperhalfbin + m_drifttimeperhalfbin/2.) :
    (tdcvalue*m_drifttimeperbin + m_drifttimeperbin/2.) ;
}


#endif  // TRT_DRIFTFUNCTIONTOOL_H

