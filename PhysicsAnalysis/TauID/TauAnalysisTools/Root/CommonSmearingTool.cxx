/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "PathResolver/PathResolver.h"

// local include(s)
#include "TauAnalysisTools/CommonSmearingTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"

// ROOT include(s)
#include "TROOT.h"
#include "TF1.h"
#include "TClass.h"
#include "TKey.h"

// tauRecTools include(s)
#include "tauRecTools/TauCombinedTES.h"

using namespace TauAnalysisTools;
/*
  This tool acts as a common tool to apply tau energy smearing and
  uncertainties. By default, only nominal smearings without systematic
  variations are applied. Unavailable systematic variations are ignored, meaning
  that the tool only returns the nominal value. In case the one available
  systematic is requested, the smeared scale factor is computed as:
    - pTsmearing = pTsmearing_nominal +/- n * uncertainty

  where n is in general 1 (representing a 1 sigma smearing), but can be any
  arbitrary value. In case multiple systematic variations are passed they are
  added in quadrature. Note that it's currently only supported if all are up or
  down systematics.

  The tool reads in root files including TH2 histograms which need to fulfill a
  predefined structure:

  nominal smearing:
    - sf_<workingpoint>_<prongness>p
  uncertainties:
    - <NP>_<up/down>_<workingpoint>_<prongness>p (for asymmetric uncertainties)
    - <NP>_<workingpoint>_<prongness>p (for symmetric uncertainties)

  where the <workingpoint> (e.g. loose/medium/tight) fields may be
  optional. <prongness> represents either 1 or 3, whereas 3 is currently used
  for multiprong in general. The <NP> fields are names for the type of nuisance
  parameter (e.g. STAT or SYST), note the tool decides whether the NP is a
  recommended or only an available systematic based on the first character:
    - uppercase -> recommended
    - lowercase -> available
  This magic happens here:
    - CommonSmearingTool::generateSystematicSets()

  In addition the root input file can also contain objects of type TF1 that can
  be used to provide kind of unbinned smearings or systematics. Currently there
  is no usecase for tau energy smearing

  The files may also include TNamed objects which is used to define how x and
  y-axes should be treated. By default the x-axis is given in units of tau-pT in
  GeV and the y-axis is given as tau-eta. If there is for example a TNamed
  object with name "Yaxis" and title "|eta|" the y-axis is treated in units of
  absolute tau eta. All this is done in:
    - void CommonSmearingTool::ReadInputs(TFile* fFile)

  Other tools for scale factors may build up on this tool and overwrite or add
  particular functionality.
*/

//______________________________________________________________________________
CommonSmearingTool::CommonSmearingTool(const std::string& sName)
  : asg::AsgMetadataTool( sName )
  , m_sSystematicSet(nullptr)
  , m_fX(&finalTauPt)
  , m_fY(&finalTauEta)
  , m_bIsData(false)
  , m_bIsConfigured(false)
  , m_tTauCombinedTES("TauCombinedTES", this)
  , m_eCheckTruth(TauAnalysisTools::Unknown)
{
  declareProperty("InputFilePath",           m_sInputFilePath           = "" );
  declareProperty("SkipTruthMatchCheck",     m_bSkipTruthMatchCheck     = false );
  declareProperty("ApplyFading",             m_bApplyFading             = true );
  declareProperty("ApplyMVATESQualityCheck", m_bApplyMVATESQualityCheck = false );
  declareProperty("ApplyInsituCorrection",   m_bApplyInsituCorrection   = true );
}

/*
  need to clear the map of histograms cause we have the ownership, not ROOT
*/
CommonSmearingTool::~CommonSmearingTool()
{
  for (auto& mEntry : m_mSF)
    delete mEntry.second;
}

