/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <numeric>
#include "TrigConfData/HLTMenu.h"
#include "TrigConfData/DataStructure.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Needed to silence Boost pragma message
#include <boost/property_tree/json_parser.hpp>
#include "TrigConfHLTData/HLTFrame.h"
#include "TrigConfHLTData/HLTChain.h"
#include "TrigConfHLTData/HLTStreamTag.h"
#include "TrigConfHLTData/HLTSignature.h"
#include "TrigConfHLTData/HLTTriggerElement.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigConfIO/JsonFileWriterHLT.h"

template<typename COLL>
boost::property_tree::ptree asArray( const COLL& data) {
   using ptree = boost::property_tree::ptree;
   ptree array;
   for ( const auto& el: data ) {
      ptree one;
      one.put("", el);
      array.push_back(std::make_pair("", one));
   }
   return array;
}

std::vector<int> legMult(const TrigConf::HLTChain* cptr) {
   return ChainNameParser::multiplicities(cptr->chain_name());
}

std::vector<std::string> l1thresholds(const TrigConf::HLTFrame* frame, const TrigConf::HLTChain* cptr) {
   std::set<std::string> names;
   for ( const auto sig: cptr->signatures() ) {
      for ( const auto te: sig->outputTEs() ) {
         auto sequence = frame->getHLTSequenceList().getSequence(te->name());
         for ( const auto inTE: sequence->inputTEs() ) {
            if ( not ( inTE->name().find("L2_") == 0 or inTE->name().find("EF_") == 0 or inTE->name().find("HLT_") == 0 ) ) {
               names.insert(inTE->name());
            }
         }
      }
   }
   return std::vector<std::string>( names.begin(), names.end() );
}

void printJSON(boost::property_tree::ptree& p) {
   // this code is useful to debug JSON before it is pushed to menu objects expecting certain structure in place
   std::stringstream ss;
   boost::property_tree::json_parser::write_json(ss, p);
   std::cout << ss.str() << std::endl;
}

