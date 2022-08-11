/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
/**
 *  @file  LArCellDeadOTXCorr.cxx
 *  @brief  CaloCell energy correction tool for missing FEBs
 *
 *
 *  @author  Deepak Kar
 *
 *  @date    08/01/09
 *
 *  @internal
 *     Created :  08/01/09
 * Last update :  07/21/2010 10:48:57 AM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */

//-----------------------
//
// LArCellDeadOTXCorr
//
// CaloCell Energy Correction Tool for Missing FEBs
// by Deepak Kar with help from Guillaume Unal and Damien Prieur
// Version 1, Date Aug 1, 2009
//
//-----------------------



#include "LArCellDeadOTXCorr.h" //Needs to be changed to correct package

#include "CLHEP/Units/SystemOfUnits.h"

#include "StoreGate/DataHandle.h"
#include "StoreGate/ReadHandle.h"

#include "Identifier/IdentifierHash.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloEvent/CaloCell.h"
#include "LArRawEvent/LArFebErrorSummary.h" 
#include "LArIdentifier/LArOnlineID.h" 
#include "CaloTriggerTool/CaloTriggerTowerService.h" 
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "CaloIdentifier/CaloLVL1_ID.h"

#include "TrigT1CaloCondSvc/L1CaloCondSvc.h"
#include "TrigT1CaloCalibConditions/L1CaloPprLutContainer.h"

#include "TH1F.h"
#include "TF1.h"

#include <math.h>
#include <memory>

//From root ... 
#include "HFitInterface.h"
#include "Math/MinimizerOptions.h"
#include "Fit/DataRange.h"

using CLHEP::GeV;


/**
 * @brief Definition of a Landau-Landau function
 */
static double landauLandau(double* x, double* par)
{
	//par[0] : A, maximum value
	//par[1] : t0, maximum position
	//par[2] : sigma1 
	//par[3] : sigma2
	//par[4] : D, undershoot
	
	double value = 0.;
	double arg = 0.;
	if(x[0]<=par[1])
	{
		if (par[2] != 0) arg = (x[0] - par[1])/par[2];
		value = (fabs(arg)<100. ? par[0]*exp(-0.5*(arg + exp(-arg))) : 0.);//set limit to avoid floating point overflow
	}
	else
	{
		if (par[3] != 0) arg = (x[0] - par[1])/par[3];
		value = (fabs(arg)<100. ? (par[0] + par[4]*exp(0.5))*exp(-0.5*(arg + exp(-arg))) - par[4] : -par[4]); 
	}

	return value;
}

const std::map<int,int> LArCellDeadOTXCorr::defineSizeType()
{
	std::map<int,int> m;
	m[-1] = 0;
	m[0]  = 5;//exponential

	return m;
}

const std::map<int, int> LArCellDeadOTXCorr::m_typeSizeMapping = LArCellDeadOTXCorr::defineSizeType();



//-------------------------------------------------------------------------------------------------------
// Constructor



LArCellDeadOTXCorr::LArCellDeadOTXCorr(
		const std::string& type, 
		const std::string& name, 
		const IInterface* parent)
  : base_class(type, name, parent),
    m_caloMgr(nullptr),
    m_lvl1Helper(nullptr),
    m_calo_id(nullptr),
    m_onlineID(nullptr),
    m_TT_ID(nullptr),
    m_l1CondSvc(nullptr),
    m_ttSvc("CaloTriggerTowerService")
{
	declareProperty("triggerTowerLocation", m_TTLocation  = "xAODTriggerTowers");
	declareProperty("triggerNoiseCut", m_triggerNoiseCut);
	declareProperty("useL1CaloDB", m_useL1CaloDBProp = false);
	declareProperty("ignoredTTs", m_ignoredTTs);
	declareProperty("CaloTriggerTowerService",m_ttSvc);

        m_useL1CaloDB = m_useL1CaloDBProp;
}

LArCellDeadOTXCorr::~LArCellDeadOTXCorr() {}

