/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"


namespace DerivationFramework{

  HadronOriginClassifier::HadronOriginClassifier(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p),
    m_mcName("TruthEvents"),
    m_HadronPtMinCut(0),
    m_HadronEtaMaxCut(0),
    m_DSID(0)
  {
    declareInterface<DerivationFramework::HadronOriginClassifier>(this);

    declareProperty("MCCollectionName",m_mcName="TruthEvents");
    declareProperty("HadronpTMinCut",m_HadronPtMinCut=5000.); /// MeV
    declareProperty("HadronetaMaxCut",m_HadronEtaMaxCut=2.5);
    declareProperty("DSID",m_DSID=410000);
  }


  //--------------------------------------------------------------------------
  HadronOriginClassifier::~HadronOriginClassifier(){
    /////
  }

  //---------------------------------------------------------------------------
  StatusCode HadronOriginClassifier::initialize() {
    ATH_MSG_INFO("Initialize " );
    // all Herwig++/Herwig7 showered samples
    if( m_DSID==410003 || m_DSID == 410008 //aMC@NLO+Hpp
          || m_DSID == 410004 || m_DSID == 410163 //Powheg+Hpp
          || m_DSID == 410232 //first attempt for Powheg+H7
          || m_DSID == 410233 //first attempt for aMC@NLO+H7
          || (m_DSID>=410525 && m_DSID<=410530) //New Powheg+H7 samples
          || (m_DSID>=407037 && m_DSID<=407040) //Powheg+Hpp MET/HT sliced
          || m_DSID ==410536 || m_DSID == 410537
          || m_DSID == 410245 //aMC@NLO+H++ , ttbb
          || (m_DSID>=410557 && m_DSID<=410559) // new Powheg+H7, mc16
          || (m_DSID>=411082 && m_DSID<=411090) //Powheg+H7 HF-filtered
          || (m_DSID>=407354 && m_DSID<=407356) //Powheg+H7 ttbar HT-filtered
          || m_DSID ==411233 || m_DSID == 411234 //Powheg+H7.1.3 ttbar 
          || (m_DSID>=411329 && m_DSID<=411334) //Powheg+H7.1.3 ttbar HF-filtered
          || (m_DSID>=411335 && m_DSID<=411337) //Powheg+H7.1.3 ttbar HT-filtered
          || m_DSID ==412116 || m_DSID == 412117 //amc@NLO+H7.1.3 ttbar 
          ){
      m_GenUsed=HerwigPP;
      if (m_DSID==410245){
        m_ttbb=true;
      }
    }
    // all Pythia8 showered samples
    else if( m_DSID==410006 //Powheg+P8 old main31
          || m_DSID==410500 //Powheg+P8 new main31, hdamp=mt
          || (m_DSID>=410501 && m_DSID<=410508) //Powheg+P8 new main31, hdamp=1.5m // Boosted samples are included 410507 410508
          || (m_DSID>=410511 && m_DSID<=410524) //Powheg+P8 new main31, hdamp=1.5mt, radiation systematics
          || (m_DSID>=410531 && m_DSID<=410535) //Powheg+P8 allhad samples
          || m_DSID==410159 || m_DSID==410160 //aMC@NLO+P8, old settings
          || (m_DSID>=410225 && m_DSID<=410227) || m_DSID==410274 || m_DSID==410275 //aMC@NLO+P8, new settings
          || m_DSID==410568 || m_DSID==410569 // nonallhad boosted c-filtered
          || m_DSID==410244 //aMC@NLO+P8, ttbb (old)
          || m_DSID==410441 || m_DSID==410442 //new aMC@NLO+P8 mc16, new shower starting scale
          || (m_DSID>=410464 && m_DSID<=410466) //new aMC@NLO+P8 mc16, new shower starting scale, no shower weights
          || (m_DSID>=410470 && m_DSID<=410472) || (m_DSID>=410480 && m_DSID<=410482) //new Powheg+P8 mc16
          || m_DSID==410452 //new aMC@NLO+P8 FxFx mc16
          || (m_DSID>=411073 && m_DSID<=411081) //Powheg+P8 HF-filtered
          || (m_DSID>=412066 && m_DSID<=412074) //aMC@NLO+P8 HF-filtered
          || (m_DSID>=411068 && m_DSID<=411070) //Powheg+P8 ttbb
          || (m_DSID>=410265 && m_DSID<=410267) //aMC@NLO+P8 ttbb
          || (m_DSID>=411178 && m_DSID<=411180) || (m_DSID==411275) //Powheg+P8 ttbb OTF production - ATLMCPROD-7240
          || (m_DSID>=407342 && m_DSID<=407344) //Powheg+P8 ttbar HT-filtered
          || (m_DSID>=407348 && m_DSID<=407350) //aMC@NLO+P8 ttbar HT-filtered
           ){
      m_GenUsed=Pythia8;
      if ( m_DSID==410244 //aMC@NLO+P8, ttbb (old)
          || (m_DSID>=411068 && m_DSID<=411070) //Powheg+P8 ttbb
          || (m_DSID>=410265 && m_DSID<=410267) //aMC@NLO+P8 ttbb
          || (m_DSID>=411178 && m_DSID<=411180) || (m_DSID==411275) //Powheg+P8 ttbb OTF production - ATLMCPROD-7240
      ){
        m_ttbb=true;
      }
    }
    // all Sherpa showered samples
    else if( (m_DSID>=410186 && m_DSID<=410189) //Sherpa 2.2.0
          || (m_DSID>=410249 && m_DSID<=410252) //Sherpa 2.2.1
          || (m_DSID>=410342 && m_DSID<=410347) //Sherpa 2.2.1 sys
          || (m_DSID>=410350 && m_DSID<=410355) //Sherpa 2.2.1 sys
          || (m_DSID>=410357 && m_DSID<=410359) //Sherpa 2.2.1 sys
          || (m_DSID>=410361 && m_DSID<=410367) //Sherpa 2.2.1 sys
          || (m_DSID>=410281 && m_DSID<=410283) //Sherpa BFilter
          || m_DSID==410051 //Sherpa ttbb (ICHEP sample)
          || (m_DSID>=410323 && m_DSID<=410325) || (m_DSID==410369) //New Sherpa 2.2.1 ttbb
          || (m_DSID>=364345 && m_DSID<=364348) //Sherpa 2.2.4 (test)
          || (m_DSID>=410424 && m_DSID<=410427) //Sherpa 2.2.4
          || (m_DSID>=410661 && m_DSID<=410664) //Sherpa 2.2.4 ttbb
          || (m_DSID>=421152 && m_DSID<=421158) //Sherpa2.2.8 ttbar
          || (m_DSID>=700051 && m_DSID<=700054) //Sherpa2.2.8 ttbb
           ){
      m_GenUsed=Sherpa;
      if( m_DSID==410051
          || (m_DSID>=410323 && m_DSID<=410325) || (m_DSID==410369)
          || (m_DSID>=410661 && m_DSID<=410664)
          || (m_DSID>=700051 && m_DSID<=700054)
        ){
        m_ttbb=true;
      }
    }
    // the default is Pythia6, so no need to list the Pythia6 showered samples
    else{
      m_GenUsed=Pythia6;
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  StatusCode HadronOriginClassifier::finalize() {
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id> HadronOriginClassifier::GetOriginMap(){
    initMaps();

    //--- map partons to hadrons
    std::vector<const xAOD::TruthParticle*> matched_partons;
    std::vector<const xAOD::TruthParticle*> matched_hadrons;

    matched_partons.clear();
    matched_hadrons.clear();

    while (matched_partons.size()<m_partonsOrigin.size() && matched_hadrons.size()<m_mainHadronMap.size()){
      float dR=999.;
      const xAOD::TruthParticle* hadron=nullptr;
      const xAOD::TruthParticle* parton=nullptr;
      for(std::map<const xAOD::TruthParticle*, HF_id>::iterator itr = m_partonsOrigin.begin(); itr!=m_partonsOrigin.end(); itr++){

        if(std::find(matched_partons.begin(), matched_partons.end(), (*itr).first) != matched_partons.end()) continue;

        TVector3 v, vtmp;
        if ((*itr).first->pt()>0.)
          v.SetPtEtaPhi((*itr).first->pt(),(*itr).first->eta(),(*itr).first->phi());
        else // Protection against FPE from eta and phi calculation
          v.SetXYZ(0.,0.,(*itr).first->pz());

        for(std::map<const xAOD::TruthParticle*, int>::iterator it = m_mainHadronMap.begin(); it!=m_mainHadronMap.end(); it++){

          if(std::find(matched_hadrons.begin(), matched_hadrons.end(), (*it).first) != matched_hadrons.end()) continue;

          if((*it).second != abs((*itr).first->pdgId()) ) continue;
          vtmp.SetPtEtaPhi((*it).first->pt(),(*it).first->eta(),(*it).first->phi());

          if(vtmp.DeltaR(v) < dR){
            dR = vtmp.DeltaR(v);
            hadron = (*it).first;
            parton = (*itr).first;
          }

        }//loop hadrons

      }//loop partons

      matched_partons.push_back(parton);
      matched_hadrons.push_back(hadron);

      m_hadronsPartons[ hadron ] = parton;
    }

    for(std::map<const xAOD::TruthParticle*, int>::iterator it = m_mainHadronMap.begin(); it!=m_mainHadronMap.end(); it++){
      const xAOD::TruthParticle* hadron = (*it).first;
      if(m_hadronsPartons.find(hadron)!=m_hadronsPartons.end()){
        m_hadronsOrigin[hadron] = m_partonsOrigin[ m_hadronsPartons[hadron] ];
      } else{
        m_hadronsOrigin[hadron] = extrajet;
      }
    }

    return m_hadronsOrigin;
  }

  //---------------------------------------------------------------------------
  void HadronOriginClassifier::initMaps(){
    m_partonsOrigin.clear();
    m_hadronsOrigin.clear();

    m_usedHadron.clear();
    m_mainHadronMap.clear();

    m_hadronsPartons.clear();

    buildPartonsHadronsMaps();
  }


  //---------------------------------------------------------------------------
  void HadronOriginClassifier::buildPartonsHadronsMaps(){

    const xAOD::TruthEventContainer* xTruthEventContainer = nullptr;
    if (evtStore()->retrieve(xTruthEventContainer,m_mcName).isFailure()) {
      ATH_MSG_WARNING("could not retrieve TruthEventContainer " <<m_mcName);
    }

    for ( const auto* truthevent : *xTruthEventContainer ) {

      for(unsigned int i = 0; i < truthevent->nTruthParticles(); i++){

        const xAOD::TruthParticle* part = truthevent->truthParticle(i);
        if(!part) continue;
        if(part->barcode() >= 200000) break;

        bool isbquark=false;
        bool iscquark=false;
        bool isHFhadron=false;

        int pdgid = abs(part->pdgId());

        //// don't loose time checking all if one found
        if(pdgid == 5 ){
          isbquark=true;
        }
        else if(pdgid == 4 ){
          iscquark=true;
        }
        else if(isBHadron(part) || isCHadron(part)){
          isHFhadron=true;
        }
        else{
          continue;
        }


        if(isbquark){
          bool islooping = isLooping(part);
          if(isDirectlyFromWTop(part, islooping)){
            m_partonsOrigin[ part ] = b_from_W;
          }
          else if(isDirectlyFromTop(part, islooping)){
            m_partonsOrigin[ part ] = b_from_top;
          }
          else if(!IsTtBb()&&(IsHerwigPP()||IsSherpa())&&isDirectlyFSR(part,islooping)){
            m_partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyFSRPythia8(part,islooping)){
            m_partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyFSRPythia6(part,islooping)){
            m_partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyMPIPythia6(part, islooping)){
            m_partonsOrigin[ part ] = b_MPI;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyMPIPythia8(part, islooping)){
            m_partonsOrigin[ part ] = b_MPI;
          }
          else if(!IsTtBb()&&IsSherpa()&&isDirectlyMPISherpa(part)){
            m_partonsOrigin[ part ] = b_MPI;
          }




        }
        if(iscquark){
          bool islooping = isLooping(part);

          if(isDirectlyFromWTop(part, islooping)){
            m_partonsOrigin[ part ] = c_from_W;
          }
          else if(isDirectlyFromTop(part, islooping)){
            m_partonsOrigin[ part ] = c_from_top;
          }
          else if(!IsTtBb()&&(IsHerwigPP()&&IsSherpa())&&isDirectlyFSR(part,islooping)){
            m_partonsOrigin[ part ] = c_FSR;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyFSRPythia8(part,islooping)){
            m_partonsOrigin[ part ] = c_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyFSRPythia6(part,islooping)){
            m_partonsOrigin[ part ] = c_FSR;
          }

          else if(!IsTtBb()&&IsPythia6()&&isDirectlyMPIPythia6(part, islooping)){
            m_partonsOrigin[ part ] = c_MPI;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyMPIPythia8(part, islooping)){
            m_partonsOrigin[ part ] = c_MPI;
          }
          else if(!IsTtBb()&&IsSherpa()&&isDirectlyMPISherpa(part)){
            m_partonsOrigin[ part ] = c_MPI;
          }



        }




        if(isHFhadron){
          if(!isCHadronFromB(part)){
            if(m_usedHadron.count(part)) continue;

            fillHadronMap(part,part);
          }
        }




      }//loop on particles
    }//loop on truthevent container



  }


  int HadronOriginClassifier::hadronType(int pdgid) {

    int rest1(abs(pdgid%1000));
    int rest2(abs(pdgid%10000));

    if ( rest2 >= 5000 && rest2 < 6000 ) return 5;
    if( rest1 >= 500 && rest1 < 600 ) return 5;

    if ( rest2 >= 4000 && rest2 < 5000 ) return 4;
    if( rest1 >= 400 && rest1 < 500 ) return 4;

    return 0;

  }


  bool HadronOriginClassifier::isBHadron(const xAOD::TruthParticle* part) {

    if(part->barcode() >= 200000) return false;
    int type = hadronType(part->pdgId());
    return type == 5;

  }


  bool HadronOriginClassifier::isCHadron(const xAOD::TruthParticle* part) {

    if(part->barcode() >= 200000) return false;
    int type = hadronType(part->pdgId());
    return type == 4;

  }


  bool HadronOriginClassifier::isQuarkFromHadron(const xAOD::TruthParticle* part) const{

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() ) continue; /// protection for sherpa
      int mothertype = hadronType( parent->pdgId() );
      if( 4 == mothertype || 5 == mothertype ){
        return true;
      }
      if(isQuarkFromHadron(parent))return true;
    }

    return false;

  }

  bool HadronOriginClassifier::isCHadronFromB(const xAOD::TruthParticle* part) const{

    if(!isCHadron(part)) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() ) continue; /// protection for sherpa
      if( isBHadron(parent) ){
        return true;
      }
      if(isCHadron(parent)){
        if(isCHadronFromB(parent))return true;
      }
    }