/*
  - Find the root files with smearing inputs on cvmfs using PathResolver
    (more info here:
    https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/PathResolver)
  - Call further functions to process and define NP strings and so on
  - Configure to provide nominal smearings by default
*/
StatusCode CommonSmearingTool::initialize()
{
  ATH_MSG_INFO( "Initializing CommonSmearingTool" );

  // FIXME: do we expect initialize() to be called several times?
  // only read in histograms once
  if (m_mSF.empty())
  {
    std::string sInputFilePath = PathResolverFindCalibFile(m_sInputFilePath);
    std::unique_ptr<TFile> fSF( TFile::Open(sInputFilePath.c_str()) );
    if(fSF == nullptr) {
      ATH_MSG_FATAL("Could not open file " << sInputFilePath.c_str());
      return StatusCode::FAILURE;
    }
    ReadInputs(fSF.get(), m_mSF);
    fSF->Close();
  }

  generateSystematicSets();

  // load empty systematic variation by default
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS )
    return StatusCode::FAILURE;

  // TauCombinedTES tool must be set up when checking compatibility between calo TES and MVA TES
  if (m_bApplyMVATESQualityCheck) 
  {
    ATH_CHECK(ASG_MAKE_ANA_TOOL(m_tTauCombinedTES, TauCombinedTES));
    ATH_CHECK(m_tTauCombinedTES.setProperty("WeightFileName", "CombinedTES_R22_Round2.5_v2.root"));
    ATH_CHECK(m_tTauCombinedTES.initialize());
  }

  return StatusCode::SUCCESS;
}

/*
  Retrieve the smearing value and if requested the values for the NP's and add
  this stuff in quadrature. Finally apply the correction to the tau pt of the
  non-const tau.
*/
//______________________________________________________________________________
CP::CorrectionCode CommonSmearingTool::applyCorrection( xAOD::TauJet& xTau ) const
{
  // optional consistency check between calo-only pt ("ptTauEnergyScale") and MVA pt ("ptFinalCalib" i.e. "pt", MVA TES is the default calibration)
  // not recommended until validated in R22: MVA TES always has better resolution than calo-only TES for true taus
  // in practice this check mostly discards muons faking taus with large track momentum but little energy deposit in the calorimeter:
  // when enforcing calo-only pt, the muon will likely fail the tau pt cut applied by TauSelectionTool

  // WARNING: overwriting ptFinalCalib would lead to irreproducibilities upon re-calibration (re-apply in-situ TES on already-calibrated PHYSLITE)
  if (m_bApplyMVATESQualityCheck) {
    bool useCaloPt = false;     
    if(xTau.isAvailable<float>("ptTauEnergyScale")) {
      const auto combinedTEStool = dynamic_cast<const TauCombinedTES*>(m_tTauCombinedTES.get());
      useCaloPt = combinedTEStool->getUseCaloPtFlag(xTau);	
      if (useCaloPt) {
	// only override pt, keep eta and phi from substructure as it has likely better angular resolution than calo-only
	ATH_MSG_DEBUG("overriding MVA pt with calo pt");
	xTau.setP4(xAOD::TauJetParameters::FinalCalib, xTau.ptTauEnergyScale(), xTau.eta(), xTau.phi(), xTau.m()); 
	xTau.setP4(xTau.ptTauEnergyScale(), xTau.eta(), xTau.phi(), xTau.m()); 
      }
    }
    static const SG::AuxElement::Accessor<char> accUseCaloPt("useCaloPt");
    accUseCaloPt(xTau) = char(useCaloPt);
  }

  // step out here if we run on data
  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  // check which true state is requested
  if (!m_bSkipTruthMatchCheck and getTruthParticleType(xTau) != m_eCheckTruth) {
    return CP::CorrectionCode::Ok;
  }

  // get prong extension for histogram name
  std::string sProng = ConvertProngToString(xTau.nTracks());

  double dCorrection = 1.;
  CP::CorrectionCode tmpCorrectionCode;
  if (m_bApplyInsituCorrection)
  {
    // get standard scale factor
    tmpCorrectionCode = getValue("sf"+sProng,
				 xTau,
				 dCorrection);
    // return correction code if histogram is not available
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
  }

  // skip further process if systematic set is empty
  if (!m_sSystematicSet->empty())
  {
    // get uncertainties summed in quadrature
    double dTotalSystematic2 = 0.;
    double dDirection = 0.;
    for (auto& syst : *m_sSystematicSet)
    {
      // check if systematic is available
      auto it = m_mSystematicsHistNames.find(syst.basename());

      // get uncertainty value
      double dUncertaintySyst = 0.;
      tmpCorrectionCode = getValue(it->second+sProng,
                                   xTau,
                                   dUncertaintySyst);
      // return correction code if histogram is not available
      if (tmpCorrectionCode != CP::CorrectionCode::Ok)
        return tmpCorrectionCode;

      // needed for up/down decision
      dDirection = syst.parameter();

      // scale uncertainty with direction, i.e. +/- n*sigma
      dUncertaintySyst *= dDirection;

      // square uncertainty and add to total uncertainty
      dTotalSystematic2 += dUncertaintySyst * dUncertaintySyst;
    }

    // now use dDirection to use up/down uncertainty
    dDirection = (dDirection > 0.) ? 1. : -1.;

    // finally apply uncertainty (eff * ( 1 +/- \sum  )
    dCorrection *= 1 + dDirection * std::sqrt(dTotalSystematic2);
  }

  // finally apply correction
  // in-situ TES is applied w.r.t. ptFinalCalib, use explicit calibration for pt to avoid irreproducibility upon re-calibration (PHYSLITE)
  // not required for eta/phi/m that we don't correct (only ptFinalCalib and etaFinalCalib are stored in DAODs)
  xTau.setP4( xTau.ptFinalCalib() * dCorrection,
              xTau.eta(), xTau.phi(), xTau.m());
  return CP::CorrectionCode::Ok;
}