//---------------------------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------
// Initialize
//----------------------------------------------------------------------------------
StatusCode LArCellDeadOTXCorr::initialize()
{
        ATH_MSG_INFO ("Initializing LArCellDeadOTXCorr");

	CHECK( base_class::initialize() );

		
	ATH_CHECK(m_TTLocation.initialize());

	if(m_useL1CaloDB)
	{
                ATH_MSG_INFO ("L1Calo database will be used to get the pedestal values.");
		ATH_CHECK( service("L1CaloCondSvc", m_l1CondSvc) );
	}
	else
                ATH_MSG_INFO ("L1Calo database won't be used. Pedestal values will be constant and equal to 32.");


	ATH_CHECK( m_badFebKey.initialize());
	ATH_CHECK( m_cablingKey.initialize());
	ATH_CHECK( detStore()->retrieve(m_onlineID, "LArOnlineID") );
	ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );
	ATH_CHECK( detStore()->retrieve(m_caloMgr) );
	
	// Use the CaloIdManager to get a pointer to an instance of the CaloLVL1_ID helper
	m_lvl1Helper = m_caloMgr->getLVL1_ID();
	if (!m_lvl1Helper) {
                ATH_MSG_ERROR ("Could not access CaloLVL1_ID helper");
		return StatusCode::FAILURE;
	}

        m_useL1CaloDB = m_useL1CaloDBProp;

	ATH_CHECK( detStore()->retrieve(m_TT_ID) );
	ATH_CHECK( m_ttSvc.retrieve());
	return StatusCode::SUCCESS;
}

//
//  process method
//

