/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NDN_PRODUCER_H
#define NDN_PRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"
#include "ndn-app.hpp"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"

#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"

#include <set>
#include <map>
#include <queue>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>


namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class Producer : public App {
public:
  static TypeId
  GetTypeId(void);

  Producer();

  // inherited from NdnApp
  virtual void
  OnInterest(shared_ptr<const Interest> interest);
  
  void
  OnReceiveInitialRequest(shared_ptr<const Interest> interest, uint32_t task_util);

  typedef void (*SentInterestTraceCallback)( uint32_t, shared_ptr<const Interest> );
  typedef void (*ReceivedDataTraceCallback)( uint32_t, shared_ptr<const Data>,uint32_t);
  typedef void (*SeverUpdateTraceCallback)( uint32_t, std::string, int,std::string );  

  void
  SendNack(shared_ptr<const Interest> interest);

  void
  PrintExpired(std::string name);

  void
  PrintResource( uint32_t task_util);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication(); // Called at time specified by Start

  virtual void
  StopApplication(); // Called at time specified by Stop

  uint32_t m_capacity; // model server task capacity (in terms of budget of milliseconds
  uint32_t m_seed;
  uint32_t m_utilization; // model server task capacity (in terms of budget of milliseconds
  uint32_t m_util; 
  Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
  //Ptr<NormalRandomVariable> m_comTime;
  
//private:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_hint;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;
  int serviceCount;
 //   uint32_t m_hint;
 // uint32_t m_capacity;

TracedCallback < uint32_t, std::string, int,  std::string > m_serverUpdate;
   std::vector<std::string>
      SplitString( std::string strLine, char delimiter ) {

        std::string str = strLine;
        std::vector<std::string> result;
        uint32_t i = 0;
        std::string buildStr = "";

        for ( i = 0; i<str.size(); i++) {

           if ( str[i]== delimiter ) {
              result.push_back( buildStr );
              buildStr = "";
           }
           else {
              buildStr += str[i];
           }
        }

        if(buildStr!="")
           result.push_back( buildStr );

        return result;
}



};

} // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_H
