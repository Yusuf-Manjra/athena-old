/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "RDBQuery.h"
#include "RDBAccessSvc.h"
#include <stdexcept>

#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/IColumn.h"

#include "RelationalAccess/SchemaException.h"
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "CxxUtils/checker_macros.h"

RDBQuery::RDBQuery(const Athena::DBLock& dblock
                   , RDBAccessSvc* accessSvc
		   , const std::string& nodeName
		   , const std::string& tagId
		   , const std::string& connName)
  : IRDBQuery()
  , m_dblock (dblock)
  , m_query(nullptr)
  , m_queryCount(nullptr)
  , m_accessSvc(accessSvc)
  , m_nodeName(nodeName)
  , m_tagId(tagId)
  , m_connName(connName)
  , m_size(0)
  , m_cursor(nullptr)
  , m_executed(false)
{
}

RDBQuery::~RDBQuery()
{
  delete m_query;
  delete m_queryCount;
}

void RDBQuery::execute()
{
  if(m_executed) {
    m_accessSvc->msg() << MSG::WARNING << "RDBQuery cannot be executed more than once! Query: "
		       << m_nodeName << ", "
		       << m_tagId << ", "
		       << m_orderField << ", "
		       << m_fields.size() << endmsg;
    return;
  }

  if(m_accessSvc->msg().level() <= MSG::DEBUG) {
    m_accessSvc->msg() << MSG::DEBUG << "Query execute " << m_nodeName << ", "
		       << m_tagId << ", "
		       << m_orderField << ", "
		       << m_fields.size() << endmsg;
  }

  if (!m_accessSvc->connect(m_connName)) {
    m_accessSvc->msg() << MSG::ERROR << "Cannot connect to the database: "
                       << m_connName << endmsg;
    throw std::runtime_error( "Cannot connect to the database " + m_connName);
  }

  m_executed=true;
  try {
    // ... Get the node name and change to to Upper Case
    std::string upperName = m_nodeName;
    for (char& ch : upperName) {
      ch = std::toupper (static_cast<unsigned int>(ch));
    }

    // ... Create query objects
    m_query = m_accessSvc->getSession(m_connName)->nominalSchema().newQuery();
    m_queryCount = m_accessSvc->getSession(m_connName)->nominalSchema().newQuery();

    // Add fields
    if(m_fields.size()>0) {
      // Custom fields
      for(unsigned int i=0; i<m_fields.size(); ++i) {
	m_query->addToOutputList(upperName+"_DATA."+m_fields[i]);
      }
    }
    else {
      // All fields from the table
      const coral::ITableDescription& dataTableDesc = m_accessSvc->getSession(m_connName)->nominalSchema().tableHandle(upperName + "_DATA").description();
      for(int i=0; i<dataTableDesc.numberOfColumns(); ++i) {
	m_query->addToOutputList(upperName+"_DATA."+dataTableDesc.columnDescription(i).name());
      }
    }

    m_queryCount->addToOutputList("COUNT("+upperName+"_DATA_ID)","SUMREC");

    // ... Define table list
    m_query->addToTableList(upperName + "_DATA");
    m_query->addToTableList(upperName + "_DATA2TAG");

    m_queryCount->addToTableList(upperName + "_DATA2TAG");    

    // ... Define order
    if(m_orderField.empty()) {
      m_query->addToOrderList(upperName + "_DATA." + upperName + "_DATA_ID");
    }
    else {
      m_query->addToOrderList(upperName + "_DATA." + m_orderField);
    }

    // ... Define conditions
    coral::AttributeList bindsData ATLAS_THREAD_SAFE;
    bindsData.extend<std::string>("tagID");

    std::string queryStructCondition = upperName +"_DATA2TAG." + upperName + "_TAG_ID =:tagID";
    m_queryCount->setCondition(queryStructCondition , bindsData);
    
    queryStructCondition += " AND " + upperName +"_DATA." + upperName + "_DATA_ID=" + upperName + "_DATA2TAG." + upperName + "_DATA_ID";
    m_query->setCondition(queryStructCondition , bindsData);

    bindsData[0].data<std::string>() = m_tagId;

    m_queryCount->defineOutputType("SUMREC","long");
    m_query->setMemoryCacheSize(1);

    // ... Get size of the result set
    coral::ICursor& cursorCount = m_queryCount->execute();
    while(cursorCount.next()) {
      // Just one iteration
      m_size = cursorCount.currentRow()["SUMREC"].data<long>();
    }

    // ... Get the data cursor
    m_cursor = &(m_query->execute());
    return;
  }
  catch(coral::SchemaException& se) {
    m_accessSvc->msg() << MSG::WARNING << "QUERY: Schema Exception : " + std::string(se.what()) << endmsg;
  }
  catch(std::exception& e) {
    m_accessSvc->msg() << MSG::WARNING << "QUERY: Exception : " + std::string(e.what()) << endmsg;
  }
  catch(...) {
    m_accessSvc->msg() << MSG::WARNING << "QUERY: Exception caught... "  << endmsg;
  }

  m_size = 0; // This indicates that there was a problem with query execution
}

long RDBQuery::size()
{
  return m_size;
}

void RDBQuery::finalize()
{
  if(m_cursor) m_cursor->close();
  m_accessSvc->disconnect(m_connName);
}

void RDBQuery::setOrder(const std::string& field)
{
  m_orderField = field;
}

void RDBQuery::addToOutput(const std::string& field)
{
  m_fields.push_back(field);
}

bool RDBQuery::next()
{
  if(m_cursor->next()) {
    m_attrList = &(m_cursor->currentRow());
    return true;
  }
  else {
    m_attrList = 0;
    return false;
  }
}