StatusCode  LArCellDeadOTXCorr::process (CaloCellContainer* cellCont,
                                         const EventContext& ctx) const
{
        ATH_MSG_DEBUG (" in process...");

        Context myctx = context (ctx);
        CxxUtils::Array<1> etaCalibrationSizes = m_etaCalibrationSizes(myctx);
        CxxUtils::Array<1> etaCalibrations = m_etaCalibrations(myctx);
        CxxUtils::Array<1> energyCalibrationTypes = m_energyCalibrationTypes(myctx);
        CxxUtils::Array<1> energyCalibrations = m_energyCalibrations(myctx);

	SG::ReadCondHandle<LArBadFebCont> badFebHdl{m_badFebKey,ctx};
	const LArBadFebCont* badFebs=*badFebHdl;

	SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey,ctx};
	const LArOnOffIdMapping* cabling=*cablingHdl;

	if (msgLvl(MSG::DEBUG)) {
	  msg(MSG::DEBUG) << " Nb of eta calibration factors found : "<<etaCalibrations.size() << endmsg;
	  for(unsigned int i=0;i<etaCalibrations.size();i++) {
	    msg(MSG::DEBUG) << "calibration["<<i<<"] = "<<etaCalibrations[i] << endmsg;
	  }
	}

	//Retrieve Trigger Towers from SG
	SG::ReadHandle<xAOD::TriggerTowerContainer> storedTTs(m_TTLocation, ctx);
	if(!storedTTs.isValid()) { 
	  ATH_MSG_ERROR("Could not read container " << m_TTLocation.key());
	  return StatusCode::FAILURE;      
	}  


	bool getDBPedestal = m_useL1CaloDB;
	L1CaloPprLutContainer* l1CaloPprLutContainer = 0;
	if(m_useL1CaloDB)
	{
		if(!m_l1CondSvc)
		{
                        ATH_MSG_WARNING ("L1CaloCondSvc is null. Pedestals will be constant and set to 32.");
			getDBPedestal = false;
			m_useL1CaloDB = false;// disable DB 
		}
		else
		{
			StatusCode sc = m_l1CondSvc->retrieve(l1CaloPprLutContainer); 
			if(sc.isFailure() || !l1CaloPprLutContainer) {
                                ATH_MSG_WARNING ("L1CaloCondSvc failed to retrieve l1CaloPprLutContainer. Pedestals will be constant and set to 32.");
				getDBPedestal = false;
				m_useL1CaloDB = false;// disable DB, it won't try to retrieve L1CaloPprLutContainer again  
			}
		}
	}

	const LArBadFebCont::BadChanVec& allMissingFebs=badFebs->fullCont();
	// vector of all missing cells Id
	std::vector<Identifier> cell_array;
	cell_array.reserve(512);

	// To store counted TTs
	std::vector <Identifier> trigtow;
	// to store corrected energy per trigger tower
	std::vector <double> en_array;
	trigtow.reserve(32);
	en_array.reserve(32);

	for (const LArBadFebCont::BadChanEntry& it : allMissingFebs) {
	  const LArBadFeb& febstatus=it.second;
	  if (febstatus.deadReadout()) {
	    const HWIdentifier febId(it.first);
	    //Loop over channels belonging to this FEB
	    const int chansPerFeb=m_onlineID->channelInSlotMax(febId);
	    for (int ch=0; ch<chansPerFeb; ++ch) {
	      const HWIdentifier hwid = m_onlineID->channel_Id(febId, ch);
	      if ( cabling->isOnlineConnected(hwid)) {
		const Identifier id=cabling->cnvToIdentifier(hwid);
		cell_array.push_back(id);
	      }//end is connected
	    }//end loop over channels on a feb
	  }//end if feb is dead
	}//end loop over problematic febs

	//Debug printout:
	//for (auto& c : cell_array)  std::cout << "Missing cell: " << c.get_identifier32().get_compact() << std::endl;

        ATH_MSG_DEBUG (" Number of missing cells " << cell_array.size());

	// nothing to do if no missing Febs
	if (cell_array.size()==0) return StatusCode::SUCCESS;


	// loop over missing cells
	for(unsigned int i=0; i < cell_array.size(); i++)
	{

		Identifier cId = cell_array[i];
		Identifier ttId;


		auto itCellTT = m_cellTTMapping.find(cId);
		if(itCellTT!=m_cellTTMapping.end())
			ttId = itCellTT->second;
		else
		{
			const Identifier layerId(m_ttSvc->whichTTID(cId));
			ttId = m_lvl1Helper->tower_id( m_lvl1Helper->pos_neg_z(layerId), m_lvl1Helper->sampling(layerId), m_lvl1Helper->region(layerId), m_lvl1Helper->eta(layerId), m_lvl1Helper->phi(layerId));
                        ATH_MSG_DEBUG (" cell id, TT ID " <<  m_calo_id->show_to_string(cId) << " " << ttId);
                        ATH_MSG_DEBUG ("  pos_neg, layer, sampling, region,eta,phi " << m_lvl1Helper->pos_neg_z(layerId) << " " <<  m_lvl1Helper->sampling(layerId) << " "
                                       << m_lvl1Helper->region(layerId) << " " << m_lvl1Helper->eta(layerId) << " " << m_lvl1Helper->phi(layerId));
			m_cellTTMapping[cId] = ttId;
		}

		double corr_energy;
		bool counted = false;

		// Check if this TT has already been processed
		for(unsigned int it = 0;  it< trigtow.size();it++){
			if(ttId == trigtow[it]) {
				counted = true;
				corr_energy = en_array[it];
				ATH_MSG_DEBUG (" this trigger tower is already processed : E to assign per missing cell " << corr_energy);
				break;
			}
		}

		if(!counted){
			//save this ttID 
			trigtow.push_back(ttId);

			bool isBad = false;

			L1CaloCoolChannelId coolChannelId = m_ttSvc->createL1CoolChannelId(m_ttSvc->createTTChannelID(ttId));
			unsigned int uiId = coolChannelId.id();
                        for (unsigned int iBad : m_ignoredTTs)
                        {
				if(iBad == uiId)  
				{
					isBad = true;
					break;
				}
			}
			if(isBad)
				corr_energy = 0.;
			else
			{
				// get energy in this TT
				double tt_energy = 0.;
				
				bool ttFound = false;
				unsigned int ttIndex = 0;
				int ttType = -1;
				const xAOD::TriggerTower* tt;

				// we first look if the index and type of this TT are already known and if this
				// index corresponds to the TT we want
				auto itTT = m_idIndexMapping.find(ttId);
				if(itTT!=m_idIndexMapping.end())
				{
					ttIndex = itTT->second.first;
					ttType = itTT->second.second;

					if(ttIndex<storedTTs->size() && (ttType==0 || ttType==1))
					{
						const xAOD::TriggerTower* tmpTT = storedTTs->at(ttIndex);
						double eta = tmpTT->eta();
						double phi = tmpTT->phi();


						int pos_neg_z = this->TTID_pos_neg_z(eta);
						int region = this->TTID_regionIndex(eta);
						int ieta   = this->TTID_etaIndex(eta);
						int iphi   = this->TTID_phiIndex(eta, phi);

						if(ttId == m_lvl1Helper->tower_id(pos_neg_z, ttType, region, ieta, iphi))
						{
							ttFound = true;
							tt = tmpTT;
						}
					}
					else
					{
						ttIndex = 0;
						ttType = -1;
					}

				}


				// if the tt has not been found we perform a sequential search on the TTs container.
				// When found, the tt id and its type and index in the container are cached
				if(!ttFound)
				{

					for(unsigned int iTT=0; iTT<storedTTs->size(); iTT++)
					{ 
						const xAOD::TriggerTower* tmpTT = storedTTs->at(iTT);
						double eta = tmpTT->eta();
						double phi = tmpTT->phi();

						Identifier id, emId, hadId;
						// Temporary, compute id out of the tower position

						/*
						   int pos_neg_z = m_l1CaloTTIdTools->pos_neg_z(eta);
						   int region = m_l1CaloTTIdTools->regionIndex(eta);
						   int ieta   = m_l1CaloTTIdTools->etaIndex(eta);
						   int iphi   = m_l1CaloTTIdTools->phiIndex(eta, phi);
						   */

						int pos_neg_z = this->TTID_pos_neg_z(eta);
						int region = this->TTID_regionIndex(eta);
						int ieta   = this->TTID_etaIndex(eta);
						int iphi   = this->TTID_phiIndex(eta, phi);

						if(ttType!=-1)
						{
							id = m_lvl1Helper->tower_id(pos_neg_z, ttType, region, ieta, iphi);

							if(id==ttId)
							{
								tt = tmpTT;
								ttFound = true;
								m_idIndexMapping[id] = std::pair<unsigned int, int>(iTT,ttType); 
								break;
							}
						}
						else
						{
							emId = m_lvl1Helper->tower_id(pos_neg_z, 0, region, ieta, iphi);
							hadId = m_lvl1Helper->tower_id(pos_neg_z, 1, region, ieta, iphi);

							if(emId==ttId)
							{
								tt = tmpTT;
								ttType = 0;
								ttFound = true;
								m_idIndexMapping[emId] = std::pair<unsigned int, int>(iTT,ttType);
								break;
							}
							if(hadId==ttId)
							{
								tt = tmpTT;
								ttType = 1;
								ttFound = true;
								m_idIndexMapping[hadId] = std::pair<unsigned int, int>(iTT,ttType);
								break;
							}

						}
					}// end loop on TT
				}


				if(ttFound && ttType==0)
				{
					double eta = tt->eta();


					if((fabs(eta)>1.4 && fabs(eta)<1.5) || fabs(eta)>3.2)// for the moment, we don't apply corrections in the crack and FCAL
					{
						tt_energy = 0.;
					}
					else
					{
						unsigned int lutPed = (getDBPedestal ? l1CaloPprLutContainer->pprLut(coolChannelId)->pedValue() : 32);

						tt_energy = getL1Energy(etaCalibrationSizes,
                                                                        etaCalibrations,
                                                                        energyCalibrationTypes,
                                                                        energyCalibrations,
                                                                        tt->adc(), (int)lutPed, eta, 0);

						if(tt_energy>0.)
						{
							double thetaTT = 2.*atan(exp(-1.*eta));
							tt_energy = tt_energy*GeV/sin(thetaTT);// conversion from ET into energy in MeV
						}
						else 
							tt_energy = 0.;

						//tt_energy = tt->emEnergy()*GeV/sin(thetaTT);
					}
				}
				else if (ttFound && ttType==1) 
				{
					double eta = tt->eta();
					unsigned int lutPed = (getDBPedestal ? l1CaloPprLutContainer->pprLut(coolChannelId)->pedValue() : 32);

					tt_energy = getL1Energy(etaCalibrationSizes,
                                                                etaCalibrations,
                                                                energyCalibrationTypes,
                                                                energyCalibrations,
                                                                tt->adc(), (int)lutPed, eta, 1);
					if(tt_energy>0.)
					{
						double thetaTT = 2.*atan(exp(-1.*eta));
						tt_energy = tt_energy*GeV/sin(thetaTT);// conversion from ET into energy in MeV
					}
					else
						tt_energy = 0.;

					//tt_energy = tt->hadEnergy()*GeV/sin(thetaTT);
				}



				if(tt_energy<1e-6)
					corr_energy = 0.;
				else
				{
					// Loop over Cells in this TT
					std::vector<Identifier> cellVec = m_ttSvc->createCellIDvecTT(ttId);
					int TTsize = cellVec.size();

					ATH_MSG_DEBUG (" # of Cells in this TT " <<  TTsize);

					// compute sum of non missing cell energies
					double sum_cell_energy =0.;
					int missing_cells_this_tt=0;
					for (unsigned int iCell=0;iCell<cellVec.size();iCell++){        
						Identifier cid1 = cellVec[iCell];

						IdentifierHash theCellHashID = m_calo_id->calo_cell_hash(cid1);
						int index = cellCont->findIndex(theCellHashID);

						double cell_energy=0.;
						CaloCell_ID::CaloSample	sampling = CaloCell_ID::Unknown;
						if(index!=-1){
							const CaloCell* aCell = cellCont->at(index);
							if (aCell) 
							{
								cell_energy= aCell->energy();
								sampling = aCell->caloDDE()->getSampling();
							}
						}
						bool cell_missing=false;
						for(unsigned int k=0; k < cell_array.size(); k++){
							if(cid1 == cell_array[k]) {
								cell_missing =true;
								break;
							}
						}
						ATH_MSG_DEBUG ("     cell in TT, energy, missing ? " << m_calo_id->show_to_string(cid1) << " " << cell_energy << " " << cell_missing);

						if(sampling!=CaloCell_ID::HEC3)//No HEC3 in TT
						{
							if(!cell_missing){
								sum_cell_energy =  sum_cell_energy + cell_energy;
							}
							else  missing_cells_this_tt++;
						}

					}
					ATH_MSG_DEBUG ("  Number of missing cells in this TT, sum of good cells energy " << missing_cells_this_tt << " " << sum_cell_energy);


                                        if (missing_cells_this_tt > 0)
                                          corr_energy = (tt_energy - sum_cell_energy)/(missing_cells_this_tt);
                                        else
                                          corr_energy = 0;
				}
			}

			en_array.push_back(corr_energy);

		}// Counted 

		// Assign energy
		IdentifierHash theCellHashID = m_calo_id->calo_cell_hash(cId);
		int index = cellCont->findIndex(theCellHashID);

		if(index!=-1){
			CaloCell* thisCell = cellCont->at(index);

			// Assign energy
			if (thisCell ) 
			{
				CaloCell_ID::CaloSample	sampling =  thisCell->caloDDE()->getSampling();
				if(corr_energy>0. && sampling!=CaloCell_ID::HEC3)
				{
					thisCell->setEnergy(corr_energy);
					ATH_MSG_DEBUG ("   assign " << corr_energy<< " to this cell ");
				}
			}
		}


	}//loop over missing cells

	ATH_MSG_DEBUG (" end process");

	return StatusCode::SUCCESS;
}






