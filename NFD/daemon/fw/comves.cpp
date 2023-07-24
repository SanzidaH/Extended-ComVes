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

#include "comves.hpp"
#include "algorithm.hpp"
#include "ns3/simulator.h"
#include "model/ndn-l3-protocol.hpp"
#include <ndn-cxx/util/random.hpp>



#include "algorithm.hpp"
#include "best-route-strategy.hpp"
#include "scope-prefix.hpp"
#include "strategy.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "table/cleanup.hpp"

#include <ndn-cxx/lp/pit-token.hpp>
#include <ndn-cxx/lp/tags.hpp>

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(Comves);
NFD_LOG_INIT(Comves);

Comves::Comves(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
{
  NFD_LOG_DEBUG("Comves.......");
  //sleep(2);
  //m_count = 0; // don't need it
  m_seq = 0; // we need it only for priod correction
  m_loadTable = {}; // important
  m_serverMap = {}; // don't need it
  m_hasServer = false; // we need it only for priod correction
  m_firsttime = true;
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("Comves does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "Comves does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
  
  const Fib& fib = this->getFib(); // getting all fib entries to fill up mtable in constructor (once while starting)
  for (Fib::const_iterator i = fib.begin(); i != fib.end(); ++i){ // interate fib
         const fib::Entry& entry = *i;
         const Name& prefix = entry.getPrefix();
         NFD_LOG_DEBUG("prefix " << prefix);
         std::string interestName = "";
         if (!prefix.empty()){
           interestName = prefix.get(0).toUri();
         //interestName = prefix.getSubName(0,2).toUri();
         }
         
        //if(interestName != "" && interestName != "/update" && interestName != "/localhost/nfd" && interestName != "/localhost"){
        if(interestName != "" && interestName != "update" && interestName != "/localhost/nfd" && interestName != "localhost"){ // excluding all name except service name
         const fib::NextHopList& nextHops = entry.getNextHops(); // all the next hop/ faces for current name of fib 
         int serverCount = 0; // need it for only periodic correction 
         int face; // second[0] need it 
         uint64_t cost; // need it for storing updated number of pending interest
         for (fib::NextHopList::const_iterator j = nextHops.begin();
               j != nextHops.end();
               ++j){ // iterate all next hops for current name
              const fib::NextHop& next = *j;
              face = next.getFace().getId();
               cost = next.getCost(); // we are not using it, we would need it for perodic correction to find leaf node
              NFD_LOG_DEBUG("name " << interestName << ", face " << face << ", cost " << cost); 
              if(cost<3){
	      if(cost == 0){  //this part for periodic correction
              //m_hasServer = true;
                 m_loadTable.insert({ interestName, {face, m_seq, 1, 0, 1} });
              }else{
                 m_loadTable.insert({ interestName, {face, m_seq, 0, 0, 0} });
                 
              }    
	      }	      
              
         }
              // m_table structure: name, face, seq, server count, pending_int, hasServer - determines if router is a correction interest sender
             
      }
      
      //if(m_hasServer){  // for periodic correction
        //ns3::Simulator::Schedule(ns3::Seconds(0.1), &Comves::SendCorrectionInterest, this);
      //}  
  }
  
  
          for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]
              
             NFD_LOG_DEBUG(" interestName " << itr -> first << " face " <<  itr -> second[0] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2] << " pending_int " << itr -> second[3] << " hasServer " << itr -> second[4]);
              
          }
  
}

const Name&
Comves::getStrategyName()
{
  static const auto strategyName = Name("/localhost/nfd/strategy/comves").appendVersion(1);
  return strategyName;
}

void
Comves::removeExpiredInterest(const Interest& interest, int outFace,
                                     const shared_ptr<pit::Entry>& pitEntry)
{
/*	for (const pit::InRecord& inRecord : pitEntry->getOutRecords()) {
   	 if (inRecord.getExpiry() > now) {
    	  if (inRecord.getFace().getId() == inFace.getId() &&
        	  inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
       	 continue;
	  }
	 }
	}*/
//	const Pit& pit = this->getPit();
//	 for (Pit::const_iterator i = pit.begin(); i != pit.end(); ++i){ // interate pit
  //      	 const pit::Entry& entry = *i;
  //  NFD_LOG_DEBUG("Can Match? " << pitEntry->canMatch(interest, 2));
         	 for (const pit::OutRecord& outRecord : pitEntry->getOutRecords()) {
                           NFD_LOG_DEBUG("PIT outRecord " << outRecord.getFace().getId() ); 
       			 }
//	 }

   if(pitEntry->isSatisfied){
	   NFD_LOG_DEBUG("already Satisfied, do not decrement " << interest);
   }else{
	 std::string interestName = interest.getName().get(0).toUri();
  	 //Face& inFace = ingress.face;

	 NFD_LOG_DEBUG("Not Satisfied, decrement " << interest);
	     bool found = false;
            for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]

             NFD_LOG_DEBUG(interest.getName() << " Face: " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4] << " << mtable interest expired before");

          }

  for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){

        if (itr -> first == interest.getName().get(0).toUri() && itr -> second[0] == outFace && itr -> second[4]== 0){
            found = true;
                itr -> second[3] = itr -> second[3] - 1;// updating table after expiration
        }
         if(found) break;
  }

              for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]

             NFD_LOG_DEBUG("mtable interest expired after: " << interest.getName() << " face " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4]);

          }
    }
}



