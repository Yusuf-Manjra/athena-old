/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1PRDSystems/PRDCollHandle_MDT.h"
#include "VP1PRDSystems/PRDHandle_MDT.h"
#include "VP1PRDSystems/PRDSystemController.h"
#include "VP1PRDSystems/TouchedMuonChamberHelper.h"

#include "VP1Base/IVP13DSystem.h"
#include <QColor>

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonPrepRawData/MdtPrepData.h"

//____________________________________________________________________
QStringList PRDCollHandle_MDT::availableCollections(IVP1System*sys)
{
  return VP1JobConfigInfo::hasMuonGeometry() ? VP1SGContentsHelper(sys).getKeys<Muon::MdtPrepDataContainer>() : QStringList();
}

//____________________________________________________________________
class PRDCollHandle_MDT::Imp {
public:
  PRDCollHandle_MDT * theclass = nullptr;
  std::map<const MuonGM::MuonStation*,unsigned> mdt2stationcounter;
  unsigned minHitsPerStation = 0U;
  VP1Interval allowedADCValues;
  bool excludeMaskedHits = false;
  bool onlyShowActive = false;
  QString status;
  bool projectionsEnabled = false;
  int appropriateprojection = 0;
  void updateProjectionFlag();
};

//____________________________________________________________________
PRDCollHandle_MDT::PRDCollHandle_MDT(PRDSysCommonData * cd,const QString& key)
  : PRDCollHandleBase(PRDDetType::MDT,cd,key), m_d(new Imp),
  m_highLightMasked(false), m_highLightADCBelow(-1), m_projection(PRDCollHandle_MDT::NONE)
{
  m_d->theclass = this;
  m_d->minHitsPerStation = 0;//If the intention is no cut, then put to 0 instead of 1.
  m_d->allowedADCValues = VP1Interval();
  m_d->excludeMaskedHits = true;
  m_d->onlyShowActive = true;
  m_d->projectionsEnabled = false;
  m_d->appropriateprojection = 0;//none
}

//____________________________________________________________________
PRDCollHandle_MDT::~PRDCollHandle_MDT()
{
  delete m_d;
}

//____________________________________________________________________
QColor PRDCollHandle_MDT::defaultColor() const
{
  return QColor::fromRgbF( 1.0, 0.666667, 0.0);//orange
}

//____________________________________________________________________
PRDHandleBase* PRDCollHandle_MDT::addPRD( const Trk::PrepRawData * prd )
{
  assert(dynamic_cast<const Muon::MdtPrepData*>(prd));
  return new PRDHandle_MDT(this,static_cast<const Muon::MdtPrepData*>(prd));
}

//____________________________________________________________________
void PRDCollHandle_MDT::postLoadInitialisation()
{
  std::vector<PRDHandleBase*>::iterator it(getPrdHandles().begin()),itE(getPrdHandles().end());
  std::map<const MuonGM::MuonStation*,unsigned>::iterator itStation;
  for (;it!=itE;++it) {
    itStation = m_d->mdt2stationcounter.find(static_cast<PRDHandle_MDT*>(*it)->driftCircle()->detectorElement()->parentMuonStation());
    if (itStation!=m_d->mdt2stationcounter.end())
      ++(itStation->second);
    else
      m_d->mdt2stationcounter[static_cast<PRDHandle_MDT*>(*it)->driftCircle()->detectorElement()->parentMuonStation()]=1;
  }
}

//____________________________________________________________________
void PRDCollHandle_MDT::eraseEventDataSpecific()
{
  m_d->mdt2stationcounter.clear();
}