StatusCode  LArCellDeadOTXCorr::finalize() {
	m_idIndexMapping.clear();
	m_cellTTMapping.clear();

	return StatusCode::SUCCESS;
}


// temporary, until somne dependency problem with trigger code are solved
//   need to duplicate these methoes from L1CaloTTIdTools

const double LArCellDeadOTXCorr::BASEDETA = 0.1;
const double LArCellDeadOTXCorr::BASEDPHI = 0.098175;
const double LArCellDeadOTXCorr::FCALDETA = 0.425;

const double LArCellDeadOTXCorr::ETAMAXREGION0 = 2.5;
const double LArCellDeadOTXCorr::ETAMAXREGION1 = 3.1;
const double LArCellDeadOTXCorr::ETAMAXREGION2 = 3.2;
const double LArCellDeadOTXCorr::ETAMAXREGION3 = 4.9;

const double LArCellDeadOTXCorr::ETAMIN = -4.9; 
const double LArCellDeadOTXCorr::PHIMIN = 0.;


int LArCellDeadOTXCorr::TTID_pos_neg_z(double eta) const {
	return (eta>=0) ? +1 : -1;
}

int LArCellDeadOTXCorr::TTID_etaIndex(double eta) const {
	int etacenter = -1;
	double deta = this->TTID_etaWidth(eta);
	double abseta = fabs(eta);

	if(abseta<ETAMAXREGION0) {
		etacenter = (int) floor(abseta/deta);

	} else if(abseta<ETAMAXREGION1) {
		etacenter = (int) (floor((abseta-ETAMAXREGION0)/deta));

	} else if (abseta<ETAMAXREGION2) {
		etacenter = (int) (floor((abseta-ETAMAXREGION1)/deta));

	} else if (abseta<ETAMAXREGION3){
		etacenter = (int) (floor((abseta-ETAMAXREGION2)/deta));
	}
	return etacenter;
}

