/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/



// Local include(s):
#include "xAODJet/versions/Jet_v1.h"
#include "xAODJet/JetAccessors.h"
#include "xAODJet/FastJetLinkBase.h"

namespace xAOD {

  Jet_v1::Jet_v1()
    : IParticle() 
    , m_fastJetLink(nullptr) 
  {
  }

  static const JetAttributeAccessor::AccessorWrapper<JetFourMom_t> momentumAcc;
  
  
  Jet_v1::Jet_v1(const Jet_v1& o ) : IParticle( o )
                                   , m_fastJetLink(nullptr)
  {
    makePrivateStore( o );
    if( o.m_fastJetLink  ) m_fastJetLink = o.m_fastJetLink->clone();    
  }   

  Jet_v1& Jet_v1::operator=(const Jet_v1& o ){
    if(this == &o) return *this;

    if( ( ! hasStore() ) && ( ! container() ) ) makePrivateStore();
    this->IParticle::operator=( o );
    if( (o.m_fastJetLink) && (m_fastJetLink==nullptr) ) m_fastJetLink = o.m_fastJetLink->clone();    
    return *this;
  }

  Jet_v1::~Jet_v1(){
    if(m_fastJetLink) delete m_fastJetLink;
  }


   double Jet_v1::pt() const {

     return momentumAcc.pt( *this );
   }

   double Jet_v1::eta() const {
     
     return momentumAcc.eta( *this );
   }

   double Jet_v1::phi() const {

     return momentumAcc.phi( *this );
   }

   double Jet_v1::m() const {
     return momentumAcc.m( *this );
   }

   double Jet_v1::e() const {
     return jetP4().E();
   }

   double Jet_v1::rapidity() const {
     return jetP4().Rapidity();
   }

   Jet_v1::FourMom_t Jet_v1::p4() const {
     JetFourMom_t p = jetP4();
     return FourMom_t(p.Px(), p.Py(), p.Pz(), p.E() );
   }

   JetFourMom_t Jet_v1::jetP4() const {
     return momentumAcc.getAttribute( *this);
   }

  
   JetFourMom_t Jet_v1::genvecP4() const {
     return jetP4();
   }

  Type::ObjectType Jet_v1::type() const {    
      return Type::Jet;
   }

  /// Assuming that when one of px py pz is called the other will be too. So provoke the build of FourMom_t
   float Jet_v1::px() const {

     return p4().Px();
   }
   float Jet_v1::py() const {

     return p4().Py();     
   }

   float Jet_v1::pz() const {

     return p4().Pz();
   }


  /// The accessor for the cluster element links
  static const SG::AuxElement::Accessor< std::vector< ElementLink< IParticleContainer > > >
      constituentAcc( "constituentLinks" );
  static const SG::AuxElement::Accessor< std::vector< float> >
      constituentWeightAcc( "constituentWeights" );

  void Jet_v1::addConstituent( const ElementLink< IParticleContainer >& link,
                               float w ) {

    constituentAcc( *this ).push_back( link );
    constituentWeightAcc( *this ).push_back( w );
    return;
  }

  void Jet_v1::addConstituent( const IParticle* c, float w ) {
    int idx = c->index();
    const IParticleContainer* pcon =
      dynamic_cast<const IParticleContainer*>(c->container());
    ElementLink<IParticleContainer> el(*pcon, idx);
#if 0
    ElementLink< IParticleContainer > el;
    el.toIndexedElement( *( dynamic_cast< const IParticleContainer* >( c->container() ) ), c->index() );
#endif
    constituentAcc(*this).push_back(el);
    constituentWeightAcc( *this ).push_back( w );
    return;
  }


  static const SG::AuxElement::Accessor< int > constitScaleAcc( "ConstituentScale" );

  JetConstitScale Jet_v1::getConstituentsSignalState() const {
    if( constitScaleAcc.isAvailable(*this) ) return (JetConstitScale)constitScaleAcc(*this);
    return UnknownConstitScale; 
  }

  void Jet_v1::setConstituentsSignalState( JetConstitScale t){
    constitScaleAcc(*this) = t;
  }



  JetConstituentVector Jet_v1::getConstituents() const {
    
    JetConstituentVector vec( & constituentAcc(*this) , getConstituentsSignalState() );
    return vec;
  }

  size_t Jet_v1::numConstituents() const {

     return constituentAcc( *this ).size();
  }

  const IParticle* Jet_v1::rawConstituent(size_t i) const {
    return * ( constituentAcc(*this)[i]);
  }

