# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

import os
import json
import six
import xml.etree.ElementTree as ET
from collections import OrderedDict as odict
import coral

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerConfigAccessBase.py')

def getFileType(filename):
    filetype = "unknown"
    with open(filename, 'r') as fp:
        config = json.load(fp)
        filetype = config['filetype']
    return filetype

from enum import Enum
class ConfigType(Enum):
    NONE = ("Config", "None")
    L1MENU = ("L1Menu", "l1menu")
    HLTMENU = ("HLTMenu", "hltmenu")
    L1PS = ("L1PrescalesSet", "l1prescale")
    HLTPS = ("HLTPrescalesSet", "hltprescale")
    BGS = ("L1BunchGroupsSet", "bunchgroupset")
    HLTJO = ("HLTJobOptions", "joboptions")
    HLTMON = ("HLTMonitoring", "hltmonitoringsummary")
    def __init__(self, basename, filetype):
        self.basename = basename
        self.filetype = filetype
    def __eq__(self, other):
        if isinstance(other,six.string_types):
            return self.filetype == other
        else:
            return self.filetype == other.filetype
    def __ne__(self, other):
        return not self.__eq__(other)

class ConfigLoader(object):
    """ 
    ConfigLoader derived classes hold the information of the configuration source
    and define the method to load the configuration
    """ 
    def __init__(self,configType):
        self.configType = configType
    def confirmConfigType(self,config):
        """
        checks that the in-file specification of the configuration type matches the expected type
        """
        if config['filetype'] != self.configType:
            raise RuntimeError("Can not load file with filetype '%s' when expecting '%s'" % (config['filetype'], self.configType.filetype))
        

class ConfigFileLoader(ConfigLoader):
    def __init__(self, configType, filename ):
        super(ConfigFileLoader,self).__init__(configType) 
        self.filename = filename
    def load(self):
        with open(self.filename, 'r') as fp:
            config = json.load(fp, object_pairs_hook = odict)
            self.confirmConfigType(config)
        return config
    def setQuery(self, query):
        pass
    def getWriteFilename(self):
        outfn = os.path.basename(self.filename)
        if outfn.endswith(".json"):
            outfn = outfn.rsplit('.',1)[0]
        return outfn + ".out.json"

class ConfigDirectLoader(ConfigLoader):
    """Class to load from json string"""
    def __init__(self, configType,  jsonString):
        super(ConfigDirectLoader,self).__init__(configType) 
        self.jsonString = jsonString
    def load(self):
        config = json.loads(self.jsonString, object_pairs_hook = odict)
        self.confirmConfigType(config)
        return config
    def setQuery(self, query):
        pass
    def getWriteFilename(self):
        pass