int LArCellDeadOTXCorr::TTID_phiIndex(double eta, double phi) const {
	double dphi = this->TTID_phiWidth(eta);
	int phicenter = (int) floor(phi/dphi);
	return phicenter;
}

int LArCellDeadOTXCorr::TTID_regionIndex(double eta) const {
	int region = -1;
	double abseta = fabs(eta);

	if(abseta<ETAMAXREGION0) {
		region = 0;

	} else if(abseta<ETAMAXREGION1) {
		region = 1;

	} else if (abseta<ETAMAXREGION2) {
		region = 2;

	} else if (abseta<ETAMAXREGION3) {
		region = 3;

	}
	return region;
}
double LArCellDeadOTXCorr::TTID_etaWidth(double eta) const { 
	double abseta = fabs(eta);
	if(abseta<ETAMAXREGION0) {
		return BASEDETA;

	} else if(abseta<ETAMAXREGION1) {
		return BASEDETA*2.;

	} else if (abseta<ETAMAXREGION2) {
		return BASEDETA;

	} else {
		return FCALDETA;
	}
}

double LArCellDeadOTXCorr::TTID_phiWidth(double eta) const { 
	double abseta = fabs(eta);
	if(abseta<ETAMAXREGION0) {
		return BASEDPHI;

	} else if(abseta<ETAMAXREGION1) {
		return BASEDPHI*2.;

	} else if (abseta<ETAMAXREGION2) {
		return BASEDPHI*2.;

	} else {
		return BASEDPHI*4.;
	}
}


//---- Functions used to calculate a parabola
double LArCellDeadOTXCorr::getA(double x1, double y1,
                                double x2, double y2,
                                double x3, double y3) const
{
	double n = y3-y1 + (x1-x3)*(y2-y1)/(x2-x1);
	double d = (x3-x1)*(x3-x2);
	return (n/d);
}

