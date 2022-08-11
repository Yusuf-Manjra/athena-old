/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//  Filename : TileDigitsFromPulse.cxx
//  Author   : Simon Molander
//  Created  : February 2013
//
//  DESCRIPTION
// 
//  Create TileDigits from simulated pulses. 
//
//  HISTORY:
//
//  BUGS:
//
//*****************************************************************************

// Tile includes
#include "TileSimAlgs/TileDigitsFromPulse.h"
#include "TileEvent/TileDigits.h"
#include "TileEvent/TileMutableDigitsContainer.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileInfo.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

//Simulator includes
#include "TilePulseSimulator/TileSampleGenerator.h"
#include "TilePulseSimulator/TileSampleBuffer.h"
#include "TilePulseSimulator/TilePulseShape.h"

// Athena includes
#include "AthAllocators/DataPool.h"
#include "PathResolver/PathResolver.h"
//Random number service
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"

#include <CLHEP/Random/Randomize.h>
#include <CLHEP/Units/SystemOfUnits.h>

//Root includes
#include "TRandom3.h"
#include "TFile.h"
#include "TH1F.h"
#include "TKey.h"
#include "TF1.h"

//C++ STL includes
#include <vector>


using CLHEP::RandGaussQ;
using CLHEP::RandFlat;

