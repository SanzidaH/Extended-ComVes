/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_FW_UC_HPP
#define NFD_DAEMON_FW_UC_HPP

#include "strategy.hpp"
#include "process-nack-traits.hpp"
#include <tuple>
#include <iostream>
#include <vector>
#include <string> 
#include "ns3/simulator.h"
#include <map>

namespace nfd {
namespace fw {

/** \brief A forwarding strategy that randomly chooses a nexthop
 *
 *  Sends the incoming Interest to a random outgoing face, excluding the incoming face.
 */
class uc : public Strategy
                     , public ProcessNackTraits<uc>
{
public:
  explicit
  uc(Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

public: // triggers
  void
  afterReceiveInterest(const Interest& interest, const FaceEndpoint& ingress,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveData(const Data& data, const FaceEndpoint& ingress,
                           const shared_ptr<pit::Entry>& pitEntry) override;
  //void
  //beforeSatisfyInterest(shared_ptr<pit::Entry> pitEntry, const Face& inFace, const Data& data);

  void
  beforeSatisfyInterest(const Data& data, const FaceEndpoint& ingress,
                                const shared_ptr<pit::Entry>& pitEntry);
  void
  afterReceiveNack(const lp::Nack& nack, const FaceEndpoint& ingress,
                   const shared_ptr<pit::Entry>& pitEntry) override;

  void
  SendCorrectionInterest();
  
  void
  broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry);
 
  //Split a string delimited by space
std::vector<std::string> SplitString(std::string strLine) {
  std::string str = strLine;
  std::vector<std::string> result;
  std::istringstream isstr(str);

  for(std::string str; isstr >> str; )
    result.push_back(str);

  return result;
}

private:
  friend ProcessNackTraits<uc>;
  //std::vector<std::tuple<Name, std::vector<std::tuple<FaceId, double>>>> m_loadTable;
  //std::vector<boost::tuple<Name, std::vector<std::tuple<FaceId, double>>>> m_loadTable;
  // std::vector<boost::tuple<Name, FaceId, double>> m_loadTable;
   //boost::tuple<Name, FaceId, double> t{"", 0 , 0};
   //std::map<std::string, std::vector<int>> m_loadTable;
   std::multimap<std::string, std::array<int, 5>> m_loadTable; //face, seq, server count, pending int
   std::multimap<std::string, std::array<int, 5>>::iterator itr;
   std::map<std::string, int> m_serverMap;
 
  // std::map<std::string, std::map<int, std::array<int, 3>>> m_cost; // prefix, face, server, cost, isServer
   std::map<std::string, std::vector<std::map<int, std::vector<int>>>> m_cost;
 //  std::map<std::string, std::vector<std::map<int, std::array<int, 3>>>> m_cost;

   // map<string, map<int, vector<int>>> m_load;
   //std::vector< std::pair <int, std::unordered_map<std::string, std::array<int, 3>>> > m_loadTable;
   
   int m_count, m_seq, m_max;
   bool m_hasServer, m_firsttime;
  //std::vector<boost::tuple<std::string, long unsigned int, int>>  m_loadTable;
  //m_loadTable tl; 
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_RANDOM_STRATEGY_HPP
