/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAOUTPUTSTREAMTOOL_H
#define ATHENAOUTPUTSTREAMTOOL_H
/** @file AthenaOutputStreamTool.h
 *  @brief This is the implementation of IAthenaOutputStreamTool.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

class IClassIDSvc;
class IDecisionSvc;
class IConversionSvc;
class StoreGateSvc;
class DataHeader;
namespace SG {
   class IFolder;
}

/** @class AthenaOutputStreamTool
 *  @brief This is the implementation of IAthenaOutputStreamTool.
 **/
class AthenaOutputStreamTool : public ::AthAlgTool, virtual public IAthenaOutputStreamTool {
public:
   /// Standard AlgTool Constructor
   AthenaOutputStreamTool(const std::string& type,
	   const std::string& name,
	   const IInterface* parent);
   /// Destructor
   virtual ~AthenaOutputStreamTool();

   /// AthAlgTool Interface method implementations:
   StatusCode initialize();
   StatusCode finalize();

   /// Specify which data store and conversion service to use
   /// and whether to extend provenence
   ///   Only use if one wants to override jobOptions
   StatusCode connectServices(const std::string& dataStore,
	   const std::string& cnvSvc,
	   bool extendProvenenceRecord);

   /// Connect to the output stream
   ///   Must connectOutput BEFORE streaming
   ///   Only specify "outputName" if one wants to override jobOptions
   StatusCode connectOutput(const std::string& outputName = "");

   /// Commit the output stream after having streamed out objects
   ///   Must commitOutput AFTER streaming
   StatusCode commitOutput(bool doCommit = false);

   /// Finalize the output stream after the last commit, e.g. in
   /// finalize
   StatusCode finalizeOutput();

   /// Stream out objects. Provide vector of typeName/key pairs.
   ///   If key is empty, assumes only one object and this
   ///   will fail if there is more than one
   typedef std::pair<std::string, std::string> TypeKeyPair;
   typedef std::vector<TypeKeyPair>            TypeKeyPairs;
   virtual StatusCode streamObjects(const TypeKeyPairs& typeKeys, const std::string& outputName = "");

   /// Stream out a vector of objects
   ///   Must convert to DataObject, e.g.
   ///   #include "AthenaKernel/StorableConversions.h"
   ///     T* obj = xxx;
   ///     DataObject* dataObject = SG::asStorable(obj);
   typedef std::vector<DataObject*> DataObjectVec;
   virtual StatusCode streamObjects(const DataObjectVec& dataObjects, const std::string& outputName = "");

   virtual StatusCode getInputItemList(SG::IFolder* m_p2BWrittenFromTool);

private:
   /// Do the real connection to services
   virtual StatusCode connectServices();

private:
   StringProperty  m_outputName{ this, "OutputFile", "", "name of the output db name"};
   StringProperty  m_dataHeaderKey{ this, "DataHeaderKey", "", "name of the data header key: defaults to tool name"};
   StringProperty  m_processTag{ this, "ProcessingTag", "", "tag of processing stage: defaults to SG key of DataHeader (Stream name)"};
   StringProperty  m_outputCollection{ this, "OutputCollection", "", "custom container name prefix for DataHeader: default = "" (will result in \"POOLContainer_\")"};
   StringProperty  m_containerPrefix{ this, "PoolContainerPrefix", "", "prefix for top level POOL container: default = \"CollectionTree\""};
   StringProperty  m_containerNameHint{ this, "TopLevelContainerName", "0", "naming hint policy for top level POOL container: default = \"0\""};
   StringProperty  m_branchNameHint{ this, "SubLevelBranchName", "0", "naming hint policy for POOL branching: default = \"0\"" };
   std::string  m_outputAttributes{""};
   SG::ReadHandleKey<AthenaAttributeList>  m_attrListKey{this, "AttributeListKey", "", "optional key for AttributeList to be written as part of the DataHeader: default = \"\""};
   //SG::WriteHandleKey<AthenaAttributeList>  m_attrListWrite{this, "AttributeListWrite", "", "optional key for AttributeList to be written as part of the DataHeader: default = <AttributeListKey>+\"Decisions\""};
   std::string  m_attrListWrite{""};

   ServiceHandle<StoreGateSvc>   m_store{ this, "Store", "StoreGateSvc/DetectorStore", "Pointer to the data store"};
   /// Keep reference to the data conversion service
   ServiceHandle<IConversionSvc> m_conversionSvc;
   /// Ref to ClassIDSvc to convert type name to clid
   ServiceHandle<IClassIDSvc>    m_clidSvc;
   /// Ref to DecisionSvc
   ServiceHandle<IDecisionSvc>   m_decSvc;
   /// Current DataHeader for streamed objects
   DataHeader*     m_dataHeader;
   /// Flag to tell whether connectOutput has been called
   bool            m_connectionOpen;
   /// Flag as to whether to extend provenance via the DataHeader
   bool            m_extendProvenanceRecord;
   /// Flag to extend attribute list with stream flags from DecisionSvc
   bool m_extend;
   
   /// set of skipped item keys, because of missing CLID
   std::set<std::string> m_skippedItems;
};

#endif
