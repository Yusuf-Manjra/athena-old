/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "AmdcOracle/AmdcDumpOracle.h"

#include "AmdcDb/AmdcDbRecord.h"

#include "AmdcDb/AmdcDb2Sql.h"

#include <fstream>

AmdcDumpOracle::AmdcDumpOracle(const std::string& name, ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator)
  , m_amdcDb ( "AmdcDb",name )
{

//Set Default values
   m_KountCallsDoIt   = 0 ;

//Declare the properties
   declareProperty("SwitchOff"     , m_SwitchOff    = 1 ) ;
   declareProperty("WarningLevel"  , m_WarningLevel = 1 ) ;
   declareProperty("EmergencyOut"  , m_EmergencyOut = 0 ) ;
   declareProperty("AsNewTableOn"  , m_AsNewTableOn = 0 ) ;
   declareProperty("UseKeysOn"     , m_UseKeysOn    = 1 ) ;

 }
 
// Initialize
StatusCode AmdcDumpOracle::initialize(){

  ATH_MSG_INFO( "Initialisation started     " ) ;


  ATH_MSG_INFO( "================================" ) ;
  ATH_MSG_INFO( "=Proprieties are     " ) ;
  if (m_SwitchOff == 0) {
    ATH_MSG_INFO( "= WarningLevel       " << m_WarningLevel      ) ;
    ATH_MSG_INFO( "= EmergencyOut       " << m_EmergencyOut      ) ;
    ATH_MSG_INFO( "= AsNewTableOn       " << m_AsNewTableOn      ) ;
    ATH_MSG_INFO( "= UseKeysOn          " << m_UseKeysOn      ) ;
  }
  ATH_MSG_INFO( "= SwitchOff          " << m_SwitchOff          ) ;
  ATH_MSG_INFO( "================================" ) ;

  if (m_SwitchOff == 0) {
    ATH_CHECK(m_amdcDb.retrieve());
    ATH_CHECK(DoIt());
  }

  ATH_MSG_INFO( "Initialisation ended     " ) ;
  if ( m_EmergencyOut == 1 ) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;

}