  const std::vector< ElementLink< IParticleContainer > >& Jet_v1::constituentLinks() const {
    return constituentAcc(*this);
  }


  ////////////////////////////////////////////////////////


  
  void Jet_v1::setJetP4( const JetFourMom_t& p4 ) {
    
    momentumAcc.setAttribute( *this, p4 );
    return;
  }

  static const JetAttributeAccessor::AccessorWrapper< JetFourMom_t > jetEMScaleAcc("JetEMScaleMomentum");
  static const JetAttributeAccessor::AccessorWrapper< JetFourMom_t > jetConstitScaleAcc("JetConstitScaleMomentum");
 
  /// Access by the enum to the historical states
  JetFourMom_t Jet_v1::jetP4(JetScale s) const {
   switch( s ){ 
    case JetEMScaleMomentum: return jetEMScaleAcc.getAttribute( *this );
    case JetConstitScaleMomentum : return jetConstitScaleAcc.getAttribute( *this );
    case JetAssignedScaleMomentum: return jetP4();
    default :
      std::cerr << " Error p4(P4SignalState) unknown state "<< s << std::endl;
      break;
    }
    return JetFourMom_t();
  }

  JetFourMom_t Jet_v1::genvecP4(JetScale s) const {
    return jetP4(s);
  }

  /// Access by the enum to the historical states
  void Jet_v1::setJetP4(JetScale s, const JetFourMom_t &p4)  {
    switch( s ){ 
    case JetEMScaleMomentum: 
      jetEMScaleAcc.setAttribute( *this, p4 );
      break;
    case JetConstitScaleMomentum: 
      jetConstitScaleAcc.setAttribute(*this, p4 ) ;
      break;
    case JetAssignedScaleMomentum:
      setJetP4(p4);
      break;
    default :
      std::cerr << " Error p4(P4SignalState) unknown state "<< s << std::endl;
      break;
    }
  }

  /// Generic access to states
  JetFourMom_t Jet_v1::jetP4(const std::string& statename) const {
    if(statename=="JetAssignedScaleMomentum") return jetP4();
    return getAttribute<JetFourMom_t>(statename);
  }

  JetFourMom_t Jet_v1::genvecP4(const std::string& statename) const {
    return jetP4(statename);
  }

  void Jet_v1::setJetP4(const std::string& statename, const JetFourMom_t &p4)  {
    if(statename=="JetAssignedScaleMomentum") return setJetP4(JetAssignedScaleMomentum,p4);
    return setAttribute<JetFourMom_t>(statename, p4);
  }


  void Jet_v1::reset() {
    if(m_fastJetLink) delete m_fastJetLink; 
    m_fastJetLink = nullptr;
  }

  const fastjet::PseudoJet * Jet_v1::getPseudoJet() const {
    if(m_fastJetLink) return m_fastJetLink->pseudoJet;
    return nullptr;
  }

  static const SG::AuxElement::Accessor<int> inputAcc("InputType");
  static const SG::AuxElement::Accessor<int> algAcc("AlgorithmType");
  static const SG::AuxElement::Accessor<float> spAcc("SizeParameter");

  float Jet_v1::getSizeParameter() const {
    return spAcc(*this);
  }
  
  JetAlgorithmType::ID Jet_v1::getAlgorithmType()const {
    return (JetAlgorithmType::ID) algAcc(*this);
  }

  JetInput::Type Jet_v1::getInputType()const {
    return (JetInput::Type) inputAcc(*this);
  }

  void Jet_v1::setSizeParameter(float p)  { spAcc(*this) = p;}
  void Jet_v1::setAlgorithmType(JetAlgorithmType::ID a)  { algAcc(*this) = a;}
  void Jet_v1::setInputType(JetInput::Type t)  {inputAcc(*this)=t;}


//   void Jet_v1::toPersistent() {
//   No longer needed, as this is done by the POOL converter for the aux container

//     if( constituentAcc.isAvailableWritable( *this ) ) { 
//        std::vector< ElementLink< IParticleContainer > >::iterator itr =
//           constituentAcc( *this ).begin();
//        std::vector< ElementLink< IParticleContainer > >::iterator end =
//           constituentAcc( *this ).end();
//        for( ; itr != end; ++itr ) {
//           itr->toPersistent();
//        }
//     }

// #ifndef SIMULATIONBASE
//     // On a new object we could only create a link with the new name, so:
//     if( btagAcc2.isAvailableWritable( *this ) ) {
//       btagAcc2( *this ).toPersistent();
//     }
// #endif //SIMULATIONBASE

    
//     return;
//   }
  

} // namespace xAOD
