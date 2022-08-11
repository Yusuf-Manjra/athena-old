#!/usr/bin/env python
#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

'''
@file TableConstructorBase.py
@brief Defines TableConstructor base class to read ROOT histograms and convert it to csv table
'''

from math import fabs
from collections import OrderedDict
from TrigCostAnalysis.CostMetadataUtil import ignoreUnderflow
from AthenaCommon.Logging import logging
log = logging.getLogger('TableConstructor')


class Column:
    ''' @brief Class representing a single csv column

    Used in TableConstructor to store header description and values of csv column
    '''

    def __init__(self, header = "Unknown", headerDesc = "Unknown", needNormalizing = False):
        '''Column header'''
        self.header = header

        '''Column description'''
        self.headerDesc = headerDesc

        '''The list storing column content'''
        self.content = []

        '''Flag if column needs normalization'''
        self.needNormalizing = needNormalizing


    def addValue(self, value, index=-1):
        ''' @brief Add value to the column '''
        if index < 0 or index >= len(self.content):
            self.content.append(value)
        else:
            self.content[index] += value

    def updateValue(self, value, index=-1):
        ''' @brief Add value to the column '''
        if index < 0 or index >= len(self.content):
            self.content.append(value)
        else:
            self.content[index] = value

    def getValue(self, index):
        ''' @brief Get value from column with given index'''
        if index >= len(self.content):
            log.error("Index out of range")
            return None
        else:
            return self.content[index]


class TableConstructorBase:
    ''' @brief Class representing a single table

    Base class trepresenting table. It defines basic behavior common for 
        all the tables like caching histograms or saving table to file. 
    '''

    def __init__(self, tableObj, underflowThreshold, overflowThreshold):
        '''ROOT table directory object, storing subdirs with histograms'''
        self.tableObj = tableObj

        '''Dictionary storing the columns. Each column is identified by given name (key)'''
        self.columns = OrderedDict()

        '''Map to cache expected histograms'''
        self.histograms = {}

        '''List with expected histograms'''
        self.expectedHistograms = []

        '''Number of underflow bins after which warning will be saved to metadata tree'''
        self.underflowThreshold = underflowThreshold

        '''Number of overflow bins after which warning will be saved to metadata tree'''
        self.overflowThreshold = overflowThreshold

        '''Overflow log'''
        self.warningMsg = []


    def getWarningMsgs(self):
        ''' @brief Raturn warning messages concerning histogram under/over flows '''
        return self.warningMsg


    def getHistogram(self, name):
        ''' @brief Return cached histogram with given name'''
        if name not in self.histograms:
            log.error("Histogram %s not found!", name)
            return None
        return self.histograms[name]


    def cacheHistograms(self, dirName, prefix):
        ''' @brief Cache histograms in map for given directory
        
        Save histograms in the map by their short name as a key. If the histogram 
        with a given name is not found function logs an error.

        @param[in] dirName Name of subdirectory to look for histograms
        @param[in] prefix Prefix of the histogram name
        
        '''
        dirKey = [key for key in self.tableObj.GetListOfKeys() if key.GetName() == dirName]
        if not dirKey:
            log.error("Subdirectory %s not found", dirName)

        dirObj = dirKey[0].ReadObj()
        for histName in self.expectedHistograms:
            fullHistName = prefix + dirName + '_' + histName
            hist = dirObj.Get(fullHistName)

            if not hist:
                log.debug("Full name %s", fullHistName)
                log.debug("Directory: %s", dirObj.ls())
                log.error("Expected histogram %s not found", histName)
            else:
                # Check for under and overflows
                # Under/overflow bin stores weighted value - we cannot base on entries
                histIntegral = hist.Integral(0, hist.GetNbinsX() + 1) # Integral including overflows
                overflowThreshold = self.overflowThreshold * histIntegral
                overflow = hist.GetBinContent(hist.GetNbinsX() + 1)
                if overflow > overflowThreshold:
                    log.warning("Histogram {0} contains overflow of {1}".format(fullHistName, overflow))
                    self.warningMsg.append("Overflow of {0} ({1} histogram integral) in {2}".format(overflow, histIntegral, fullHistName))

                underflowThreshold = self.underflowThreshold * histIntegral
                underflow = hist.GetBinContent(0)
                if underflow > underflowThreshold and not ignoreUnderflow(fullHistName):
                    log.warning("Histogram {0} contains underflow of {1}".format(fullHistName, underflow))
                    self.warningMsg.append("Underflow of {0} ({1} histogram integral) in {2}".format(underflow, histIntegral, fullHistName))

                self.histograms[histName] = hist


    def getXWeightedIntegral(self, histName, isLog = True):
        ''' @brief Get "total" value by integrating over a histogram, weighting every entry by its x-axis mean.

        @param[in] histName Histogram name
        @param[in] isLog If histogram is log x-axis, modifies how x-axis mean is computed for each bin.

        @return Total value of the histogram.
        '''

        hist = self.getHistogram(histName)
        if not hist:
            return 0

        total = 0
        for i in range(1, hist.GetXaxis().GetNbins()+1):
            if isLog:
                total += hist.GetBinContent(i) * hist.GetXaxis().GetBinCenterLog(i)
            else:
                total += hist.GetBinContent(i) * hist.GetXaxis().GetBinCenter(i)

        return total


    def saveToFile(self, fileName):
        ''' @brief Function to save table content to csv file with specified fileName

        @param[in] fileName  Name of the file to save the table
        '''

        from TrigCostAnalysis.Util import convert
        import csv

        with open(fileName, mode='w') as csvFile:
            csvWriter = csv.writer(csvFile, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
            
            header = []
            desc = []
            rows = []
            for column in self.columns.values():
                header.append(column.header)
                desc.append(column.headerDesc)
                content = [ convert(entry) for entry in column.content ]
                rows.append(content)

            csvWriter.writerow(header)
            csvWriter.writerow(desc)

            # Transpose columns to rows
            rows = list(zip(*rows))
            for row in rows:
                csvWriter.writerow(row)


    def normalizeColumns(self, denominator):
        ''' @brief Perform normalization on marked columns 
        
        @param[in] denominator Value to normalize choosen columns
        '''
        for columnName, column in self.columns.items():
            if column.needNormalizing:
                if fabs(denominator) < 1e-10:
                    log.error("Normalise denominator is zero. Setting %s to zero.", columnName)
                    column.content = [0] * len(column.content)
                else:
                    column.content = [entry/denominator for entry in column.content]


    def fillTable(self, prefix=''):
        ''' @brief Fill the table based on ROOT directory's content.'''

        self.defineColumns()

        # Fill the columns
        for hist in self.tableObj.GetListOfKeys():
            self.cacheHistograms(hist.GetName(), prefix)
            self.fillColumns(hist.GetName())

        self.postProcessing()


    def defineColumns(self):
        ''' @brief Define the columns for the table

        Columns should be objects of class Column, added to the map self.columns.
        '''
        log.error("Should not call defineColumns on base class!")


    def fillColumns(self, histName=''):
        ''' @brief Fill the columns with values '''
        log.error("Should not call fillColumns on base class!")


    def postProcessing(self):
        ''' @brief Additional operations

        Normalization in performed separatly!
        '''
        pass