double LArCellDeadOTXCorr::getB(double a,
                                double x1, double y1,
                                double x2, double y2) const
{
	return (y2-y1 - a*(x2*x2-x1*x1))/(x2-x1);
}
double LArCellDeadOTXCorr::getC(double a, double b,
                                double x1, double y1) const
{
	return (y1 - a*x1*x1 - b*x1);
}



void
LArCellDeadOTXCorr::getInitialFitParameters(const std::vector<uint_least16_t> & ADCsamples,
                                            double & max,
                                            double& maxPos,
                                            unsigned int& TTADCMaxIndex) const
{
	max = -9999.;
	maxPos = -1.;
	TTADCMaxIndex = -1;
	uint_least16_t maxADC = 0;
	for(unsigned int ADCi = 0;ADCi<ADCsamples.size();ADCi++)
	{
		if(ADCsamples[ADCi]>maxADC)
		{
			maxADC = ADCsamples[ADCi];
			TTADCMaxIndex = ADCi;
		}
	}

	if(TTADCMaxIndex>=1 && TTADCMaxIndex<ADCsamples.size()-1)
	{
		double x1 =  (double)(TTADCMaxIndex-1);
		double y1 = (double)(ADCsamples[TTADCMaxIndex-1]);
		double x2 =  (double)TTADCMaxIndex;
		double y2 = (double)(ADCsamples[TTADCMaxIndex]);
		double x3 =  (double)(TTADCMaxIndex+1);
		double y3 = (double)(ADCsamples[TTADCMaxIndex+1]);
		double a = getA(x1,y1,x2,y2,x3,y3);
		if(a<0.)
		{
			double b = getB(a,x1,y1,x2,y2);
			double c = getC(a,b,x1,y1);

			maxPos = -1.*b/(2.*a);
			max = b*b/(-4.*a)+c;
		}
	}
}


double LArCellDeadOTXCorr::getL1Energy(const CxxUtils::Array<1>& etaCalibrationSizes,
                                       const CxxUtils::Array<1>& etaCalibrations,
                                       const CxxUtils::Array<1>& energyCalibrationTypes,
                                       const CxxUtils::Array<1>& energyCalibrations,
                                       const std::vector<uint_least16_t> & ADCsamples, int pedestal, double eta, int type) const
{
	double energy = 0;
	int nbSamples = ADCsamples.size();
	double max0, maxPos0;
	unsigned int maxIndex;
	getInitialFitParameters(ADCsamples, max0, maxPos0, maxIndex);
	if(max0==-9999.)
		return -9999.;

	max0 -= pedestal;

	//-- cut intended to remove noise  
	if(m_triggerNoiseCut.size()>0 && max0*0.25<m_triggerNoiseCut[0])//Energy cut
		return 0.; 
	
	double ratio = getMaxOverSumRatio(ADCsamples, pedestal);

	if(m_triggerNoiseCut.size()>4)
	{
		double energyCut = m_triggerNoiseCut[0];
		double minRatioCut = m_triggerNoiseCut[1];
		double lowerMaxRatioCut = m_triggerNoiseCut[2];
		double higherMaxRatioCut = m_triggerNoiseCut[3];
		double decRateRatioCut = m_triggerNoiseCut[4];

		double maxRatioCut = lowerMaxRatioCut + higherMaxRatioCut*exp(-decRateRatioCut*(max0*0.25-energyCut));
		if(ratio<minRatioCut || ratio>maxRatioCut)
			return 0.;
	}

	if((int)maxIndex-2>=0 && (int)maxIndex+2<nbSamples)
	{
	        std::unique_ptr<TF1> pulseFit = std::make_unique<TF1>("pulseFit", landauLandau , (float)maxIndex-2., (float)maxIndex+2., 5);
		//-- values from parabola are used as initial values
		pulseFit->SetParameters(max0*exp(0.5),maxPos0,0.7, 1., 10.); 
		pulseFit->SetParLimits(1, 0.,15.);
		pulseFit->SetParLimits(2, 0.48, 1);
		pulseFit->SetParLimits(3, 0.04, 1.6);
		pulseFit->SetParLimits(4, 1, 300); 

		std::unique_ptr<TH1F> h_samples = std::make_unique<TH1F>("samples", "samples", nbSamples,-0.5,(float)nbSamples - 0.5);

		for(int sample=0; sample<nbSamples;sample++)
		{
			h_samples->SetBinContent(sample+1, ADCsamples[sample]-pedestal);
			h_samples->SetBinError(sample+1, 4.);
		}

		//To ensure thread-safety, we need to use Minuit2 and avoid usage of static
		//variables (like the DefaultMinimizerOptions) under the hood. 
		//To this end, the following few lines have been copied from the root's TH1.cxx
		//to replace the line
		//h_samples->Fit(pulseFit,"QRN");
		
		Foption_t fitOption;
		ROOT::Fit::FitOptionsMake(ROOT::Fit::kHistogram,"QRN",fitOption);
		ROOT::Fit::DataRange range(0,0);
		ROOT::Math::MinimizerOptions minOption;
		minOption.SetMinimizerType("Minuit2");
		{
		  std::lock_guard<std::mutex> guard(m_fitMutex);
		  //unsure if the following function is really thread-safe ... 
		  ROOT::Fit::FitObject(h_samples.get(), pulseFit.get() , fitOption , minOption, "", range);
		}
		energy = pulseFit->GetParameter(0)*exp(-0.5)*0.25;// 1 ADC <-> 0.25GeV
		//std::cout << "DeadOTXEnergy: " <<  energy << " at " << Gaudi::Hive::currentContext().evt() << std::endl;
	}
	else //-- if we can't fit a Landau-Landau, we use the result from parabola
		energy = max0*0.25;

	//-- we apply eta and energy corrections
	double etaCal = getEtaCalibration(etaCalibrationSizes,
                                          etaCalibrations,
                                          eta, type);
	if(etaCal!=0.)
		energy /= etaCal;

	double energyCal = getEnergyCalibration(energyCalibrationTypes,
                                                energyCalibrations,
                                                eta, type, energy);
	energy *= energyCal;

	return energy;
}