    return false;
  }



  void HadronOriginClassifier::fillHadronMap(const xAOD::TruthParticle* mainhad, const xAOD::TruthParticle* ihad, bool decayed){
    if (!ihad) return;

    m_usedHadron.insert(ihad);
    int parent_flav,child_flav;
    bool isFinal = true;

    if(!ihad->nChildren()) return;

    for(unsigned int j=0; j<ihad->nChildren(); ++j){
      const xAOD::TruthParticle* child = ihad->child(j);

      if(decayed){
        fillHadronMap(mainhad,child,true);
        isFinal=false;
      }
      else{
        child_flav = hadronType(child->pdgId());
        if(child_flav!=4 && child_flav!=5) continue;
        parent_flav = hadronType(mainhad->pdgId());
        if(child_flav!=parent_flav) continue;
        fillHadronMap(mainhad,child);
        isFinal=false;
      }

    }

    if(isFinal && !decayed){

      m_mainHadronMap[mainhad]=hadronType(mainhad->pdgId());

      for(unsigned int j=0; j<ihad->nChildren(); ++j){
        const xAOD::TruthParticle* child = ihad->child(j);
        fillHadronMap(mainhad,child,true);
      }
    }

  }

  //--------------------------------------------------------------------------
  bool HadronOriginClassifier::passHadronSelection(const xAOD::TruthParticle* part) const{
    double pt = part->pt();
    double eta = fabs(part->eta());

    if(pt<m_HadronPtMinCut) return false;
    if(eta>m_HadronEtaMaxCut) return false;

    return true;
  }

  //--------------------------------------------------------------------------
  bool HadronOriginClassifier::isFromTop(const xAOD::TruthParticle* part, bool looping) const{
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromTop(initpart, looping);
  }

  //--------------------------------------------------------------------------
  bool HadronOriginClassifier::isDirectlyFromTop(const xAOD::TruthParticle* part, bool looping) {
    if(!part || !part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs( parent->pdgId() ) == 6 ) return true;
    }

    return false;
  }

  //--------------------------------------------------------------------------
  bool HadronOriginClassifier::isFromWTop(const xAOD::TruthParticle* part, bool looping) const{
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromWTop(initpart, looping);
  }

  //--------------------------------------------------------------------------
  bool HadronOriginClassifier::isDirectlyFromWTop(const xAOD::TruthParticle * part, bool looping) const{
    if(!part || !part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs( parent->pdgId() ) == 24 ){
        if( isFromTop(parent, looping) ) return true;
      }
    }

    return false;


  }




  bool HadronOriginClassifier::isDirectlyFromGluonQuark(const xAOD::TruthParticle* part, bool looping) {



    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs( parent->pdgId() ) == 21 || abs(parent->pdgId())<5 ) return true;
    }

    return false;
  }

  bool HadronOriginClassifier::isFromGluonQuark(const xAOD::TruthParticle* part, bool looping) const{

    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromGluonQuark(initpart, looping);

  }



  bool HadronOriginClassifier::isDirectlyFSRPythia6(const xAOD::TruthParticle * part, bool looping) const{


    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if(abs( parent->pdgId() ) == 24){
        if(abs(part->pdgId())==4){
          //trick to get at least 50% of PowhegPythia c from FSR
          if(part->pdgId()==-(parent->pdgId())/6){
            if( isFromGluonQuark(parent, looping) ) return true;
          }
        }
        else{
          if( isFromGluonQuark(parent, looping) ) return true;
        }
      }
    }

    return false;


  }




  bool HadronOriginClassifier::isDirectlyFSR(const xAOD::TruthParticle * part, bool looping) const{


    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs(parent->pdgId())== 21 || abs(parent->pdgId())==22 ){
        if( isFromQuarkTop( parent,looping ) ) return true;
      }

    }

    return false;


  }

  bool HadronOriginClassifier::isDirectlyFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const{



    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs(parent->pdgId())<6 ) {

        if(isFromTop(parent,looping)){
          return true;
        }
        else if(isFromWTop(parent,looping)){
          return true;
        }

      }
    }

    return false;
  }

  bool HadronOriginClassifier::isFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const{

    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromQuarkTop(initpart, looping);

  }




  bool HadronOriginClassifier::isDirectlyFSRPythia8(const xAOD::TruthParticle * part, bool looping) const{


    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs(parent->pdgId())== 21 || abs(parent->pdgId())==22 ){
        if( isFromQuarkTopPythia8( parent,looping ) ) return true;
      }

    }

    return false;


  }

  bool HadronOriginClassifier::isDirectlyFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const{



    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs(parent->pdgId())<6 ) {

        if(isFromWTop(parent,looping)){
          return true;
        }

      }
    }

    return false;
  }

  bool HadronOriginClassifier::isFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const{

    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromQuarkTopPythia8(initpart, looping);

  }






  bool HadronOriginClassifier::isDirectlyMPIPythia6(const xAOD::TruthParticle * part, bool looping) {

    if(!part->nParents()) return false;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping ) continue; /// protection for sherpa
      if( abs(parent->pdgId())== 2212 && part->status()!=3) return true;

    }

    return false;

  }



  bool HadronOriginClassifier::isDirectlyMPIPythia8(const xAOD::TruthParticle * part, bool looping) const{


    const xAOD::TruthParticle* initpart = findInitial(part, looping);

    return initpart->status()>30 && initpart->status()<40;

  }

  bool HadronOriginClassifier::isDirectlyMPISherpa(const xAOD::TruthParticle * part) {

    if(!part->hasProdVtx()) return false;

    const xAOD::TruthVertex* vertex = part->prodVtx();
    return vertex->id()==2;

  }



  bool HadronOriginClassifier::isLooping(const xAOD::TruthParticle* part, std::set<const xAOD::TruthParticle*> init_part) const{

    if(!part->nParents()) return false;

    init_part.insert(part);

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( init_part.find(parent) != init_part.end() ) return true;
      if( isLooping(parent, init_part) ) return true;
    }

    return false;

  }



  const xAOD::TruthParticle*  HadronOriginClassifier::findInitial(const xAOD::TruthParticle* part, bool looping) const{


    if(!part->nParents()) return part;

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if( part->barcode() < parent->barcode() &&  looping) continue; /// protection for sherpa
      if( part->pdgId() == parent->pdgId() ){
        return findInitial(parent, looping);
      }
    }

    return part;
  }


  //--------------------------------------------------------------------------
  const xAOD::TruthParticle* HadronOriginClassifier::partonToHadron(const xAOD::TruthParticle* parton){

    const xAOD::TruthParticle* hadron(nullptr);

    TVector3 v, vtmp;
    v.SetPtEtaPhi(parton->pt(),parton->eta(),parton->phi());
    float dR=999.;

    for(std::map<const xAOD::TruthParticle*,int>::iterator it = m_mainHadronMap.begin(); it != m_mainHadronMap.end(); it++){

      //      const xAOD::TruthParticle* fhadron=(*it).first;

      if((*it).second != abs(parton->pdgId()) ) continue; // || ((fhadron->pt()/parton->pt())>1)
      vtmp.SetPtEtaPhi((*it).first->pt(),(*it).first->eta(),(*it).first->phi());

      if(vtmp.DeltaR(v) < dR){
        dR = vtmp.DeltaR(v);
        hadron = (*it).first;
      }

    }

    return hadron;
  }

}//namespace