//
// Constructor
//
TileDigitsFromPulse::TileDigitsFromPulse(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_tileHWID(0),
  m_tileInfo(0)
{
	m_rChUnit = TileRawChannelUnit::ADCcounts;
	m_rChType = TileFragHash::Default;

	declareProperty("ImperfectionMean", m_imperfectionMean = 1.01);
	declareProperty("ImperfectionRms", m_imperfectionRms = 0.02);
	declareProperty("InTimeAmp", m_inTimeAmp = 300.);
	declareProperty("OutOfTimeAmp", m_ootAmp = 150.);
	declareProperty("InTimeOffset", m_itOffset = 0.);
	declareProperty("OutOfTimeOffset", m_ootOffset = 50.);
	declareProperty("OutOfTimeOffsetHistogramFile", m_ootOffsetFileName = "");
	declareProperty("OutOfTimeOffsetHistogramName", m_ootOffsetHistName = "");
	declareProperty("UseGaussNoise", m_gaussNoise = kFALSE);
	declareProperty("GaussNoiseAmpOne", m_GNAmpOne = 1 / 1.039);
	declareProperty("GaussNoiseSigmaOne", m_GNSigmaOne = 1.6);
	declareProperty("GaussNoiseAmpTwo", m_GNAmpTwo = 0.039);
	declareProperty("GaussNoiseSigmaTwo", m_GNSigmaTwo = 3.6);
	declareProperty("UseInTimeAmpDist", m_useItADist = kFALSE);
	declareProperty("UseOutOfTimeAmpDist", m_useOotADist = kFALSE);
	declareProperty("InTimeAmpDistFileName", m_itADistFileName = "");
	declareProperty("OutOfTimeAmpDistFileName", m_ootADistFileName = "");
	declareProperty("PileUpFraction", m_pileUpFraction = 1);
	declareProperty("GaussianC2CPhaseVariation", m_gausC2C = 0);
	declareProperty("ChannelSpecificPedestal", m_chanPed = kFALSE);
	declareProperty("ChannelSpecificNoise", m_chanNoise = kFALSE);
	declareProperty("PedestalValueHG", m_ped_HG = 100);
	declareProperty("PedestalValueLG", m_ped_LG = 100);
	declareProperty("AmpDistLowerLimit", m_AmpDistLowLim = 135);
	declareProperty("InTimeAmpDistHistogramName", m_itADistHistName = "h_Eopt_hi");
	declareProperty("OutOfTimeAmpDistHistogramName", m_ootADistHistName = "h_Eopt_hi");

	declareProperty("SimulatePileUpWithPoiss", m_simPUwPoisson = kFALSE); 
	declareProperty("AvgMuForPileUpSimulation", m_avgMuForPU = 40);
	declareProperty("PileUpAmpDistFileName", m_pileupAmpDistFileName = "");

	declareProperty("RandomSeed", m_seed = 4357);
	declareProperty("BunchSpacing", m_BunchSpacing = 25.); // 25, 50 or 75
	declareProperty("SimulateQIE", m_simQIE = kFALSE);

	declareProperty("TileInfoName", m_infoName = "TileInfo");
	declareProperty("TilePhaseII", m_PhaseII = kFALSE);
	declareProperty("Bigain", m_bigain = kFALSE);
	declareProperty("NSamples", m_nSamples = 7); 
	declareProperty("nPulses", m_nPul = 21); 

	//Initialisations
	m_ps[0] = new TilePulseShape(msgSvc(), "TilePulseShapeLo"); //Low Gain
	m_ps[1] = new TilePulseShape(msgSvc(), "TilePulseShapeHi"); //High Gain

	
	m_itFile = new TFile();
	m_itDist = new TH1F();
	m_ootFile = new TFile();
	m_ootDist = new TH1F();
	m_ootOffsetDist = new TH1F();
	m_ootOffsetFile = new TFile();

	m_pileup_AmpDistFile = new TFile();

	m_useOffsetHisto = kFALSE;

}

TileDigitsFromPulse::~TileDigitsFromPulse() {

	delete m_ootOffsetFile;
	delete m_ootFile;
	delete m_itFile;
	delete m_pileup_AmpDistFile;
	
	delete m_ps[0];
	delete m_ps[1];
}

//
// Alg standard initialize function
//
StatusCode TileDigitsFromPulse::initialize() {

	ATH_MSG_DEBUG("in initialize()");

	m_buf = new TileSampleBuffer(m_nSamples, -(m_nSamples-1)*25/2, 25.);
	
	m_tsg = new TileSampleGenerator(m_ps[0], m_buf, false); //Set third parameter to true for debug of the sum of pulses

	m_nPul_eff = (m_nPul - 1) / 2; //Used for symetrization of PU in computation

	ATH_CHECK(detStore()->retrieve(m_tileHWID, "TileHWID"));
	ATH_CHECK(detStore()->retrieve(m_tileInfo, m_infoName));
 	m_i_ADCmax = m_tileInfo->ADCmax();
	ATH_MSG_DEBUG("Max ADC counts TileDigitsFromPulse: " << m_i_ADCmax);

        ATH_CHECK(m_tileToolNoiseSample.retrieve());

        ATH_CHECK(m_digitsContainerKey.initialize());
        ATH_MSG_INFO("Output digits container: " <<  m_digitsContainerKey.key());

        ATH_CHECK(m_rawChannelContainerKey.initialize());
	ATH_MSG_INFO("Output raw channel container: " << m_rawChannelContainerKey.key());


	//Build pulse shapes
	m_ps[0]->setPulseShape(m_tileInfo->digitsFullShapeLo());
	m_ps[1]->setPulseShape(m_tileInfo->digitsFullShapeHi());

	//Initialise distribution histograms if in use
	if (m_useItADist) {
		if (m_itADistFileName.size() == 0) {
			m_itADistFileName = PathResolver::find_file("Distributions_small_h2000_177531_JetTauEtmiss.root", "DATAPATH");
			if (m_itADistFileName.size() == 0) {
				ATH_MSG_FATAL("Could not find input file Distributions_small_h2000_177531_JetTauEtmiss.root");
				return StatusCode::FAILURE;
			}
		}
		if (makeDist(m_itFile, m_itDist, m_itADistFileName, m_itADistHistName) == kFALSE)
			return StatusCode::FAILURE;
		ATH_MSG_DEBUG("Made in-time distribution");
	} else
		delete m_itDist;

	if (m_simPUwPoisson){

		if (m_pileupAmpDistFileName.size() == 0) {
			m_pileupAmpDistFileName = PathResolver::find_file("Distributions_MB_minbias_inelastic_lowjetphoton_e8314_e7400_s3508.root", "DATAPATH");
			//m_pileupAmpDistFileName = PathResolver::find_file("Distributions_small_h2000_177531_ZeroBias.root", "DATAPATH");
			if (m_pileupAmpDistFileName.size() == 0 ) {
				ATH_MSG_FATAL("Could not find input file Distributions_MB_minbias_inelastic_lowjetphoton_e8314_e7400_s3508.root");
				return StatusCode::FAILURE;
			}

		}
		if (makeDist(m_pileup_AmpDistFile, m_pileup_AmpDists, m_pileupAmpDistFileName) == kFALSE)
			return StatusCode::FAILURE;
		ATH_MSG_DEBUG("Made PU amp distributions for each partition and channel");

	}

	if (m_useOotADist) {
		if (m_ootADistFileName.size() == 0) {
			m_ootADistFileName = PathResolver::find_file("Distributions_small_h2000_177531_ZeroBias.root", "DATAPATH");
			if (m_ootADistFileName.size() == 0) {
				ATH_MSG_FATAL("Could not find input file Distributions_small_h2000_177531_ZeroBias.root");
				return StatusCode::FAILURE;
			}
		}
		if (makeDist(m_ootFile, m_ootDist, m_ootADistFileName, m_ootADistHistName) == kFALSE)
			return StatusCode::FAILURE;
		ATH_MSG_DEBUG("Made Oot distribution");
	} else
		delete m_ootDist;

	//Initialise timing offset distribution. If filename is empty, use static offset
	if (m_ootOffsetFileName.size() != 0) {
		m_ootOffsetFile = TFile::Open(m_ootOffsetFileName.c_str());
		if (m_ootOffsetFile->IsZombie()) {
			ATH_MSG_WARNING("Error reading offset timing distribution from " << m_ootOffsetFileName << ". Using static timing offset.");
		} else {
			TKey *key = m_ootOffsetFile->FindKey(m_ootOffsetHistName.c_str());
			if (key == 0) {
				ATH_MSG_WARNING("Histogram " << m_ootOffsetHistName << " not found in file " << m_ootOffsetFileName << ". Using static timing offset.");
			} else {
				m_ootOffsetDist = (TH1F*) m_ootOffsetFile->Get(m_ootOffsetHistName.c_str());
				m_useOffsetHisto = kTRUE;
			}
		}
	}

	//Start the random number service used to create channel specific noise
	if (!m_rndmSvc.retrieve().isSuccess()) {
		ATH_MSG_FATAL("Could not initialize find Random Number Service.");
		return StatusCode::FAILURE;
	}
	if (m_chanNoise)
		m_gaussNoise = kFALSE; //Make sure channel noise overrides gaussian noise.

	ATH_MSG_DEBUG("initialize() successful");

	return StatusCode::SUCCESS;
}
/*==========================================================================*/
//
// Begin Execution Phase.
//
StatusCode TileDigitsFromPulse::execute() {

	ATH_MSG_DEBUG("in execute()");

	const EventContext& ctx = Gaudi::Hive::currentContext();

	// Prepare RNG service
	ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomStreamName);
	rngWrapper->setSeed( m_randomStreamName, ctx );

	// Create new container for digits
	auto digitsContainer = std::make_unique<TileMutableDigitsContainer>(true,
                                                                            TileFragHash::Digitizer,
                                                                            TileRawChannelUnit::ADCcounts,
                                                                            SG::VIEW_ELEMENTS);

        ATH_CHECK( digitsContainer->status() );

	//Create RawChannel for truth values.
	auto rawChannelContainer = std::make_unique<TileMutableRawChannelContainer>(true, m_rChType, m_rChUnit);
        ATH_CHECK( rawChannelContainer->status() );

	DataPool < TileDigits > tileDigitsPool(m_tileHWID->adc_hash_max());

	TRandom3 *random = new TRandom3(m_seed); //Randomizer for pulse-shape imperfection
	double tFit = 0, ped = 100; //Settings for simulation 
	double mu = 0; //Interaction per bunch crossing for PU simulation

	TF1 *pdf = new TF1();
	TF1 *pdf_PhaseI = new TF1();
	TF1 *pdf_lo = new TF1();
	TF1 *pdf_hi = new TF1();
	if (!m_simQIE) {
		

	  	if(m_PhaseII){
		  Double_t sigma_lo = 1; //Noise value obtained from ATL-COM-TILECAL-2020-031
	    	pdf_lo = new TF1("pdf_lo","(1/(sqrt(2*pi)*[0])) * (exp(-0.5*(x/[0])**2)/(sqrt(2*pi)*[0]))", -100, 100);
			pdf_lo->SetParameter(0,sigma_lo);

			Double_t sigma_hi = 2.5; //Noise value obtained from ATL-COM-TILECAL-2020-031
			pdf_hi = new TF1("pdf_hi","(1/(sqrt(2*pi)*[0])) * (exp(-0.5*(x/[0])**2)/(sqrt(2*pi)*[0]))", -100, 100);
			pdf_hi->SetParameter(0,sigma_hi);
		}
	  	else{
			//Noise pdf for general noise. Maybe use as a member and put in init.
			//pdf = new TF1("pdf", "[0] * (Gaus(x,0,[1]) + [2] * Gaus(x,0,[3]))", -100., 100.); //Root goes not like "Gaus"		

	    	pdf_PhaseI = new TF1("pdf_PhaseI", "[0] * (exp(-0.5*(x/[1])**2)/(sqrt(2*pi)*[1]) + [2] *exp(-0.5*(x/[3])**2)/(sqrt(2*pi)*[3]))", -100., 100.);
	    	pdf_PhaseI->SetParameters(m_GNAmpOne, m_GNSigmaOne, m_GNAmpTwo, m_GNSigmaTwo);
	  	}
	}

	std::vector<float> samples(m_nSamples);

	double Rndm[16]; // Can't use variable size array,
	double Rndm_dG[1]; // uniform random number for the double gaussian

	double amp_1;
	double amp_2;

	ATH_MSG_DEBUG("Starting loop");
	int gain = 1;
	double n_inTimeAmp = 0.0; //!< Local loop variable for amplitude of in-time pulse

	for (int ros = 1; ros < 5; ++ros) {
		for (int drawer = 0; drawer < 64; ++drawer) {
			unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
			for (int channel = 0; channel < 48; ++channel) {

				if (!m_simQIE) { //3-in-1 is simulated below

					if(m_PhaseII){
						ATH_MSG_VERBOSE("executing FENICS code");
					}
					else{
						ATH_MSG_VERBOSE("executing 3-in-1 code");
					}
					

					bool isHGSaturated = false;

					for (int igain = 1; igain >= 0; igain--) {
						gain = igain;
						if (gain == 1) {
							n_inTimeAmp = m_useItADist ? m_itDist->GetRandom() : m_inTimeAmp;

							if (m_chanPed){
							  	ped = m_tileToolNoiseSample->getPed(drawerIdx, channel, gain, TileRawChannelUnit::ADCcounts, ctx);
							}
							else{
								ped = m_ped_HG;
							}
							if (random->Rndm() >= m_pileUpFraction)
							  m_ootAmp = 0; //Set oot amplitude to 0 if no pile-up.
							tFit = random->Gaus(0., m_gausC2C); //C2C phase variation
							double deformatedTime = random->Gaus(m_imperfectionMean, m_imperfectionRms); //Widening of pulseshape
							m_ps[gain]->scalePulse(deformatedTime, deformatedTime); // Deformation of pulse shape by changing its width
							//if(m_useOffsetHisto) m_ootOffset = m_ootOffsetDist->GetRandom();  //OLD Remove for 7 samples -> BunchSpacing

							//Pileup samples
							m_PUAmp.clear();
							m_PUAmp.resize(m_nPul);

							for (int i = 0; i <= m_nPul_eff; i++) {
								if (((i * 25) % m_BunchSpacing) == 0) {
									if(m_simPUwPoisson){
										mu=random->Poisson(m_avgMuForPU);
										ATH_MSG_VERBOSE("Effective pulse number " << i);
										ATH_MSG_VERBOSE("Number of interactions for simulation: " << mu );
										for (int imu = 0; imu<mu; imu++){

											amp_1 = m_pileup_AmpDists[ros-1][channel]->GetRandom();
											amp_2 = m_pileup_AmpDists[ros-1][channel]->GetRandom();

											ATH_MSG_VERBOSE("Random amplitudes for PU: " << amp_1 << " " << amp_2);

											if(i==0){
												m_PUAmp.at(m_nPul_eff) += abs(amp_1);
											}
											else{
												m_PUAmp.at(m_nPul_eff + i) += abs(amp_1);
												m_PUAmp.at(m_nPul_eff - i) += abs(amp_2);
											}
										}

										ATH_MSG_VERBOSE("Final amplitudes for pulse " << m_PUAmp.at(m_nPul_eff + i) << " " << m_PUAmp.at(m_nPul_eff - i));
									}
									else{
										m_PUAmp.at(m_nPul_eff + i) = m_useOotADist ? m_ootDist->GetRandom() : m_ootAmp;
										m_PUAmp.at(m_nPul_eff - i) = m_useOotADist ? m_ootDist->GetRandom() : m_ootAmp;
									}
								} else {
									m_PUAmp.at(m_nPul_eff + i) = 0;
									m_PUAmp.at(m_nPul_eff - i) = 0;
								}
							}
							
						
						} else {
							if (m_chanPed)
								ped = m_tileToolNoiseSample->getPed(drawerIdx, channel, gain, TileRawChannelUnit::ADCcounts, ctx);
							double deformatedTime = random->Gaus(m_imperfectionMean, m_imperfectionRms); //Widening of pulseshape
							m_ps[gain]->scalePulse(deformatedTime, deformatedTime); // Deformation of pulse shape by changing its width

							if (m_chanPed)
								ped = m_tileToolNoiseSample->getPed(drawerIdx, channel, gain, TileRawChannelUnit::ADCcounts, ctx);

							else{
								ped=m_ped_LG;
							}
							
							if(!m_PhaseII){
							  n_inTimeAmp /= 64;
							  for (int i = 0; i <= m_nPul_eff; i++) {
							    m_PUAmp.at(m_nPul_eff + i) /= 64;
							    m_PUAmp.at(m_nPul_eff - i) /= 64;
							  }
							} else{
							  n_inTimeAmp /= 40; 
							  for (int i = 0; i <= m_nPul_eff; i++) {
							    m_PUAmp.at(m_nPul_eff + i) /= 40;
							    m_PUAmp.at(m_nPul_eff - i) /= 40;
							  }
							}
						}

						if(m_PhaseII && m_gaussNoise){
							pdf = (gain==1) ? pdf_hi : pdf_lo;
						}
						else if(m_gaussNoise){
							pdf = pdf_PhaseI;
						}

						m_tsg->setPulseShape(m_ps[gain]);
						m_tsg->fillNSamples(tFit, ped, n_inTimeAmp, m_PUAmp, pdf, m_gaussNoise, m_itOffset, m_nSamples, m_nPul); // Sum of Intime + PU pulses			
						
						samples.clear();
						samples.resize(m_nSamples);
						m_buf->getValueVector(samples);

						if (m_chanNoise) {
							double Hfn1 = m_tileToolNoiseSample->getHfn1(drawerIdx, channel, gain, ctx);
							double Hfn2 = m_tileToolNoiseSample->getHfn2(drawerIdx, channel, gain, ctx);
							double Norm = m_tileToolNoiseSample->getHfnNorm(drawerIdx, channel, gain, ctx);
							RandGaussQ::shootArray(*rngWrapper, samples.size(), Rndm, 0.0, 1.0);
							RandFlat::shootArray(*rngWrapper, 1, Rndm_dG, 0.0, 1.0);
							for (unsigned int js = 0; js < samples.size(); ++js) {
								//using the same gaussian(sigma) for all samples in one channel in one event
								if (Rndm_dG[0] < Norm)
									samples[js] += (float) Hfn1 * Rndm[js];
								else
									samples[js] += (float) Hfn2 * Rndm[js];
							}
						}

						for (unsigned int i = 0; i < samples.size(); ++i) {
						  if (samples[i] >= m_i_ADCmax){
						    isHGSaturated = true;
						    if(m_bigain) samples[i]=m_i_ADCmax;
						  }
							
						}
						
						if(!m_bigain){
						  if (!isHGSaturated)
						    break;
						}
						
						if(m_bigain){
						  ATH_MSG_VERBOSE("New ADC " << ros << "/" << drawer << "/" << channel << "/   saving gain  " << gain);

						  TileDigits * digit = tileDigitsPool.nextElementPtr();
						  *digit = TileDigits (m_tileHWID->adc_id(ros, drawer, channel, gain),
								       std::move(samples));

						  ATH_CHECK( digitsContainer->push_back(digit) ); 
						  
						  auto rawChannel = std::make_unique<TileRawChannel>(digit->adc_HWID(),
												     n_inTimeAmp,
												     tFit,
												     m_ootAmp,
												     m_ootOffset);
						  
						  ATH_CHECK( rawChannelContainer->push_back(std::move(rawChannel)) );
						}
					}

				} else { //QIE is simulated here --------------------------------------------

					//ATH_MSG_DEBUG("executing QIE code");

					gain = 1; //This is just a place holder. The gain is not used in QIE.
					n_inTimeAmp = m_useItADist ? m_itDist->GetRandom() : m_inTimeAmp;
					//if (random->Rndm() >= m_pileUpFraction) //m_pileUpFraction is 1 by default
					m_ootAmp = 0; //Set oot amplitude to 0 if no pile-up.
					tFit = 0; //TODO: Introduce jitter of the PMT pulse; random->Gaus(0., m_gausC2C); //C2C phase variation

					//Pileup samples
					//m_PUAmp.clear();
					//m_PUAmp.resize(nPul);
					float my_PUAmp[7]; //I use an array to store the energies/charges of the out-of-time pulses

					for (int i = 0; i < 7; i++)
						if ((((i - 3) * 25) % (int) m_BunchSpacing) == 0) {
							if (i != 3) { //index 3 corresponds to the in-time pulse, the signal
								//m_PUAmp.at(i) = m_useOotADist ? m_ootDist->GetRandom() : m_ootAmp; //out-of-time pulses
								my_PUAmp[i] = m_useOotADist ? m_ootDist->GetRandom() : m_ootAmp; //out-of-time pulses
							} else {
								//m_PUAmp.at(i) = 0; //it-time pulse
								my_PUAmp[i] = 0;
							}
						}

					//fill7SamplesQIE(float t0, float amp_it, float *amp_pu, bool addNoise);
					m_tsg->fill7SamplesQIE((float) n_inTimeAmp, my_PUAmp); // Sum of In time + out-of-time PU pulses

					samples.clear();
					samples.resize(m_nSamples);
					m_buf->getValueVector(samples);
				}

				if(!m_bigain){

					ATH_MSG_VERBOSE("New ADC " << ros << "/" << drawer << "/" << channel << "/   saving gain  " << gain);

					TileDigits * digit = tileDigitsPool.nextElementPtr();
					*digit = TileDigits (m_tileHWID->adc_id(ros, drawer, channel, gain),
						     std::move(samples));
				
					ATH_CHECK( digitsContainer->push_back(digit) );
				
					auto rawChannel = std::make_unique<TileRawChannel>(digit->adc_HWID(),
										   n_inTimeAmp,
										   tFit,
										   m_ootAmp,
										   m_ootOffset);
				
					ATH_CHECK( rawChannelContainer->push_back(std::move(rawChannel)) );
				}
			}
		}
	}

        SG::WriteHandle<TileRawChannelContainer> rawChannelCnt(m_rawChannelContainerKey, ctx);
        ATH_CHECK( rawChannelCnt.record(std::move(rawChannelContainer)) );

        SG::WriteHandle<TileDigitsContainer> digitsCnt(m_digitsContainerKey, ctx);
        ATH_CHECK( digitsCnt.record(std::move(digitsContainer)) );

	if (!m_simQIE) {
		//delete pdf;
		delete pdf_PhaseI;
		delete pdf_hi;
		delete pdf_lo;
	}
	delete random;

	ATH_MSG_DEBUG("Execution completed");

	return StatusCode::SUCCESS;
}