double LArCellDeadOTXCorr::getEtaCalibration(const CxxUtils::Array<1>& etaCalibrationSizes,
                                             const CxxUtils::Array<1>& etaCalibrations,
                                             double eta, int type) const
{
	unsigned int totalEtaSize = 0;

	int barrelEtaSize = 0;
	int emecEtaSize = 0;
	int hecEtaSize = 0;
	//int fcalEtaSize = 0;

	for(unsigned int i=0;i<etaCalibrationSizes.size();i++)
		totalEtaSize += etaCalibrationSizes[i];
	

	if(totalEtaSize != etaCalibrations.size())
	{
                ATH_MSG_WARNING ("The number of eta-dependent calibration factors is not consistent with the given sizes for each calorimeter parts.");
		ATH_MSG_WARNING ("Eta-dependent calibrations will not be applied");
	}
	else
	{
		barrelEtaSize = (etaCalibrationSizes.size()>0 ? etaCalibrationSizes[0] : 0);
		emecEtaSize = (etaCalibrationSizes.size()>1 ? etaCalibrationSizes[1] : 0);
		hecEtaSize = (etaCalibrationSizes.size()>2 ? etaCalibrationSizes[2] : 0);
		//fcalEtaSize = (etaCalibrationSizes.size()>3 ? etaCalibrationSizes[3] : 0);
	}

	int region = TTID_regionIndex(eta);
	int nbIndexBarrel = 15;
	int nbIndexEC0 = 10;
	int nbIndexEC1 = 3;
	int index = TTID_etaIndex(eta);
	double calibration = 1.;


	int ieta = 0;
	int shift = 0;
	bool doCorr = false;


	if(region==0 && type==0 && index < nbIndexBarrel)// Barrel
	{
		ieta = index;
		shift = 0;
		if(ieta<barrelEtaSize)
			doCorr = true; 
	}
	else if(region==0 && type==0)// EMEC
	{
		ieta = index - nbIndexBarrel;
		shift = barrelEtaSize;
		if(ieta<emecEtaSize)
			doCorr = true;
	}
	else if(region==1 && type==0)// EMEC
	{
		ieta = index + nbIndexEC0;
		shift = barrelEtaSize; 
		if(ieta<emecEtaSize)
			doCorr = true;
	}
	else if(region==2 && type==0)// EMEC
	{
		ieta = index  + nbIndexEC0 + nbIndexEC1;
		shift = barrelEtaSize;
		if(ieta<emecEtaSize)
			doCorr = true;
	}
	else if(region==0 && type==1)// HEC
	{
		ieta = index - nbIndexBarrel;
		shift = barrelEtaSize + emecEtaSize;
		if(ieta<hecEtaSize)
			doCorr = true;
	}
	else if(region==1 && type==1)// HEC
	{
		ieta = index + nbIndexEC0;
		shift=  barrelEtaSize + emecEtaSize;
		if(ieta<hecEtaSize)
			doCorr = true;
	}
	else if(region==2 && type==1)//HEC
	{
		ieta = index + nbIndexEC0 + nbIndexEC1;
		shift = barrelEtaSize + emecEtaSize;
		if(ieta<hecEtaSize)
			doCorr = true;
	}

	if(doCorr)
		calibration = etaCalibrations[shift+ieta];


	return calibration;
}


