/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloJiveXML/CaloFCalRetriever.h"

#include "AthenaKernel/Units.h"

#include "EventContainers/SelectAllObject.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArRawChannel.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "Identifier/HWIdentifier.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  CaloFCalRetriever::CaloFCalRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("FCAL"),
    m_calocell_id(nullptr),
    m_sgKey ("AllCalo")
  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    declareInterface<IDataRetriever>(this);
    declareProperty("StoreGateKey" , m_sgKey);
    declareProperty("FCallCellThreshold", m_cellThreshold = 50.);
    declareProperty("RetrieveFCal" , m_fcal = true);
    declareProperty("DoBadFCal",     m_doBadFCal = false);
    declareProperty("DoFCalCellDetails",  m_doFCalCellDetails = false); 
    declareProperty("CellConditionCut", m_cellConditionCut = false);
    declareProperty("LArChannelsToIgnoreM5",  m_LArChannelsToIgnoreM5);
    declareProperty("DoMaskLArChannelsM5", m_doMaskLArChannelsM5 = false);

    declareProperty("CellEnergyPrec", m_cellEnergyPrec = 3);
    declareProperty("CellTimePrec", m_cellTimePrec = 3);
  }

  /**
   * Initialise the ToolSvc
   */

  StatusCode CaloFCalRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool"  );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );

    ATH_CHECK( m_cablingKey.initialize() );
    ATH_CHECK( m_adc2mevKey.initialize(m_doFCalCellDetails) );

    return StatusCode::SUCCESS;	
  }
   
  /**
   * FCal data retrieval from default collection
   */
  StatusCode CaloFCalRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieve()"  );

    const CaloCellContainer* cellContainer;
    if ( !evtStore()->retrieve(cellContainer,m_sgKey))
      {
	ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
	return StatusCode::FAILURE;
      }

   if(m_fcal){
      DataMap data = getFCalData(cellContainer);
      ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKey, &data) );
      ATH_MSG_DEBUG( "FCal retrieved"  );
    }
    //FCal cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve FCal cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloFCalRetriever::getFCalData(const CaloCellContainer* cellContainer) {
    
    ATH_MSG_DEBUG( "getFCalData()"  );
    const EventContext& ctx = Gaudi::Hive::currentContext();

    DataMap DataMap;

    DataVect x; x.reserve(cellContainer->size());
    DataVect y; y.reserve(cellContainer->size());
    DataVect z; z.reserve(cellContainer->size());
    DataVect dx; dx.reserve(cellContainer->size());
    DataVect dy; dy.reserve(cellContainer->size());
    DataVect dz; dz.reserve(cellContainer->size());

    DataVect energy; energy.reserve(cellContainer->size());
    DataVect idVec; idVec.reserve(cellContainer->size());
    DataVect channel; channel.reserve(cellContainer->size());
    DataVect feedThrough; feedThrough.reserve(cellContainer->size());
    DataVect slot; slot.reserve(cellContainer->size());
    DataVect sub; sub.reserve(cellContainer->size());

    DataVect cellTimeVec; cellTimeVec.reserve(cellContainer->size());
    DataVect cellGain; cellGain.reserve(cellContainer->size());
    DataVect cellPedestal; cellPedestal.reserve(cellContainer->size());
    DataVect adc2Mev; adc2Mev.reserve(cellContainer->size());
    DataVect BadCell; BadCell.reserve(cellContainer->size());

    char rndStr[30]; // for rounding (3 digit precision)

    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::LARFCAL);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::LARFCAL);

    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    const LArOnOffIdMapping* cabling{*cablingHdl};

    if(!cabling) {
       ATH_MSG_WARNING( "Do not have cabling mapping from key " << m_cablingKey.key() );
       return DataMap;
    }

    const ILArPedestal* larPedestal = nullptr;
    if(m_doFCalCellDetails){
	if( detStore()->retrieve(larPedestal).isFailure() ){
	  ATH_MSG_ERROR( "in getFCalData(), Could not retrieve LAr Pedestal"  );
	}
      }
      
    const LArOnlineID* onlineId;
    if ( detStore()->retrieve(onlineId, "LArOnlineID").isFailure()) {
      ATH_MSG_ERROR( "in getFCalData(),Could not get LArOnlineID!"  );
     }
    
    const LArADC2MeV* adc2mev = nullptr;
    if (m_doFCalCellDetails) {
      SG::ReadCondHandle<LArADC2MeV> adc2mevH (m_adc2mevKey, ctx);
      adc2mev = *adc2mevH;
    }

      double energyGeV, xmm, ymm, zmm, dxmm, dymm, dzmm, cellTime;
      double energyAllLArFcal = 0.;

      for(;it1!=it2;++it1){

      if((*it1)->badcell()) BadCell.push_back(1);
      else if((*it1)->energy()>= m_cellThreshold) BadCell.push_back(0);
      else BadCell.push_back(-1);

	  if ((((*it1)->provenance()&0xFF)!=0xA5)&&m_cellConditionCut) continue; // check full conditions for FCal
	  Identifier cellid = (*it1)->ID(); 

          HWIdentifier LArhwid = cabling->createSignalChannelIDFromHash((*it1)->caloDDE()->calo_hash());
	  
	  //ignore FCal cells that are to be masked
	  if (m_doMaskLArChannelsM5){
	    bool maskChannel = false;
	    for (size_t i = 0; i < m_LArChannelsToIgnoreM5.size(); i++){
              if (cellid == m_LArChannelsToIgnoreM5[i]){
		maskChannel = true; 
		break;  // exit loop over bad channels
	      }	      
	    }
	    if (maskChannel) continue;  // continue loop over all channels
	  }

	  energyGeV = (*it1)->energy()*(1./GeV);
	  energy.push_back(DataType( gcvt( energyGeV, m_cellEnergyPrec, rndStr) ));
          energyAllLArFcal += energyGeV;
          idVec.push_back(DataType((Identifier::value_type)(*it1)->ID().get_compact() ));
        
	  xmm = (*it1)->x()*0.1;
	  ymm = (*it1)->y()*0.1;
	  zmm = (*it1)->z()*0.1;
	  x.push_back(DataType( gcvt( xmm, 4, rndStr)  ));
	  y.push_back(DataType( gcvt( ymm, 4, rndStr)  ));
	  z.push_back(DataType( gcvt( zmm, 4, rndStr)  ));
	  
          channel.push_back(DataType(onlineId->channel(LArhwid))); 
          feedThrough.push_back(DataType(onlineId->feedthrough(LArhwid))); 
       	  slot.push_back(DataType(onlineId->slot(LArhwid))); 

	  if ( m_doFCalCellDetails){
	    cellTime = (*it1)->time();
	    cellTimeVec.push_back(DataType( gcvt( cellTime, m_cellTimePrec, rndStr) ) );
	    cellGain.push_back(DataType( (*it1)->gain() ) ); 
      	
	    int fcalgain = (*it1)->gain();
	    float pedestal=larPedestal->pedestal(LArhwid,fcalgain);
	    float pedvalue=0;
	    if (pedestal >= (1.0+LArElecCalib::ERRORCODE)) pedvalue = pedestal;
	    else pedvalue = 0;
	    cellPedestal.push_back(DataType(pedvalue));
	         
            LArVectorProxy polynom_adc2mev = adc2mev->ADC2MEV(cellid,fcalgain);
            if (polynom_adc2mev.size()==0){ adc2Mev.push_back(DataType(-1)); }
            else{ adc2Mev.push_back(DataType(polynom_adc2mev[1])); }
	  }

	  const CaloDetDescrElement* elt = (*it1)->caloDDE();

	  dxmm = elt->dx()*0.1;
	  dymm = elt->dy()*0.1;
	  dzmm = elt->dz()*0.1;
	  dx.push_back(DataType( gcvt( dxmm, 4, rndStr)  ));
	  dy.push_back(DataType( gcvt( dymm, 4, rndStr)  ));
	  dz.push_back(DataType( gcvt( dzmm, 4, rndStr)  ));
	    
	  if(m_calocell_id->pos_neg(cellid)==2)
	    sub.push_back(DataType(1));
	  else
	    sub.push_back(DataType(0));
      }

    ATH_MSG_DEBUG( " Total energy in FCAL (LAr) in GeV : " <<  energyAllLArFcal  );

    // write values into DataMap
    DataMap["x"] = x;
    DataMap["y"] = y;
    DataMap["z"] = z;
    DataMap["dx"] = dx;
    DataMap["dy"] = dy;
    DataMap["dz"] = dz;
    DataMap["energy"] = energy;
    DataMap["id"] = idVec;
    DataMap["channel"] = channel;
    DataMap["feedThrough"] = feedThrough;
    DataMap["slot"] = slot;
    //Bad Cells
    if (m_doBadFCal==true) {
      DataMap["BadCell"] = BadCell;
    }    DataMap["sub"] = sub;

    // adc counts
    if ( m_doFCalCellDetails){
       DataMap["cellTime"] = cellTimeVec;
       DataMap["cellGain"] = cellGain;
       DataMap["cellPedestal"] = cellPedestal;
       DataMap["adc2Mev"] = adc2Mev;
    }
    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << x.size() << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // getFCalData

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