StatusCode AmdcDumpOracle::DoIt()
{
  ATH_MSG_INFO( "DoIt called     " ) ;  

  if (m_SwitchOff == 0) {
  
//  Count calls
    m_KountCallsDoIt = m_KountCallsDoIt + 1 ;
    std::string KountCallsDoItASstring = "" ;
    std::stringstream KountCallsDoItASstringstream;
    KountCallsDoItASstringstream << m_KountCallsDoIt ;
    if (m_KountCallsDoIt > 1 ) KountCallsDoItASstring = KountCallsDoItASstringstream.str();

//  Control ouput names
    std::string StringBidon = "";

//  Outout sql files
    AmdcDb2Sql aAmdcDb2Sql;
    aAmdcDb2Sql.SetAsNewTableOn(m_AsNewTableOn);
    aAmdcDb2Sql.SetUseKeysOn(m_UseKeysOn);
    
    StringBidon = "Out.AmdcOracle.DB" + KountCallsDoItASstring ;
    aAmdcDb2Sql.DoIt(StringBidon,"RDB",m_amdcDb.get());
    
    StringBidon = "Out.AmdcOracle.AM" + KountCallsDoItASstring ;
    aAmdcDb2Sql.DoIt(StringBidon,"Amdc",m_amdcDb.get());

//  Check contents
    StringBidon = "Out.AmdcOracle" + KountCallsDoItASstring ;
    std::ofstream OutFile;
    OutFile.open(StringBidon.c_str());
    OutFile << setiosflags(std::ios::fixed);
    
    int Kwarn = 0 ;
    int Kchck = 0 ;
    int KwarnTot = 0 ;
    int KchckTot = 0 ;

//  Check ASZT
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpASZT(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ASZT: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;
    
//  Check ISZT
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpISZT(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ISZT: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;
    
//  Check APTP
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpAPTP(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "APTP: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ALMN
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpALMN(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ALMN: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ALIN
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpALIN(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ALIN: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WMDT
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWMDT(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WMDT: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WSPA
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWSPA(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WSPA: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WSUP
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWSUP(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WSUP: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WCHV
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWCHV(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WCHV: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WCMI
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWCMI(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WCMI: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WCRO
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWCRO(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WCRO: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WLBI
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWLBI(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WLBI: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WDED
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWDED(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WDED: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ASMP
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpASMP(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ASMP: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check DBAM
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpDBAM(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "DBAM: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WCSC
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWCSC(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WCSC: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ATLN
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpATLN(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ATLN: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check GGLN
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpGGLN(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "GGLN: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WTGC
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWTGC(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WTGC: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check AWLN
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpAWLN(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "AWLN: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check WRPC
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpWRPC(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "WRPC: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ACUT
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpACUT(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ACUT: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check ATYP
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpATYP(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "ATYP: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check AMDC
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpAMDC(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "AMDC: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check HwSwIdMapping
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpHwSwIdMapping(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "HwSwIdMapping: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

//  Check XtomoData
    Kwarn = 0 ;
    Kchck = 0 ;
    DumpXtomoData(m_amdcDb.get(),KountCallsDoItASstring,Kwarn,Kchck);
    KwarnTot = KwarnTot + Kwarn ;
    KchckTot = KchckTot + Kchck ;
    OutFile 
            << "XtomoData: " << Kwarn << " Warnings " << " for " << Kchck << " tested elements " 
            << std::endl;

    OutFile 
            << "Summary: " << KwarnTot << " Warnings " << " for " << KchckTot << " tested elements " 
            << std::endl;

    OutFile.close();

  }

  return StatusCode::SUCCESS;

}

void AmdcDumpOracle::DumpASZT( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ASZT";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("TYP",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JFF",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JZZ",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JOB",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpISZT( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ISZT";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("TYP",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JFF",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JZZ",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JOB",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JLAY",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpAPTP( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){\

  std::string NameOfTheSet = "APTP";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      for (int DB_JFF=1; DB_JFF<=8 ; ++DB_JFF){
        int DB_JFF_Shifted = DB_JFF - 1 ;
        std::ostringstream Aostringstream;
        Aostringstream << DB_JFF_Shifted ;
        std::string DbVar    = "IPHI_"+Aostringstream.str();
        if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable(DbVar,pAmdcDbRecordFromRDB);
      }
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("TYP",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("IZ",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpALMN( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ALMN";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JTYP",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("INDX",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JOB",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpALIN( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ALIN";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
//DRING: here, because sub line is missing, there is no other way than going assuming that tables are parallel
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end() && itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromRDB,++itFromAmdc){

    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
    if (pAmdcDbRecordFromAmdc == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
      return;
    } 

    pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

  }


  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWMDT( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WMDT";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("IW",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWSPA( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WSPA";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWSUP( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WSUP";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWCHV( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WCHV";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWCMI( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WCMI";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWCRO( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WCRO";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWLBI( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WLBI";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWDED( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WDED";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpASMP( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ASMP";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("INDX",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JTYP",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);
  
    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpDBAM( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "DBAM";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;

      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 
     
      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWCSC( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WCSC";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpATLN( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ATLN";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);
      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("I",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpGGLN( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "GGLN";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWTGC( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WTGC";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpAWLN( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "AWLN";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JSTA",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpWRPC( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "WRPC";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;
     
      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);
 
    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpACUT( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ACUT";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
//DRING: here, because JTYP is missing, there is no other way than going assuming that tables are parallel
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end() && itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromRDB,++itFromAmdc){

    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
    if (pAmdcDbRecordFromAmdc == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
      return;
    } 

    pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpATYP( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "ATYP";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("JTYP",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpAMDC( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "AMDC";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpHwSwIdMapping( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "HwSwIdMapping";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("HARDNAME",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}

void AmdcDumpOracle::DumpXtomoData( IRDBAccessSvc* pIRDBAccessSvc, const std::string& TagAdd, int& Kwarn,int& Kchck){

  std::string NameOfTheSet = "XtomoData";

  std::ofstream OutFile;
  std::string FileName = "Out.AmdcOracle."+ TagAdd + NameOfTheSet;
  OutFile.open(FileName.c_str());
  OutFile << setiosflags(std::ios::fixed);
  OutFile << "Checking " << NameOfTheSet << " content started" << std::endl;
  Kwarn = -1 ;
  Kchck = 0 ;
  
  IRDBRecordset_ptr pIRDBRecordsetFromRDB = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"RDB");
  if (pIRDBRecordsetFromRDB->size() == 0){
    OutFile << "No " << NameOfTheSet << " in RDB " << std::endl;
    return;
  } 
  
  IRDBRecordset_ptr pIRDBRecordsetFromAmdc = pIRDBAccessSvc->getRecordsetPtr(NameOfTheSet,"Amdc");
  if (pIRDBRecordsetFromAmdc->size() == 0){
    OutFile << "No " << NameOfTheSet << " in Amdc " << std::endl;
    return;
  } 
  
  Kwarn = 0 ;
  
  std::vector<IRDBRecord*>::const_iterator itFromRDB = pIRDBRecordsetFromRDB->begin();
  for ( ; itFromRDB<pIRDBRecordsetFromRDB->end(); ++itFromRDB){
    const AmdcDbRecord* pAmdcDbRecordFromRDB = dynamic_cast<const AmdcDbRecord*>((*itFromRDB));
    if (pAmdcDbRecordFromRDB == nullptr){
      OutFile << "No way to cast pAmdcDbRecordFromRDB for " << NameOfTheSet << std::endl;
      return;
    } 
    int Ifound = 0 ;
    std::vector<IRDBRecord*>::const_iterator itFromAmdc = pIRDBRecordsetFromAmdc->begin();
    for ( ; itFromAmdc<pIRDBRecordsetFromAmdc->end(); ++itFromAmdc){
      if (Ifound==1) continue;
    
      Ifound = 1;

      const AmdcDbRecord* pAmdcDbRecordFromAmdc = dynamic_cast<const AmdcDbRecord*>((*itFromAmdc));
      if (pAmdcDbRecordFromAmdc == nullptr){
        OutFile << "No way to cast pAmdcDbRecordFromAmdc for " << NameOfTheSet << std::endl;
        return;
      } 

      if (Ifound == 1) Ifound = pAmdcDbRecordFromAmdc->doWeMatchForThisVariable("XtomoChberName",pAmdcDbRecordFromRDB);

      if (Ifound == 1) pAmdcDbRecordFromAmdc->doIMatch(m_WarningLevel,Kwarn,Kchck,OutFile,pAmdcDbRecordFromRDB);

    }

    if (Ifound!=1){
      Kwarn = Kwarn + 1;
      OutFile << "WARNING: No way to check " << pAmdcDbRecordFromRDB->getUniversalId() << std::endl;
    }

  }

  OutFile << "Checking " << NameOfTheSet << " content ended" << std::endl;

  OutFile.close();
  
}
