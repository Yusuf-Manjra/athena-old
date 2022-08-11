/*
    Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "EgammaAnalysisHelpers/AsgEGammaConfigHelper.h"
#include "AsgMessaging/AsgMessaging.h"
#include "TEnv.h"
#include <iostream>
#include <sstream>

namespace AsgConfigHelper {

std::string
findConfigFile(const std::string& input,
               const std::map<std::string, std::string>& configmap)
{
  auto confFile_itr = configmap.find(input);
  if (confFile_itr == configmap.end()) {
    static const asg::AsgMessaging msg("Egamma::AsgConfigHelper");
    msg.msg(MSG::WARNING) << "Key " << input
                          << " not found in map, no config file returned"
                          << endmsg;
    return "";
  }
  return confFile_itr->second;
}

unsigned int
findMask(const std::string& input,
         const std::map<std::string, unsigned int>& maskmap)
{
  auto mask_itr = maskmap.find(input);
  if (mask_itr == maskmap.end()) {
    static const asg::AsgMessaging msg("Egamma::AsgConfigHelper");
    msg.msg(MSG::WARNING)
      << "Key " << input
      << " not found in map,  egammaPID::EgPidUndefined mask returned"
      << endmsg;
    // mask has the choice to default to all 1 or all 0 bits, choose the former
    return std::numeric_limits<unsigned int>::max();
  }
  return static_cast<unsigned int>(mask_itr->second);
}

template<typename T>
bool
strtof(const std::string& input, T& f)
{
  int diff = 0;
  std::string tmp = input;
  std::string::size_type first(0);
  std::string::size_type last(0);
  first = (input.find('#'));

  // if we do not find a comment character "#" we are fine
  if (first == std::string::npos) {
    std::istringstream buffer(tmp);
    buffer >> f;
    return true;
  } else {
    // if we have found comment character check if it is inlined between two "#"
    last = (input.find('#', first + 1));
    // if nor error
    if (last == std::string::npos) {
      static const asg::AsgMessaging msg("Egamma::AsgConfigHelper");
      msg.msg(MSG::WARNING) << " Improper comment format , inline comment "
                               "should be enclosed between two #  "
                            << endmsg;
      return false;
    }
    // else if between two "#" remove this part
    diff = last - first;
    tmp = tmp.erase(first, diff + 1);
    std::istringstream buffer(tmp);
    buffer >> f;
    return true;
  }
}

template<typename T>
std::vector<T>
Helper(const std::string& input, TEnv& env)
{
  std::vector<T> CutVector;
  std::string env_input(env.GetValue(input.c_str(), ""));
  if (!env_input.empty()) {
    std::string::size_type end;
    do {
      end = env_input.find(';');
      T myValue{}; //default init
      if (AsgConfigHelper::strtof(env_input.substr(0, end), myValue)) {
        CutVector.push_back(myValue);
      }
      if (end != std::string::npos) {
        env_input = env_input.substr(end + 1);
      }
    } while (end != std::string::npos);
  }
  return CutVector;
}
}

// use the specializations
std::vector<double>
AsgConfigHelper::HelperDouble(const std::string& input, TEnv& env)
{
  return AsgConfigHelper::Helper<double>(input, env);
}
std::vector<float>
AsgConfigHelper::HelperFloat(const std::string& input, TEnv& env)
{
  return AsgConfigHelper::Helper<float>(input, env);
}
std::vector<int>
AsgConfigHelper::HelperInt(const std::string& input, TEnv& env)
{
  return AsgConfigHelper::Helper<int>(input, env);
}
// template does not work for std::string because of the T myValue(0);
// declaration, so implement it again for std::string
std::vector<std::string>
AsgConfigHelper::HelperString(const std::string& input, TEnv& env)
{
  return AsgConfigHelper::Helper<std::string>(input, env);
}