class ConfigDBLoader(ConfigLoader):
    def __init__(self, configType, dbalias, dbkey):
        super(ConfigDBLoader,self).__init__(configType)
        self.dbalias = dbalias
        self.dbkey = dbkey
        self.query = None
        self.schema = None

    def setQuery(self, query):
        """
        query template is a dictionary of queries, identified by schema version, 
        similar to TrigConf::TrigDBMenuLoader::m_hltQueries and TrigConf::TrigDBMenuLoader::m_l1Queries
        """
        self.query = query

    @staticmethod
    def getResolvedFileName(filename, pathenv=""):
        """ looks for file, first absolute, then by resolving envvar pathenv"""
        if os.access(filename,os.R_OK):
            return filename
        pathlist = os.getenv(pathenv,'').split(os.pathsep)
        for path in pathlist:
            f = os.path.join( path, filename )
            if os.access( f, os.R_OK ):
                return f
        raise RuntimeError("Can't read file %s, neither locally nor in %s" % (filename, pathenv) )

    @staticmethod
    def getConnectionParameters(dbalias):
        dblookupFile = ConfigDBLoader.getResolvedFileName("dblookup.xml", "CORAL_DBLOOKUP_PATH")
        dbp = ET.parse(dblookupFile)
        listOfServices = None
        for logSvc in dbp.iter("logicalservice"):
            if logSvc.attrib["name"] != dbalias:
                continue
            listOfServices = [ serv.attrib["name"] for serv in logSvc.iter("service") ]
            if len(listOfServices) == 0:
                raise RuntimeError("DB %s has no services listed in %s" % (dbalias, dblookupFile))
            break
        if listOfServices is None:
            raise RuntimeError("DB %s not available in %s" % (dbalias, dblookupFile))
        # now get the account and pw for oracle connections
        credentials = odict().fromkeys(listOfServices)

        for svc in filter(lambda s : s.startswith("frontier:"), listOfServices):
            credentials[svc] = dict()
            credentials[svc]["user"] = svc
            credentials[svc]["password"] = ""

        try:
            authFile = ConfigDBLoader.getResolvedFileName("authentication.xml", "CORAL_AUTH_PATH")
        except Exception as e:
            log.warning("File authentication.xml is not available! Oracle connection cannot be established. Exception message is: {0}".format(e))
        else:
            for svc in filter(lambda s : s.startswith("oracle:"), listOfServices):
                ap = ET.parse(authFile)
                count = 0
                for con in filter( lambda c: c.attrib["name"]==svc, ap.iter("connection")):
                    credentials[svc] = dict([(par.attrib["name"],par.attrib["value"]) for par in con])
                    count += 1
                if count==0:
                    raise RuntimeError("No credentials found for connection %s from service %s for db %s" % (con,svc,dbalias))
                if count>1:
                    raise RuntimeError("More than 1 connection found in %s for service %s" % (authFile, svc))
        
        return credentials

    @staticmethod
    def getSchema(connStr):
        ''' Read schema from connection string '''
        if connStr.startswith("oracle:"):
            [_, schema] = connStr.split("/")[-2:]
            return schema

        if connStr.startswith("frontier:"):
            import re
            pattern = r"frontier://ATLF/\(\)/(.*)"
            m = re.match(pattern, connStr)
            if not m:
                raise RuntimeError("connection string '%s' doesn't match the pattern '%s'?" % (connStr, pattern))
            (schema, ) = m.groups()
            return schema

        if connStr.startswith("sqlite_file:"):
            raise NotImplementedError("Python-loading of trigger configuration from sqlite has not yet been implemented")

    @staticmethod
    def readSchemaVersion(qdict, session):
        ''' Read schema version form database, based on TrigConf::TrigDBLoader::schemaVersion '''
        try:
            q = "SELECT TS_TAG FROM {schema}.TRIGGER_SCHEMA TS"
            query = ConfigDBLoader.getCoralQuery(session, q.format(**qdict))
            cursor = query.execute()
            cursor.next()

            versionTag = cursor.currentRow()[0].data()

            versionTagPrefix = "Trigger-Run3-Schema-v"
            if not versionTag.startswith(versionTagPrefix):
                raise RuntimeError( "Tag format error: Trigger schema version tag {0} does not start with {1}".format(versionTag, versionTagPrefix))   

            vstr = versionTag[len(versionTagPrefix)]

            if not vstr.isdigit():
                raise RuntimeError( "Invalid argument when interpreting the version part {0} of schema tag {1} is {2}".format(vstr, versionTag, type(vstr))) 

            log.info("Found schema version {0}".format(vstr))
            return int(vstr)

        except Exception as e:
            log.warning("Failed to read schema version: {0}".format(e))

    @staticmethod
    def getCoralQuery(session, queryStr):
        ''' Parse output, tables and contidion from the query string into coral query object'''
        query = session.nominalSchema().newQuery()

        output = queryStr.split("SELECT")[1].split("FROM")[0]
        query.addToOutputList(output)

        log.info("Conversion for Coral of query: {0}".format(queryStr))

        for table in queryStr.split("FROM")[1].split("WHERE")[0].split(","):
            tableSplit = list(filter(None, table.split(" ")))
            # Schema name is stripped from TableList in Coral query
            query.addToTableList(tableSplit[0].split(".")[1], tableSplit[1])

        if "WHERE" in queryStr:
            query.setCondition(queryStr.split("WHERE")[1], coral.AttributeList())

        return query

    def getQueryDefinition(self, schemaVersion):
        '''Choose query based on schema version, based on TrigConf::TrigDBLoader::getQueryDefinition '''
        maxDefVersion = 0
        for vkey in self.query.keys():
            if vkey>maxDefVersion and vkey<=schemaVersion:
                maxDefVersion = vkey

        if maxDefVersion == 0:
            raise RuntimeError("No query available for schema version {0}".format(schemaVersion))

        return self.query[maxDefVersion]

    def load(self):
        credentials = ConfigDBLoader.getConnectionParameters(self.dbalias)

        svc = coral.ConnectionService() 
        svcconfig = svc.configuration()
        svcconfig.disablePoolAutomaticCleanUp()
        svcconfig.setConnectionTimeOut(0)

        for credential in credentials:
            log.info("Trying credentials {0}".format(credential))

            try: 
                session = svc.connect(credential, coral.access_ReadOnly)
            except Exception as e:
                log.warning("Failed to establish connection: {0}".format(e))
                continue

            try:
                # Check that the FRONTIER_SERVER is set properly, if not reduce the retrial period and time out values
                if not ('FRONTIER_SERVER' in os.environ and os.environ['FRONTIER_SERVER']):
                    svcconfig.setConnectionRetrialPeriod(1)
                    svcconfig.setConnectionRetrialTimeOut(1)
                else:
                    svcconfig.setConnectionRetrialPeriod(300)
                    svcconfig.setConnectionRetrialTimeOut(3600)

                session.transaction().start(True) # readOnly
                schema = ConfigDBLoader.getSchema(credential)
                qdict = { "schema" : schema, "dbkey" : self.dbkey }
                
                # Choose query basen on schema
                schemaVersion = ConfigDBLoader.readSchemaVersion(qdict, session)
                qstr = self.getQueryDefinition(schemaVersion)

                if "HLT_MONITORING_GROUPS" in qstr:
                    query = ConfigDBLoader.readMonGroupKey(qstr, qdict, session)
                else:
                    query = ConfigDBLoader.getCoralQuery(session, qstr.format(**qdict))

                cursor = query.execute()

                # Read query result
                cursor.next()
                configblob = cursor.currentRow()[0].data()
                if type(configblob) != str:
                    configblob = configblob.readline()
                config = json.loads(configblob, object_pairs_hook = odict)
                session.transaction().commit()
                
                self.confirmConfigType(config)
                return config       
            except Exception as e:
                log.warning("Failed to execute query: {0}".format(e))
                ConfigDBLoader.readMaxTableKey(qstr, qdict, session)
                session.transaction().commit()

        raise RuntimeError("Query failed")


    @staticmethod
    def readMaxTableKey(q, qdict, session):
        ''' Read highest available key in table, based on the query'''
        try:
            # In the case of multiple queries (e.g. MonGroups) take only the former
            q = q.split(";")[0]
            # Create query - remove last condition for the key value and add it in begining as "SELECT MAX"
            id_str = q.split("WHERE")[1].split("AND")[-1].split("=")[0]

            q_formatted = "SELECT MAX(" + id_str + ") FROM " + q.split("FROM")[1].split("WHERE")[0]

            query = ConfigDBLoader.getCoralQuery(session, q_formatted.format(**qdict))
            cursor = query.execute()
            cursor.next()

            log.info("Highest available key in %s is %i", id_str, int(cursor.currentRow()[0].data()))
        except Exception as e:
            log.warning("Failed to read maximum available key: {0}".format(e))

    @staticmethod
    def readMonGroupKey(q, qdict1, session):
        ''' Read highest available key in table, based on the query'''
        try:
            # First query is to find HLTMenuTableID
            qStr1 = q.split(";")[0]
            query1 = ConfigDBLoader.getCoralQuery(session, qStr1.format(**qdict1))
            cursor1 = query1.execute()
            cursor1.next()
            dbkeyResult = int(cursor1.currentRow()[0].data())

            # Second query is to use the found HLTMenuTableID and find the matching MonGroupKey
            # The query is performed as part of load(), and is only built here
            qdict2 = { "schema" : qdict1["schema"], "dbkeyResult" : dbkeyResult }
            qStr2 = q.split(";")[1]
            query2 = ConfigDBLoader.getCoralQuery(session, qStr2.format(**qdict2))
            return query2

        except Exception as e:
            log.warning("Failed to read HLT menu to find MonGroup key: {0}".format(e))


    # proposed filename when writing config to file
    def getWriteFilename(self):
        return "{basename}_{schema}_{dbkey}.json".format(basename = self.configType.basename, schema = self.schema, dbkey = self.dbkey)



