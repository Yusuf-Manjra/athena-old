/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/* Generated by Together */

/* date of creation : 12/06/2001 */

#ifndef LAROFFLINEID_EXCEPTION_H
#define LAROFFLINEID_EXCEPTION_H

#include <string>

/**
 * @brief Exception class for LAr Identifiers 

 * @author Johann Collot , Brinick Simmons
 */

class LArID_Exception {
public:

    /**
     * default constructor 
     */
    LArID_Exception() : m_message("No error message") , m_code( 999 ) {  }

    /**
     * constructor to be used 
     */
    LArID_Exception(const std::string&  l_message , int l_code) : m_message ( l_message ) , m_code ( l_code ) { }

    /**
     * set error message 
     */
    void message(const std::string& l_message) { m_message = l_message ;}

    /**
     * return error message <br> 
     */
    std::string message() const { return m_message;}

    /**
     * set error code number<br> 
     */
    void code(int l_code) { m_code = l_code ;}

    /**
     * return error code <br><br>
     * 
     * error codes : <br>
     * 0 : no error <br>
     * 5 : LArEM::region_id Error <br>
     * 6 : LArEM::channel_id Error <br>
     * 7 : LArHEC::region_id Error <br>
     * 8 : LArHEC::channel_id Error <br>
     * 9 : LArFCAL::region_id Error <br>
     *10 : LArFCAL::channel_id Error <br>
     * 999 : undefined error <br>
     */
    int code() const { return m_code;}

    operator std::string() const;


private:

    /**
     * error message 
     */
    std::string m_message;

    /**
     * error code 
     */
    int m_code;
};
#endif //LAROFFLINEID_EXCEPTION_H