bool convertHLTMenu(const TrigConf::HLTFrame* frame, TrigConf::HLTMenu& menu) {
   using ptree = boost::property_tree::ptree;
   ptree top;
   top.put("filetype", "hltmenu");
   top.put("name", frame->name());
   ptree pChains;

   std::map<std::string, const TrigConf::HLTStreamTag*> allStreams;

   for ( auto cptr : frame->getHLTChainList() ) {
      ptree pChain;
      pChain.put("counter", cptr->chain_counter());
      pChain.put("nameHash", cptr->chain_hash_id());
      pChain.put("l1item", cptr->lower_chain_name());
      pChain.add_child("l1thresholds", asArray(l1thresholds(frame, cptr)));
      pChain.add_child("legMultiplicities", asArray(legMult(cptr)) );
      pChain.add_child("sequencers", asArray(std::vector<std::string>({"missing"})));

      std::vector<std::string> strNames;
      for ( const auto st: cptr->streams()) {
         strNames.push_back(st->stream());
         allStreams[st->stream()] = st;
      }
      pChain.add_child("streams", asArray(strNames));

      pChain.add_child("groups", asArray(cptr->groups()));

      // Signature data
      // Note: This is run-2 only.
      // It is propagated here to allow legacy trigger feature access.
      std::vector<uint32_t> counters;
      std::vector<int> logics;
      std::vector<std::string> labels;
      ptree outputTEs_outerArray; // outputTEs is a std::vector<std::vector<std::string>>

      for(auto& signature : cptr->signatureList() ){
         uint32_t cntr = signature->signature_counter();
         counters.push_back(cntr);
         logics.push_back(signature->logic());
         labels.push_back(signature->label());
         ptree outputTEs_innerArray;
         for(auto& outputTE : signature->outputTEs()){
            outputTEs_innerArray.push_back( ptree::value_type("", outputTE->name()) );
         }
         outputTEs_outerArray.push_back( ptree::value_type("", outputTEs_innerArray) );
      }

      ptree pSig; 
      pSig.add_child("counters", asArray(counters));
      pSig.add_child("logics", asArray(logics));
      pSig.add_child("outputTEs", outputTEs_outerArray);
      pSig.add_child("labels", asArray(labels));

      pChain.add_child("signature", pSig);
      // End of signature data

      pChains.push_back(std::make_pair(cptr->chain_name(), pChain));
   }
   ptree pStreams;
   for ( auto [sname, stream]: allStreams ) {
      ptree pStream;
      pStream.put("name", sname);
      pStream.put("type", stream->type());
      pStream.put("obeyLB", stream->obeyLB());
      pStream.put("forceFullEventBuilding", true);  // TODO understand how to get this information from old menu
      pStreams.push_back(std::make_pair(sname, pStream));
   }

   top.add_child("chains", pChains);

   top.add_child("streams", pStreams);
   ptree pSequencers;
   pSequencers.add_child("missing", asArray(std::vector<std::string>({""})));
   top.add_child("sequencers", pSequencers);

   // Set run2 sequence information:
   const TrigConf::HLTSequenceList& sequenceList = frame->getHLTSequenceList();
   std::vector<std::string> outputTEs;
   ptree inputTEs_outerArray;  // sequenceInputTEs is a std::vector<std::vector<std::string>>
   ptree algorithms_outerArray;  // sequenceAlgorithms is a std::vector<std::vector<std::string>>
   for(auto& seq : sequenceList){
      outputTEs.push_back(seq->outputTE()->name());

      ptree inputTEs_innerArray;
      for(auto& input : seq->inputTEs()) {
         inputTEs_innerArray.push_back( ptree::value_type("", input->name()) );
      }
      inputTEs_outerArray.push_back( ptree::value_type("", inputTEs_innerArray) );

      ptree algorithms_innerArray;
      for(const std::string& alg : seq->algorithms()) {
         algorithms_innerArray.push_back( ptree::value_type("", alg) );
      }
      algorithms_outerArray.push_back( ptree::value_type("", algorithms_innerArray) );
   }
   ptree pSequence;
   pSequence.add_child("outputTEs", asArray(outputTEs));
   pSequence.add_child("inputTEs", inputTEs_outerArray);
   pSequence.add_child("algorithms", algorithms_outerArray);
   top.add_child("sequence_run2", pSequence);

   menu.setData(std::move(top));
   menu.setSMK(frame->smk());
   return true;
}

void convertRun2HLTMenuToRun3(const TrigConf::HLTFrame* frame, const std::string& filename) {
  TrigConf::HLTMenu menu;
  convertHLTMenu(frame, menu);

  TrigConf::JsonFileWriterHLT writer;
  writer.writeJsonFile(filename, menu);

}

void convertRun2HLTPrescalesToRun3(const TrigConf::HLTFrame* frame, const std::string& filename) {
   using ptree = boost::property_tree::ptree;
   ptree top;
   top.put("filetype", "hltprescale");
   top.put("name", frame->name());
   ptree pChains;
   for ( auto cptr : frame->getHLTChainList() ) {
      ptree pChain;
      pChain.put("name", cptr->chain_name());
      pChain.put("counter", cptr->chain_counter());
      pChain.put("hash", cptr->chain_hash_id());
      pChain.put("prescale", cptr->prescale());
      pChain.put("enabled", (cptr->prescale()>0 ? true: false));

      pChains.push_back(std::make_pair(cptr->chain_name(), pChain));
   }
   top.add_child("prescales", pChains);
   TrigConf::HLTPrescalesSet psk(std::move(top));

   TrigConf::HLTMenu menu;
   convertHLTMenu(frame, menu);

   TrigConf::JsonFileWriterHLT writer;
   writer.writeJsonFile(filename, menu, psk);
}

