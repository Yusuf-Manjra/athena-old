/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/Identifier.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>

#include <iostream>
#include <iomanip>




//-----------------------------------------------
void Identifier::set (const std::string& id)
{
  sscanf (id.c_str(), "0x%" IDENTIFIER_PCODE "x", &m_id);
}


//-----------------------------------------------
std::string Identifier::getString() const
{
  std::string result;
  char temp[20];

  sprintf (temp, "0x%" IDENTIFIER_PCODE "x", (Identifier::value_type)m_id);
  result += temp;
  return (result);
}

//-----------------------------------------------
void Identifier::show () const
{
    static_assert(std::is_trivially_destructible<Identifier>::value);
    static_assert(std::is_trivially_copy_constructible<Identifier>::value);
    const Identifier& me = *this;
    std::cout << me.getString();
}