/*
  Create a non-const copy of the passed const xTau object and apply the
  correction to the non-const copy.
 */
//______________________________________________________________________________
CP::CorrectionCode CommonSmearingTool::correctedCopy( const xAOD::TauJet& xTau,
    xAOD::TauJet*& xTauCopy ) const
{

  // A sanity check:
  if( xTauCopy )
  {
    ATH_MSG_WARNING( "Non-null pointer received. "
                     "There's a possible memory leak!" );
  }

  // Create a new object:
  xTauCopy = new xAOD::TauJet();
  xTauCopy->makePrivateStore( xTau );

  // Use the other function to modify this object:
  return applyCorrection( *xTauCopy );
}

/*
  standard check if a systematic is available
*/
//______________________________________________________________________________
bool CommonSmearingTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  const CP::SystematicSet& sys = affectingSystematics();
  return sys.find(systematic) != sys.end();
}

/*
  standard way to return systematics that are available (including recommended
  systematics)
*/
//______________________________________________________________________________
CP::SystematicSet CommonSmearingTool::affectingSystematics() const
{
  return m_sAffectingSystematics;
}

/*
  standard way to return systematics that are recommended
*/
//______________________________________________________________________________
CP::SystematicSet CommonSmearingTool::recommendedSystematics() const
{
  return m_sRecommendedSystematics;
}