class TriggerConfigAccess(object):
    """ 
    base class to hold the configuration (OrderedDict) 
    and provides basic functions to access and print
    """
    def __init__(self, configType, mainkey, filename = None, jsonString=None, dbalias = None, dbkey = None):
        self._getLoader(configType = configType, filename = filename, jsonString=jsonString, dbalias = dbalias, dbkey = dbkey)
        self._mainkey = mainkey
        self._config = None

    def _getLoader(self, configType, filename = None, jsonString=None, dbalias = None, dbkey = None ):
        if filename:
            self.loader = ConfigFileLoader( configType, filename )
        elif dbalias and dbkey:
            self.loader = ConfigDBLoader( configType, dbalias, dbkey )
        elif jsonString:
            self.loader = ConfigDirectLoader( configType, jsonString )
        else:
            raise RuntimeError("Neither input file, nor JSON nor db alias and key provided")

    def load(self):
        self._config = self.loader.load()

    def __str__(self):
        return str(self._config)

    def __iter__(self):
        return iter(self[self._mainkey])

    def __getitem__(self, item):
        return self._config[item]

    def __len__(self):
        return len(self[self._mainkey])

    def config(self):
        """ returns the configuration """
        return self._config

    def prettyPrint(self):
        if self._config:
            print(json.dumps(self._config, indent = 4, separators=(',', ': ')))
    
    def name(self):
        return self["name"]

    def filetype(self):
        return self["filetype"]

    def printSummary(self):
        """ print summary info, should be overwritten by derived classes """
        log.info("Configuration name: {0}".format(self.name()))
        log.info("Configuration size: {0}".format(len(self)))
    def writeFile(self, filename = None):
        if filename is None:
            filename = self.loader.getWriteFilename()
        with open(filename, 'w') as fh:
            json.dump(self.config(), fh, indent = 4, separators=(',', ': '))
            log.info("Wrote file {0}".format(filename))