//____________________________________________________________________
bool PRDCollHandle_MDT::cut(PRDHandleBase*handlebase)
{
  
  PRDHandle_MDT * handle = static_cast<PRDHandle_MDT*>(handlebase);
  assert(handle);

  if (m_d->excludeMaskedHits) {
    // messageVerbose(QString("Handle status = ")+handle->driftCircleStatus()+QString(", GUI: ")+common()->controller()->mdt_cutMdtDriftCircleStatus() );
    if(!(handle->driftCircleStatus()==common()->controller()->mdt_cutMdtDriftCircleStatus()))
      return false;
  }

  if (!m_d->allowedADCValues.isAllR()) {
    if (!m_d->allowedADCValues.contains(handle->ADC()))
      return false;
  }

  if (m_d->minHitsPerStation) {
    assert(m_d->mdt2stationcounter.find(handle->driftCircle()->detectorElement()->parentMuonStation())!=m_d->mdt2stationcounter.end());
    if (m_d->mdt2stationcounter[handle->driftCircle()->detectorElement()->parentMuonStation()]<m_d->minHitsPerStation)
      return false;
  }

  // messageVerbose("PRDCollHandle_MDT::cut: ");
  if (m_d->onlyShowActive) {
    if (!common()->touchedMuonChamberHelper()->isTouchedByTrack(handle->parentMuonChamberPV()))
      return false;
  }

  return true;
}

//____________________________________________________________________
void PRDCollHandle_MDT::setupSettingsFromControllerSpecific(PRDSystemController*controller)
{
  connect(controller,SIGNAL(mdtMinNHitsPerStationChanged(unsigned)),this,SLOT(setMinNHitsPerStation(unsigned)));
  setMinNHitsPerStation(controller->mdtMinNHitsPerStation());

  connect(controller,SIGNAL(highLightMDTHitsByMaskChanged(bool)),this,SLOT(setHighLightByMask(bool)));
  setHighLightByMask(controller->highLightMDTHitsByMask());

  connect(controller,SIGNAL(highLightMDTHitsByUpperADCBoundChanged(int)),this,SLOT(setHighLightByUpperADCBound(int)));
  setHighLightByUpperADCBound(controller->highLightMDTHitsByUpperADCBound());

  connect(controller,SIGNAL(mdtAllowedADCValuesChanged(VP1Interval)),this,SLOT(setAllowedADCValues(VP1Interval)));
  setAllowedADCValues(controller->mdtAllowedADCValues());

  connect(controller,SIGNAL(mdtExcludeMaskedHitsChanged(bool)),this,SLOT(setExcludeMaskedHits(bool)));
  setExcludeMaskedHits(controller->mdtExcludeMaskedHits());

  connect(controller,SIGNAL(mdt_cutMdtDriftCircleStatusChanged(QString)),this,SLOT(setStatus(QString)));
  setStatus(controller->mdt_cutMdtDriftCircleStatus());

  connect(controller,SIGNAL(projectMDTHitsChanged(bool)),this,SLOT(setEnableProjections(bool)));
  setEnableProjections(controller->projectMDTHits());

  connect(controller,SIGNAL(limitToActiveChambersChanged(bool)),this,SLOT(setLimitToActiveChambers(bool)));
  setLimitToActiveChambers(controller->limitToActiveChambers());
  
  connect(common()->touchedMuonChamberHelper(),SIGNAL(muonChambersTouchedByTracksChanged(void)),this,SLOT(muonChambersTouchedByTracksChanged(void)));  
}