/*
  Configure the tool to use a systematic variation for further usage, until the
  tool is reconfigured with this function. The passed systematic set is checked
  for sanity:
    - unsupported systematics are skipped
    - only combinations of up or down supported systematics is allowed
    - don't mix recommended systematics with other available systematics, cause
      sometimes recommended are a quadratic sum of the other variations,
      e.g. TOTAL=(SYST^2 + STAT^2)^0.5
*/
//______________________________________________________________________________
StatusCode CommonSmearingTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet )
{
  // first check if we already know this systematic configuration
  auto itSystematicSet = m_mSystematicSets.find(sSystematicSet);
  if (itSystematicSet != m_mSystematicSets.end())
  {
    m_sSystematicSet = &itSystematicSet->first;
    return StatusCode::SUCCESS;
  }

  // sanity checks if systematic set is supported
  double dDirection = 0.;
  CP::SystematicSet sSystematicSetAvailable;
  for (auto& sSyst : sSystematicSet)
  {
    // check if systematic is available
    auto it = m_mSystematicsHistNames.find(sSyst.basename());
    if (it == m_mSystematicsHistNames.end())
    {
      ATH_MSG_VERBOSE("unsupported systematic variation: "<< sSyst.basename()<<"; skipping this one");
      continue;
    }

    if (sSyst.parameter() * dDirection < 0)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should either use only \"UP\" or only \"DOWN\" systematics in one set!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }
    dDirection = sSyst.parameter();

    if ((m_sRecommendedSystematics.find(sSyst.basename()) != m_sRecommendedSystematics.end()) and sSystematicSet.size() > 1)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should not combine \"TAUS_{TRUE|FAKE}_SME_TOTAL\" with other systematic variations!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }

    // finally add the systematic to the set of systematics to process
    sSystematicSetAvailable.insert(sSyst);
  }

  // store this calibration for future use, and make it current
  m_sSystematicSet = &m_mSystematicSets.insert(std::pair<CP::SystematicSet,std::string>(sSystematicSetAvailable, sSystematicSet.name())).first->first;

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
/*
  Executed at the beginning of each event, checks if the tool is used on data or MC.
  This tool is mostly for MC (in-situ TES correction).
  But the TES compatibility requirement is applied to both data and MC (when ApplyMVATESQualityCheck=true).
*/
//______________________________________________________________________________
StatusCode CommonSmearingTool::beginEvent()
{
  if (m_bIsConfigured)
    return StatusCode::SUCCESS;

  const xAOD::EventInfo* xEventInfo = nullptr;
  ATH_CHECK(evtStore()->retrieve(xEventInfo,"EventInfo"));
  m_bIsData = !(xEventInfo->eventType( xAOD::EventInfo::IS_SIMULATION));
  m_bIsConfigured = true;

  return StatusCode::SUCCESS;
}

/*
  prongness converter, note that it returns "_3p" for all values, except
  fProngness==1, i.e. for 0, 2, 3, 4, 5...
 */
//______________________________________________________________________________
std::string CommonSmearingTool::ConvertProngToString(const int fProngness) const
{
  std::string prong = "";
  if (fProngness == 0)
    ATH_MSG_DEBUG("passed tau with 0 tracks, which is not supported, taking multiprong SF for now");
  fProngness == 1 ? prong = "_1p" : prong = "_3p";
  return prong;
}

//______________________________________________________________________________
template<class T>
void CommonSmearingTool::ReadInputs(TFile* fFile, std::map<std::string, T>& mMap)
{
  // initialize function pointer
  m_fX = &finalTauPt;
  m_fY = &finalTauEta;

  TKey *kKey;
  TIter itNext(fFile->GetListOfKeys());
  while ((kKey = (TKey*)itNext()))
  {
    TClass *cClass = gROOT->GetClass(kKey->GetClassName());

    // parse file content for objects of type TNamed, check their title for
    // known strings and reset funtion pointer
    std::string sKeyName = kKey->GetName();
    if (sKeyName == "Xaxis")
    {
      TNamed* tObj = (TNamed*)kKey->ReadObj();
      std::string sTitle = tObj->GetTitle();
      delete tObj;
      if (sTitle == "P" || sTitle == "PFinalCalib")
      {
        m_fX = &finalTauP;
        ATH_MSG_DEBUG("using full momentum for x-axis");
      }
    }
    if (sKeyName == "Yaxis")
    {
      TNamed* tObj = (TNamed*)kKey->ReadObj();
      std::string sTitle = tObj->GetTitle();
      delete tObj;
      if (sTitle == "track-eta")
      {
        m_fY = &tauLeadTrackEta;
        ATH_MSG_DEBUG("using leading track eta for y-axis");
      }
      else if (sTitle == "|eta|")
      {
        m_fY = &finalTauAbsEta;
        ATH_MSG_DEBUG("using absolute tau eta for y-axis");
      }
    }
    if (!cClass->InheritsFrom("TH1"))
      continue;
    T tObj = (T)kKey->ReadObj();
    tObj->SetDirectory(0);
    mMap[sKeyName] = tObj;
  }
  ATH_MSG_INFO("data loaded from " << fFile->GetName());
}

//______________________________________________________________________________
void CommonSmearingTool::generateSystematicSets()
{
  std::vector<std::string> vInputFilePath;
  split(m_sInputFilePath,'/',vInputFilePath);
  std::string sInputFileName = vInputFilePath.back();

  // creation of basic string for all NPs, e.g. "TAUS_TRUEHADTAU_SME_TES_"
  std::vector<std::string> vSplitInputFilePath = {};
  split(sInputFileName,'_',vSplitInputFilePath);
  std::string sEfficiencyType = vSplitInputFilePath.at(0);
  std::string sTruthType = vSplitInputFilePath.at(1);
  std::transform(sEfficiencyType.begin(), sEfficiencyType.end(), sEfficiencyType.begin(), toupper);
  std::transform(sTruthType.begin(), sTruthType.end(), sTruthType.begin(), toupper);
  std::string sSystematicBaseString = "TAUS_" + sTruthType + "_SME_" + sEfficiencyType + "_";

  // set truth type to check for in truth matching
  if (sTruthType=="TRUEHADTAU") m_eCheckTruth = TauAnalysisTools::TruthHadronicTau;
  if (sTruthType=="TRUEELECTRON") m_eCheckTruth = TauAnalysisTools::TruthElectron;
  if (sTruthType=="TRUEMUON") m_eCheckTruth = TauAnalysisTools::TruthMuon;
  if (sTruthType=="TRUEJET") m_eCheckTruth = TauAnalysisTools::TruthJet;

  for (auto& mSF : m_mSF)
  {
    // parse for nuisance parameter in histogram name
    std::vector<std::string> vSplitNP = {};
    split(mSF.first,'_',vSplitNP);
    std::string sNP;
    std::string sNPUppercase;
    if (vSplitNP.size() > 2)
    {
      sNP = vSplitNP.at(0)+'_'+vSplitNP.at(1);
      sNPUppercase = vSplitNP.at(0) + '_' + vSplitNP.at(1);
    } else {
      sNP = vSplitNP.at(0);
      sNPUppercase = vSplitNP.at(0);
    }

    // skip nominal scale factors
    if (sNP == "sf") continue;
    // skip if non 1p histogram to avoid duplications (TODO: come up with a better solution)
    if (mSF.first.find("_1p") == std::string::npos) continue;

    // test if NP starts with a capital letter indicating that this should be recommended
    bool bIsRecommended = false;
    if (isupper(sNP.at(0)))
      bIsRecommended = true;

    // make sNP uppercase and build final NP entry name
    std::transform(sNPUppercase.begin(), sNPUppercase.end(), sNPUppercase.begin(), toupper);
    std::string sSystematicString = sSystematicBaseString + sNPUppercase;

    // add all found systematics to the AffectingSystematics
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    // only add found uppercase systematics to the RecommendedSystematics
    if (bIsRecommended)
    {
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    }

    ATH_MSG_DEBUG("connected histogram base name " << sNP << " with systematic " << sSystematicString);
    m_mSystematicsHistNames.insert({sSystematicString,sNP});
  }
}

//______________________________________________________________________________
CP::CorrectionCode CommonSmearingTool::getValue(const std::string& sHistName,
    const xAOD::TauJet& xTau,
    double& dEfficiencyScaleFactor) const
{
  TH1* hHist = m_mSF.at(sHistName);
  if (hHist == nullptr)
  {
    ATH_MSG_ERROR("Histogram with name " << sHistName << " was not found in input file.");
    return CP::CorrectionCode::Error;
  }

  double dPt = m_fX(xTau);
  double dEta = m_fY(xTau);

  // protect values from underflow bins
  dPt = std::max(dPt,hHist->GetXaxis()->GetXmin());
  dEta = std::max(dEta,hHist->GetYaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dPt = std::min(dPt,hHist->GetXaxis()->GetXmax() * .999);
  dEta = std::min(dEta,hHist->GetYaxis()->GetXmax() * .999);

  int iBin = hHist->FindFixBin(dPt,dEta);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);

  if (m_bApplyFading)
  {
    std::string sTitle = hHist->GetTitle();
    if (!sTitle.empty())
    {
      TF1 f("",sTitle.c_str(), 0, 1000);
      if (sHistName.find("sf_") != std::string::npos)
        dEfficiencyScaleFactor = (dEfficiencyScaleFactor -1.) *f.Eval(m_fX(xTau)) + 1.;
      else
        dEfficiencyScaleFactor *= f.Eval(m_fX(xTau));
    }
  }
  return CP::CorrectionCode::Ok;
}