void
Comves::afterReceiveInterest(const Interest& interest, const FaceEndpoint& ingress,
                                     const shared_ptr<pit::Entry>& pitEntry)
{

   //Check m_table first
   std::string interestName = interest.getName().get(0).toUri();
   Face& inFace = ingress.face;
   //NFD_LOG_INFO("interest received " << interest);
  
   const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
   const Fib& fib = this->getFib();
      
  bool incCost = false, found = false;
  int mtableCost = INT_MAX;
  int mtableFace;
  long unsigned int outface; 
  int newOutface;
  bool inTable = false;
  int serverCount = 0;
  int othernodeCount = 0;
  bool isServer = false;
    
  //insert all faces that is not in mtable for received name and initialize with zero (can we run it once?)
  

/*
   for (const auto& nexthop : fibEntry.getNextHops()) {  //iterate all next hop in fib for received name
        Face& outFaceTemp = nexthop.getFace();
        uint64_t cost = nexthop.getCost();
        inTable = false;
        for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // iterate m_table
              if (itr -> first == interestName && itr -> second[0] == boost::numeric_cast<int>(outFaceTemp.getId())){ 
              // if name and face match, we have it in mtable
                inTable = true;
                break;
              }
        }       
        if(!inTable){ //if not in mtable - insert in mtable
            NFD_LOG_INFO(" NEW entry initilaize * " << interestName << " "<< outFaceTemp.getId()); 
             if(cost == 0){
              m_loadTable.insert({ interestName, {boost::numeric_cast<int>(outFaceTemp.getId()), 0, 1, 0, 1} });
            }else{
              m_loadTable.insert({ interestName, {boost::numeric_cast<int>(outFaceTemp.getId()), 0, 0, 0, 0} });
            }
            
        }  
   } */
    
    
  // Check if m_table has entry for the incoming interest and find interface with lowest cost
  for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){   
        // NFD_LOG_DEBUG("*** before "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
        if (itr -> first == interestName){
            NFD_LOG_DEBUG(" Found incoming interest: face <<" << itr->second[0] << ", cost << " << itr->second[3] << " interest " << interest.getName());
           // if(itr->second[4] == 1) {isServer = true; break; NFD_LOG_DEBUG("Server found, don't follow m_table ");}
            found = true;
           // int server = itr->second[2];
           // if(server < 1) server = 1;
            if(mtableCost > (itr->second[3]) && itr->second[0] != boost::numeric_cast<int>(inFace.getId())){ //finding face with lowest cost
              mtableCost = itr->second[3];
              mtableFace = itr->second[0];
            }
        }       
        // NFD_LOG_DEBUG("*** after "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
   }
   
    NFD_LOG_DEBUG(" Selected in mtable: Face: <<" << mtableFace << ", cost << " << mtableCost << " interest " << interest.getName());
 
 
// if(!isServer) { 
 // if entry found sent to the face with lowest cost

  if(found){ 
    for (const auto& nexthop : fibEntry.getNextHops()) {
             Face& outFaceTemp = nexthop.getFace();
             if(boost::numeric_cast<int>(outFaceTemp.getId()) == mtableFace){
               if (!wouldViolateScope(inFace, interest, outFaceTemp) && outFaceTemp.getId() != inFace.getId()) { 
                 NFD_LOG_DEBUG(interest << " sent from=" << ingress << " newPitEntry-to=" << outFaceTemp.getId());
                 this->sendInterest(interest, outFaceTemp, pitEntry);
                
                 //outface = outFaceTemp.getId(); 
                 newOutface = boost::numeric_cast<int>(outFaceTemp.getId());
                 break;                 
                 }
                
             }
        }
 } 
  // if entry is not found, check fib and send to the best face in terms of hops 
  if(!found){
    for (const auto& nexthop : fibEntry.getNextHops()) {
        Face& outFace = nexthop.getFace();        
        if (!wouldViolateScope(inFace, interest, outFace)) {
           NFD_LOG_DEBUG(interest << " FIB from=" << ingress << " newPitEntry-to=" << outFace.getId());
     // NFD_LOG_DEBUG("PIT entry: "<< fibEntry.getPrefix());
        this->sendInterest(interest, outFace, pitEntry);
        newOutface = boost::numeric_cast<int>(outFace.getId()); 
        break;  
    }
   }
  }
  
 