StatusCode TileDigitsFromPulse::finalize() {
	ATH_MSG_DEBUG("in finalize()");
	delete m_buf;
	delete m_tsg;
	

	if (m_useItADist)
		m_itFile->Close();
	if (m_useOotADist)
		m_ootFile->Close();
	if (m_simPUwPoisson)
		m_pileup_AmpDistFile->Close();

	return StatusCode::SUCCESS;
}

bool TileDigitsFromPulse::makeDist(TFile*& file, TH1F*& hist, std::string fileName, std::string histName) {
	file = new TFile(fileName.c_str());
	if (file->IsZombie()) {
		ATH_MSG_FATAL("Error reading amplitude distribution from " << fileName << ".");
		return kFALSE;
	}
	TKey *key = file->FindKey(histName.c_str());
	if (key == 0) {
		ATH_MSG_FATAL("Could not find histogram " << histName << " in file " << fileName << ".");
		return kFALSE;
	}
	hist = (TH1F*) file->Get(histName.c_str());
	for (int i = 0; i < m_AmpDistLowLim; i++)
		hist->SetBinContent(i, 0.); // Puts a cut on the amplitude distribution.
	return kTRUE;

}

bool TileDigitsFromPulse::makeDist(TFile*& file, std::vector<std::vector<TH1F*>>& hists, std::string fileName) {

	std::string histName;
	TKey *key;
	TH1F* hist;

	file = new TFile(fileName.c_str());
	if (file->IsZombie()) {
		ATH_MSG_FATAL("Error reading amplitude distributions from " << fileName << ".");
		return kFALSE;
	}

	for(int ros=0; ros<4; ros++){

    	hists.push_back(std::vector<TH1F*>());
    	for(int channel=0; channel<48; channel++){

			histName = "ene_ros_" + std::to_string(ros+1) + "_channel_" + std::to_string(channel+1);

			key = file->FindKey(histName.c_str());
			if (key == 0) {
				ATH_MSG_FATAL("Could not find histogram " << histName << " in file " << fileName << ".");
				return kFALSE;
			}

			hist = (TH1F*) file->Get(histName.c_str());

			for (int i = 0; i < m_AmpDistLowLim; i++)
			 	hist->SetBinContent(i, 0.); // Puts a cut on the amplitude distribution.

        	hists[ros].push_back(hist);   
			hist->Clear();
    	}
	}

	return kTRUE;

}