//____________________________________________________________________
void PRDCollHandle_MDT::setMinNHitsPerStation(unsigned minnhits)
{
  if (minnhits==1)
    minnhits = 0;//Since 0 and 1 gives same result, we map all 1's to 0's to avoid unnecessary cut rechecks.
  if (m_d->minHitsPerStation==minnhits)
    return;
  bool cut_relaxed = minnhits < m_d->minHitsPerStation;
  m_d->minHitsPerStation=minnhits;
  if (cut_relaxed)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

//____________________________________________________________________
void PRDCollHandle_MDT::setAllowedADCValues(const VP1Interval& newinterval)
{
  if (m_d->allowedADCValues==newinterval)
    return;
  VP1Interval old(m_d->allowedADCValues);
  m_d->allowedADCValues = newinterval;
  if (newinterval.contains(old)) {
    recheckCutStatusOfAllNotVisibleHandles();//cut relaxed
  } else {
    if (old.contains(newinterval))
      recheckCutStatusOfAllVisibleHandles();//cut tightened
    else
      recheckCutStatusOfAllHandles();
  }
}

//____________________________________________________________________
void PRDCollHandle_MDT::setExcludeMaskedHits(bool b)
{
  if (m_d->excludeMaskedHits==b)
    return;
  bool cut_relaxed = !b;
  m_d->excludeMaskedHits=b;
  if (cut_relaxed)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

//____________________________________________________________________
void PRDCollHandle_MDT::setStatus(const QString& b)
{
  // messageVerbose(QString("SetStatus: ")+b);
  if (m_d->status==b)
    return;
  m_d->status=b;
  recheckCutStatusOfAllHandles();
}

//____________________________________________________________________
void PRDCollHandle_MDT::setHighLightByMask(bool b)
{
  if (m_highLightMasked==b)
    return;
  m_highLightMasked=b;

  //Fixme: check PRDCollHandle::hasCustomHighlighting() before proceeding to loop here?.

  //call updateMaterial on all handles which are masked.
  std::vector<PRDHandleBase*>::iterator it(getPrdHandles().begin()),itE(getPrdHandles().end());
  for (;it!=itE;++it) {
    if (static_cast<PRDHandle_MDT*>(*it)-> masked())
      (*it)->updateMaterial();
  }
}

//____________________________________________________________________
void PRDCollHandle_MDT::setHighLightByUpperADCBound(int bound)
{
  if (bound<-1)
    bound = -1;//Since all negative gives same result, we map all those to -1 to avoid unnecessary rechecks.
  if (m_highLightADCBelow==bound)
    return;

  int low=std::min(m_highLightADCBelow,bound);
  int high=std::max(m_highLightADCBelow,bound);
  m_highLightADCBelow=bound;

  //Fixme: check PRDCollHandle::hasCustomHighlighting() before proceeding to loop here?.

  //call updateMaterial on all handles which are between the former and present values of the bound.
  std::vector<PRDHandleBase*>::iterator it(getPrdHandles().begin()),itE(getPrdHandles().end());
  int adc;
  for (;it!=itE;++it) {
    adc = static_cast<PRDHandle_MDT*>(*it)->ADC();
    if (adc>=low&&adc<=high)
      (*it)->updateMaterial();
  }
}

//____________________________________________________________________
void PRDCollHandle_MDT::setEnableProjections( bool b )
{
  if (m_d->projectionsEnabled==b)
    return;
  m_d->projectionsEnabled=b;
  m_d->updateProjectionFlag();
}

//____________________________________________________________________
void PRDCollHandle_MDT::setAppropriateProjection( int i ) {
  if (m_d->appropriateprojection==i)
    return;
  m_d->appropriateprojection=i;
  m_d->updateProjectionFlag();
}

//____________________________________________________________________
void PRDCollHandle_MDT::setLimitToActiveChambers(bool l)
{
  messageVerbose("PRDCollHandle_MDT::setLimitToActiveChambers => "+str(l)+" current="+str(m_d->onlyShowActive));

  if (m_d->onlyShowActive==l)
    return;
  bool cut_relaxed=(m_d->onlyShowActive);
  m_d->onlyShowActive=l;
  if (cut_relaxed)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void PRDCollHandle_MDT::muonChambersTouchedByTracksChanged(void)
{
  messageVerbose("PRDCollHandle_MDT::muonChambersTouchedByTracksChanged");
  recheckCutStatusOfAllHandles();
}


//____________________________________________________________________
void PRDCollHandle_MDT::Imp::updateProjectionFlag() {
  PROJECTION projflag(PRDCollHandle_MDT::NONE);
  if (projectionsEnabled) {
    if (appropriateprojection==1)
      projflag = PRDCollHandle_MDT::TOTUBES;
    else if (appropriateprojection==2)
      projflag = PRDCollHandle_MDT::TOCHAMBERS;
  }

  if (theclass->m_projection!=projflag) {
    theclass->m_projection = projflag;
    theclass->common()->system()->deselectAll();
    theclass->largeChangesBegin();
    std::vector<PRDHandleBase*>::iterator it(theclass->getPrdHandles().begin()),itE(theclass->getPrdHandles().end());
    for (;it!=itE;++it)
      (*it)->update3DObjects();
    theclass->largeChangesEnd();
  }
}