//------------------------

            for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]
              
             NFD_LOG_DEBUG(interest.getName() << " Face: " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4] << " <<mtable check before increment");
              
          }


  // increment cost in mtable for corresponding service and outface since interest has been received and sent
  for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){   
        if (itr -> first == interestName && itr -> second[0] == newOutface && itr -> second[4]== 0){
            //NFD_LOG_DEBUG("*** before "<< itr->first   << " face:" << itr->second[0] << ", cost: " << itr->second[3]);
         //   NFD_LOG_DEBUG(" ** Found interest and face in mtable to update/increment " << interest.getName());
            incCost = true;
            itr -> second[3] = itr -> second[3] + 1;
            //NFD_LOG_DEBUG("*** after "<< itr->first   << " face:" << itr->second[0] << ", cost: " << itr->second[3]);
        }       
         
         if(incCost) break;
  }
  // if there is not exisiting entry for this service and outface, insert a new entry in mtable 
/*  if(!incCost){
    NFD_LOG_INFO(" not in mtable - NEW * " << interestName << " "<< newOutface);
    m_loadTable.insert({ interestName, {newOutface, 0, 0, 1, 0} });
  } 
 */ 
            for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]
              
             NFD_LOG_DEBUG(interest.getName() << " Face: " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4] << " << mtable check after increment");
              
          }
          
          
 // }
  
/*      for (const auto& nexthop : fibEntry.getNextHops()) {
        Face& outFace = nexthop.getFace();        
        if (!wouldViolateScope(inFace, interest, outFace)) {
           NFD_LOG_DEBUG(interest << " ** Server FIB from=" << ingress << " newPitEntry-to=" << outFace.getId());
     // NFD_LOG_DEBUG("PIT entry: "<< fibEntry.getPrefix());
        this->sendInterest(interest, outFace, pitEntry);
        newOutface = boost::numeric_cast<int>(outFace.getId()); 
        break;  
    }
   }
 */ 
//   Simulator::Schedule(Seconds(2.0), &Comves::removeExpiredInterest,interest, ingress, pitEntry ,this);
  getScheduler().schedule(2_s, [=] { removeExpiredInterest(interest, newOutface, pitEntry); }); //scheduled to be expired
	   // ns3::Simulator::Schedule(2000000000, [=] { removeExpiredInterest(interest, ingress, pitEntry); });
           
}


void
Comves::broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  Face& inFace = ingress.face;
  for (auto& outFace : this->getFaceTable()) {
    if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
        wouldViolateScope(inFace, interest, outFace) || outFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL) {
      continue;
    }
    this->sendInterest(interest,inFace, pitEntry);
    //pitEntry->getOutRecord(outFace)->insertStrategyInfo<OutRecordInfo>().first->isNonDiscoveryInterest = false;
    //NFD_LOG_DEBUG("send Util Interest (broadcasted)=" << interest << " from="
               //   << inFace.getId() << " to=" << outFace.getId());
  }
}

void
Comves::afterReceiveData(const Data& data, const FaceEndpoint& ingress,
                           const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("afterReceiveData pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());
  this->beforeSatisfyInterest(data, ingress, pitEntry);
  this->sendDataToAll(data, pitEntry, ingress.face);
}


void 
Comves::beforeSatisfyInterest(const Data& data, const FaceEndpoint& ingress,
                                const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("beforeSatisfyInterest pitEntry=" << pitEntry->getName()
                << " in=" << ingress.face.getId() << " data=" << data.getName());
                
  Block params = data.getMetaInfo().getAppMetaInfo().front();
  std::string device_hint((char*)params.value(), params.value_size());
  int newOutface = boost::numeric_cast<int>(ingress.face.getId());
  NFD_LOG_INFO("number of process: " << device_hint << "  ,face, " << newOutface);              
        
  
  bool found = false;              
  std::string dataName = data.getName().get(0).toUri();	  
  // correction interest format: /update/seq/prefix/serverCount/load
  //m_table format: face, seq, server count, pending int, hasServer 

            for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]
              
             NFD_LOG_DEBUG(data.getName() << " Face: " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4] << "<< mtable check before decrement");
              
          }
  
  for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){   
       
        if (itr -> first == dataName && itr -> second[0] == newOutface && itr -> second[4]== 0){
            //NFD_LOG_DEBUG("*** before "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
            found = true;
            //if(itr -> second[3] > 0)
           // itr -> second[3] = std::stoi(device_hint);// updating table with Data carried correction
           // NFD_LOG_DEBUG("*** after "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
 		itr -> second[3] = itr -> second[3] - 1;// decrementing cost

        }       
         if(found) break;
  }  
  
          for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]
              
             NFD_LOG_DEBUG(data.getName() << " Face: " <<  itr -> second[0] << " cost >> " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4] << "<< mtable check after decrement");
              
          }
             
}


void
Comves::afterReceiveNack(const lp::Nack& nack, const FaceEndpoint& ingress,
                                 const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(nack, ingress.face, pitEntry);
}

} // namespace fw
} // namespace nfd
