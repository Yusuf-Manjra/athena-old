/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IOVTIME_H
#define ATHENAKERNEL_IOVTIME_H

/**
 *
 *  @file IOVTime.h
 *  @brief Basic time unit for IOVSvc. 
 *  Hold time as a combination of run and event numbers
 *
 *  @author Charles Leggett
 *  $Id: IOVTime.h,v 1.8 2007-06-14 01:57:23 calaf Exp $
 *
 *
 *****************************************************************************/

#include <iosfwd>
#include <string>
#include <stdint.h>
#include <limits>

class MsgStream;
class EventIDBase;

/**
 *  @class IOVTime
 *  @brief Basic time unit for IOVSvc. 
 *  Hold time as a combination of run and event numbers
 */
class IOVTime {

private:
  enum IOVTime_type {
    UNDEF = 0,
    TIMESTAMP,
    RUN_EVT,
    BOTH
  };

public:
  static constexpr uint32_t MINRUN = std::numeric_limits<uint32_t>::min();

  // We remove the top bit from MAXRUN to allow use of this to set
  // CondDBKey which has a sign bit
  static constexpr uint32_t MAXRUN = (std::numeric_limits<uint32_t>::max() >> 1);

  static constexpr uint32_t MINEVENT = std::numeric_limits<uint32_t>::min();
  static constexpr uint32_t MAXEVENT = (std::numeric_limits<uint32_t>::max());
  
  static constexpr uint64_t MAXRETIME =( ((uint64_t) IOVTime::MAXRUN << 32) + IOVTime::MAXEVENT );
  static constexpr uint64_t UNDEFRETIME = std::numeric_limits<uint64_t>::max();

  static constexpr uint64_t MINTIMESTAMP = std::numeric_limits<uint64_t>::min();
  // Set MAXTIMESTAMP to 63 bit max
  static constexpr uint64_t MAXTIMESTAMP = (std::numeric_limits<uint64_t>::max() >> 1);
  static constexpr uint64_t UNDEFTIMESTAMP = std::numeric_limits<uint64_t>::max();

public:
  /**
   *  @class SortByTimeStamp
   *  @brief Predicate. Used to sort by time stamp
   */
  class SortByTimeStamp {
  public:
    bool operator() ( const IOVTime& t1, const IOVTime& t2 ) const noexcept {
      return t1.timestamp() > t2.timestamp();
    }
    bool operator() ( const IOVTime* t1, const IOVTime* t2 ) const noexcept {
      return t1->timestamp() > t2->timestamp();
    }
  };

  /**
   *  @class  SortByRunEvent
   *  @brief Predicate. Used to sort by run and event number
   */
  class SortByRunEvent {
  public:
    bool operator() ( const IOVTime& t1, const IOVTime& t2 ) const noexcept {
      return t1.re_time() > t2.re_time();
    }
    bool operator() ( const IOVTime* t1, const IOVTime* t2 ) const noexcept {
      return t1->re_time() > t2->re_time();
    }
  };

public:
  IOVTime(): m_status(IOVTime::UNDEF), m_time(UNDEFRETIME), 
    m_timestamp(UNDEFTIMESTAMP){};
  explicit IOVTime( uint64_t timestamp ): m_status(IOVTime::TIMESTAMP),
    m_time(IOVTime::UNDEFRETIME), m_timestamp(timestamp){};
  explicit IOVTime(  uint32_t run,  uint32_t event );
  explicit IOVTime(  uint32_t run,  uint32_t event, 
	    uint64_t timestamp );
  IOVTime( const EventIDBase& eid);

  void setTimestamp(  uint64_t timestamp ) noexcept;
  void setRETime(  uint64_t time ) noexcept;
  void setRunEvent( uint32_t run,  uint32_t event ) noexcept;
  void reset() noexcept;

  inline uint32_t run() const noexcept { return static_cast<uint32_t> (m_time>>32); }
  inline uint32_t event() const noexcept { return static_cast<uint32_t> (m_time & 0xFFFFFFFF); }
  inline uint64_t re_time() const noexcept { return m_time; }
  inline uint64_t timestamp() const noexcept { return m_timestamp; }

  bool isValid() const noexcept;
  inline bool isTimestamp() const noexcept { return (m_status == IOVTime::TIMESTAMP ||
				     m_status== IOVTime::BOTH) ? 1 : 0; }
  inline bool isRunEvent() const noexcept { return (m_status == IOVTime::RUN_EVT ||
				    m_status == IOVTime::BOTH) ? 1 : 0; }
  inline bool isBoth() const noexcept { return (m_status == IOVTime::BOTH) ? 1 : 0; }

  operator std::string() const;
  operator EventIDBase() const;

  friend bool operator<(const IOVTime& lhs, const IOVTime& rhs) noexcept;
  friend bool operator>(const IOVTime& lhs, const IOVTime& rhs) noexcept;
  friend bool operator==(const IOVTime& lhs, const IOVTime& rhs) noexcept;
  friend bool operator!=(const IOVTime& lhs, const IOVTime& rhs) noexcept;
  friend bool operator>=(const IOVTime& lhs, const IOVTime& rhs) noexcept;
  friend bool operator<=(const IOVTime& lhs, const IOVTime& rhs) noexcept;

  friend std::ostream& operator<<(std::ostream& os, const IOVTime& rhs);
  friend MsgStream& operator<<(MsgStream& os, const IOVTime& rhs);

private:

  IOVTime_type m_status;
  uint64_t m_time;
  uint64_t m_timestamp;
};

inline bool operator<(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  if (lhs.isTimestamp() && rhs.isTimestamp()) {
    return lhs.m_timestamp < rhs.m_timestamp;
  } else {
    return lhs.m_time < rhs.m_time;
  }
}
inline bool operator>(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  if (lhs.isTimestamp() && rhs.isTimestamp()) {
    return lhs.m_timestamp > rhs.m_timestamp;
  } else {
    return lhs.m_time > rhs.m_time;
  }
}
inline bool operator==(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  if (lhs.isTimestamp() && rhs.isTimestamp()) {
    return lhs.m_timestamp == rhs.m_timestamp;
  } else {
    return lhs.m_time == rhs.m_time;
  }
}
inline bool operator!=(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  return !(lhs == rhs) ;
}
inline bool operator>=(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  return !( lhs < rhs );
}
inline bool operator<=(const IOVTime& lhs, const IOVTime& rhs) noexcept {
  return !( lhs > rhs );
}

// template < class STR >
// inline STR& operator << (STR& os, const IOVTime& rhs) {
//   os << rhs.m_time << ":  [" << (rhs.m_time>>32) << "," 
//      << ( rhs.m_time & 0xFFFFFFFF ) << "]";
//   return os;
// }

#endif
  