double LArCellDeadOTXCorr::getEnergyCalibration(const CxxUtils::Array<1>& energyCalibrationTypes,
                                                const CxxUtils::Array<1>& energyCalibrations,
                                                double eta, int type, double energy) const
{
	unsigned int totalEnergySize = 0;

	int barrelEnergySize = 0;
	int emecEnergySize = 0;
	//int hecEnergySize = 0;
	//int fcalEnergySize = 0;

	int barrelEnergyType = -1;
	int emecEnergyType = -1;
	int hecEnergyType = -1;
	//int fcalEnergyType = -1;

	for(unsigned int i=0;i<energyCalibrationTypes.size();i++)
	{
		std::map<int, int>::const_iterator it = m_typeSizeMapping.find(energyCalibrationTypes[i]);
		if(it!=m_typeSizeMapping.end())
			totalEnergySize += it->second;

	}
	if(totalEnergySize != energyCalibrations.size())
	{
                ATH_MSG_WARNING ("The number of parameters for the energy-dependent calibrations is not consistent with the given parametrizations for each calorimeter parts");
		ATH_MSG_WARNING ("Energy-dependent calibrations will not be applied");
	}
	else
	{
		barrelEnergyType = (energyCalibrationTypes.size()>0 ? energyCalibrationTypes[0] : -1);
		emecEnergyType = (energyCalibrationTypes.size()>1 ? energyCalibrationTypes[1] : -1);
		hecEnergyType = (energyCalibrationTypes.size()>2 ? energyCalibrationTypes[2] : -1);
		//fcalEnergyType = (energyCalibrationTypes.size()>3 ? energyCalibrationTypes[3] : -1);


		std::map<int, int>::const_iterator itBarrel = m_typeSizeMapping.find(barrelEnergyType);
		if(itBarrel!=m_typeSizeMapping.end())
			barrelEnergySize = itBarrel->second;

		std::map<int, int>::const_iterator itEmec = m_typeSizeMapping.find(emecEnergyType);
		if(itEmec!=m_typeSizeMapping.end())
			emecEnergySize = itEmec->second;

		//std::map<int, int>::const_iterator itHec = m_typeSizeMapping.find(hecEnergyType);
		//if(itHec!=m_typeSizeMapping.end())
                //    hecEnergySize = itHec->second;

		//std::map<int, int>::const_iterator itFcal = m_typeSizeMapping.find(fcalEnergyType);
		//if(itFcal!=m_typeSizeMapping.end())
	        //		fcalEnergySize = itFcal->second;
	}


	double calibration = 1.;
	int calType = -1;
	int shift = 0;


	//-- Barrel
	if(fabs(eta)<1.5)
	{
		calType = barrelEnergyType;
		shift = 0;
	}
	//-- EMEC
	else if(fabs(eta)>1.5 && fabs(eta)<3.2 && type==0)
	{
		calType = emecEnergyType;
		shift = barrelEnergySize;
	}
	//-- HEC
	else if(fabs(eta)>1.5 && fabs(eta)<3.2 && type==1)
	{
		calType = hecEnergyType;
		shift = barrelEnergySize + emecEnergySize;
	}


	switch(calType)
	{
		case 0 : //exponential 
			{
				double threshold = energyCalibrations[shift];
				double yShift = energyCalibrations[shift + 1];
				double max = energyCalibrations[shift + 2];
				double variation = energyCalibrations[shift + 3];
				double xShift = energyCalibrations[shift + 4];
				if(energy<threshold) energy = threshold; //threshold on the calibration
				calibration = yShift+ max*exp(variation*(energy - xShift));
				break;
			}
		case -1 : 
			break;
		default : 
			break;
	}



	return calibration;


}


double LArCellDeadOTXCorr::getMaxOverSumRatio(const std::vector<uint_least16_t>& ADCsamples, int pedestal) const
{
        int max = -9999;
	int sum = 0;
	double ratio = -9999.;
	unsigned int maxIndex = 0;
	for(unsigned int ADCi = 0;ADCi<ADCsamples.size();ADCi++)
	{
		if(ADCsamples[ADCi] - pedestal>max)
		{
			max = ADCsamples[ADCi] -pedestal;
			maxIndex = ADCi;
		}
	}
	
	if(maxIndex>=2 && maxIndex+2<ADCsamples.size())
		sum = ADCsamples[maxIndex-2] + ADCsamples[maxIndex-1] + ADCsamples[maxIndex] + ADCsamples[maxIndex+1] + ADCsamples[maxIndex+2] - 5*pedestal;

	if(sum!=0)
		ratio = (double)max/(double)sum;

	return ratio;
}



 

