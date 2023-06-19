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

#include "uc.hpp"
#include "algorithm.hpp"

#include <ndn-cxx/util/random.hpp>

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(uc);
NFD_LOG_INIT(uc);

uc::uc(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
{
  NFD_LOG_DEBUG("uc.......");
  //sleep(2);
  //m_count = 0; // don't need it
  m_seq = 0; // we need it only for priod correction
  m_loadTable = {}; // important
  m_serverMap = {}; // don't need it
  m_hasServer = false; // we need it only for priod correction
  m_firsttime = true;
  m_max = 20;
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("uc does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "uc does not support version " + to_string(*parsed.version)));
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
              if(cost == 0){  //this part for periodic correction
			//	m_loadTable.insert({ interestName, {face, m_seq, 1, 0, 1} });
				m_cost[interestName].push_back({{face, {0, 0, 1}}});
              }else{
			//		m_loadTable.insert({ interestName, {face, m_seq, m_max, 0, 0} });
					m_cost[interestName].push_back({{face, {m_max, 0, 0}}});
              }             
              
         }
              // m_table structure: name, face, seq, server count, pending_int, hasServer - determines if router is a correction interest sender
             
      }
 
  }
        
for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( " Name: " << key);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
	    /* for (auto const& elem : inner_vec) {
                std::cout << "Element: " << elem << std::endl;
            }*/
        }
    }
}
  
}

const Name&
uc::getStrategyName()
{
  static const auto strategyName = Name("/localhost/nfd/strategy/uc").appendVersion(1);
  return strategyName;
}

void
uc::afterReceiveInterest(const Interest& interest, const FaceEndpoint& ingress,
                                     const shared_ptr<pit::Entry>& pitEntry)
{

   //Check m_table first
   std::string interestName = interest.getName().get(0).toUri();
   Face& inFace = ingress.face;
   NFD_LOG_INFO("interest received " << interest << ", incoming face: " << inFace.getId());
  
   const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
   const Fib& fib = this->getFib();
      
  bool incCost = false, found = false;
  int mtableCost = INT_MIN;
  int mtableFace;
  long unsigned int outface; 
  int newOutface;
  bool inTable = false;
  int serverCount = 0;
  int othernodeCount = 0;
  bool isServer = false;
    
  //insert all faces that is not in mtable for received name and initialize with zero (can we run it once?)
 

   for (const auto& nexthop : fibEntry.getNextHops()) {  //iterate all next hop in fib for received name
        Face& outFaceTemp = nexthop.getFace();
        uint64_t cost = nexthop.getCost();
        inTable = false;
	auto it = m_cost.find(interestName);

	// Check if the outer key was found
	if (it != m_cost.end()) {
	  // Find the inner key in the vector of maps
		 std::vector<std::map<int, std::vector<int>>> &inner_map_vector = it->second;
		 for (auto &inner_map : inner_map_vector) {
   			 auto inner_it = inner_map.find(boost::numeric_cast<int>(outFaceTemp.getId()));
   			 if (inner_it != inner_map.end()) {
     			 // Found the value for both outer and inner keys
     				// std::vector<int> &result = inner_it->second;
     				 // Do something with the result vector
    				 inTable = true;
					 break;
			}
		}
   	 }

    // Iterate over the outer map
 /*  	 for (const auto& outerMapPair : m_cost) {
		 std::cout << "Outer key: " << outerMapPair.first << std::endl;
		 if(outerMapPair.first == interestName){
       		 // Access the inner vector using the outer map iterator
       		 const auto& innerVector = outerMapPair.second;
       		 // Iterate over the inner vector to access each inner map
       		 for (const auto& innerMap : innerVector) {
           	 // Access the inner map key-value pairs using the inner map iterator
           		 for (const auto& innerMapPair : innerMap) {	
				 std::cout << "Inner key: " << innerMapPair.first << ", Values: ";
					if(!inTable && innerMapPair.first == outFaceTemp.getId()){
						inTable = true;
					}
					if(mtableCost < (innerMapPair.second[0]) &&
						 innerMapPair.first != boost::numeric_cast<int>(inFace.getId())){
					 //finding face with lowest cost
             				 mtableCost = innerMapPair.second[0];
             				 mtableFace = innerMapPair.first;
					}           			 
			    }
     		}
   		}
	 }*/

		if(!inTable){ //if not in mtable - insert in mtable
				NFD_LOG_INFO(" NEW entry initilaize * " << interestName << " "<< outFaceTemp.getId());
                if(cost == 0){
                	 m_cost[interestName].push_back({{boost::numeric_cast<int>(outFaceTemp.getId()), {0, 0, 1}}});
                }else{
             		 m_cost[interestName].push_back({{boost::numeric_cast<int>(outFaceTemp.getId()), {m_max, 0, 0}}});
                }

        }
   }
   // Check if m_table has entry for the incoming interest and find interface with lowest cost
bool flag = false;
for (auto const& [key, vec_of_maps] : m_cost) {
   // NFD_LOG_DEBUG( "Name: " << key);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
				
	  // if(inner_vec[2] == 1){
          //                               mtableCost = inner_vec[0];
          //                               mtableFace = inner_key;
          //                               found = true;
       	//				break;
	  // }
	  // else
		   if(mtableCost <= (inner_vec[0]) &&
						 inner_key != boost::numeric_cast<int>(inFace.getId())){
					 //finding face with lowest cost
             	//	NFD_LOG_DEBUG("mtableCost " << mtableCost << " is less than inner_vec[0] " << inner_vec[0] << " so update...");  
  					 mtableCost = inner_vec[0];
             				 mtableFace = inner_key;
					 found = true;

	     /*  if (mtableCost == inner_vec[0]) {
                    // 50% chance of updating the face
                    if (rand() % 2 == 0) {
                        mtableFace = inner_key;
			flag = true;
                        break;
                    }
                }*/
           } 
        if(flag)  break;
	}
    }
}

/*
 for (const auto& outerMapPair : m_cost) {
		 std::cout << "Outer key: " << outerMapPair.first << std::endl;
		 if(outerMapPair.first == interestName){
       		 // Access the inner vector using the outer map iterator
       		 const auto& innerVector = outerMapPair.second;
       		 // Iterate over the inner vector to access each inner map
       		 for (const auto& innerMap : innerVector) {
           	 // Access the inner map key-value pairs using the inner map iterator
           		 for (const auto& innerMapPair : innerMap) {	
				 std::cout << "Inner key: " << innerMapPair.first << ", Values: ";
				 if(mtableCost < (innerMapPair.second[0]) &&
						 innerMapPair.first != boost::numeric_cast<int>(inFace.getId())){
					 //finding face with lowest cost
             				 mtableCost = innerMapPair.second[0];
             				 mtableFace = innerMapPair.first;
            			  }           			 
			    }
			}
   		}
	 }
 */  
    NFD_LOG_DEBUG(" Selected in mtable: face <<" << mtableFace << ", cost << " << mtableCost << " interest " << interest.getName());
 
 
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

  for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "Before Update Name: " << newOutface);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
            /* for (auto const& elem : inner_vec) {
                std::cout << "Element: " << elem << std::endl;
            }*/
        }
    }
}

  
  


/*
for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;
    
    NFD_LOG_DEBUG(" updating m_cost Name: " << key);
    
    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;
        
        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;
           //find lowest loaded, highest cost
	   // 
            if (inner_key == newOutface && inner_vec[2] == 0) {
                inner_vec[0] -= 1;
            }
            
            NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0] << ", " << inner_vec[1] << ", " << inner_vec[2]);
        }
    }
}
*/


// initialize highest_cost to a very small value
/*
int highest_cost = std::numeric_limits<int>::min();

for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;

    NFD_LOG_DEBUG(" updating m_cost Name: " << key);

    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;

        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;

            if (inner_key == newOutface && inner_vec[2] == 0) {
                // update highest_cost if inner_vec[0] is greater
                if (inner_vec[0] > highest_cost) {
                    highest_cost = inner_vec[0];
                }
            }

          //  NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0] << ", " << inner_vec[1] << ", " << inner_vec[2]);
        }
    }
}*/

// decrement the highest_cost found earlier
bool decrementComplete = false;

for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;
//    int count_matching_entries = 0; // Track the number of matching entries
//    int selected_index = -1; // Index of the selected entry (-1 indicates not found)

    
    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;

        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;
         //   auto rng = std::default_random_engine {};
         //   std::shuffle(std::begin(inner_vec), std::end(inner_vec), rng);
            if (inner_key == mtableFace && inner_vec[0] == mtableCost) {
//			++count_matching_entries;

                    // Randomly select an entry based on count_matching_entries
//                    std::random_device rd;
//                    std::mt19937 generator(rd());
//                    std::uniform_int_distribution<int> distribution(1, count_matching_entries);
//                    int random_number = distribution(generator);

                    // Update selected_index based on random_number
//                    if (random_number == 1) {
//                        selected_index = i;
//                    }	    
                inner_vec[0] -= 1;
		decrementComplete = true;  // Set the flag to true indicating decrement is done
                break;
            }
        }
/*      if (selected_index != -1) {
            // Decrement the selected entry
            m_cost[key][selected_index][newOutface][0] -= 1;
            decrementComplete = true;  // Set the flag to true indicating decrement is done
            break;
        }*/
	 if (decrementComplete) {
            break;  // Break the inner loop
         }
    }

           if (decrementComplete) {
            break;  // Break the inner loop
        } 
}



    // Assuming m_cost map is already populated
/*
    for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
        auto const& key = it->first;
        auto& vec_of_maps = it->second;

        int count_matching_entries = 0; // Track the number of matching entries
        int selected_index = -1; // Index of the selected entry (-1 indicates not found)

        for (int i = 0; i < vec_of_maps.size(); ++i) {
            auto& inner_map = vec_of_maps[i];

            for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
                auto& inner_key = it_inner_key->first;
                auto& inner_vec = it_inner_key->second;

                if (inner_key == newOutface && inner_vec[2] == 0 && inner_vec[0] == mtableCost) {
                    ++count_matching_entries;

                    // Randomly select an entry based on count_matching_entries
                    std::random_device rd;
                    std::mt19937 generator(rd());
                    std::uniform_int_distribution<int> distribution(1, count_matching_entries);
                    int random_number = distribution(generator);

                    // Update selected_index based on random_number
                    if (random_number == 1) {
                        selected_index = i;
                    }
                }
            }
        }

        if (selected_index != -1) {
            // Decrement the selected entry
            m_cost[key][selected_index][newOutface][0] -= 1;
            decrementComplete = true;  // Set the flag to true indicating decrement is done
            break;
        }
    }
*/
for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "After Update m_cost Name: " << newOutface);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
        }
    }
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
   
}

/*
void
uc::broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
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
*/


void
uc::afterReceiveData(const Data& data, const FaceEndpoint& ingress,
                           const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG(" ** afterReceiveData pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());
  this->beforeSatisfyInterest(data, ingress, pitEntry);
  this->sendDataToAll(data, pitEntry, ingress.face);
}


void 
uc::beforeSatisfyInterest(const Data& data, const FaceEndpoint& ingress,
                                const shared_ptr<pit::Entry>& pitEntry)
{
/*  if(ingress.face.getId() <= 255) // this are virtual faces like the content store/ skip it
  {
    Strategy::beforeSatisfyInterest (data, ingress, pitEntry);
    return;
  }*/
  NFD_LOG_DEBUG("beforeSatisfyInterest pitEntry=" << pitEntry->getName()
                << " in=" << ingress.face.getId() << " data=" << data.getName());
                
  Block params = data.getMetaInfo().getAppMetaInfo().front();
  std::string device_hint((char*)params.value(), params.value_size());
  std::vector<std::string> netParams;
  netParams = SplitString( device_hint );
  int cost = 0;
  int face = 0;
  int serverhint = 0;

  cost = stoi( netParams[0] );
  serverhint = stoi( netParams[1] );

  int newOutface = boost::numeric_cast<int>(ingress.face.getId());
  NFD_LOG_INFO("number of process can be handled by Server: " << device_hint << "  ,face, " << newOutface);              
        	  
  std::string dataName = data.getName().get(0).toUri();	  
  // correction interest format: /update/seq/prefix/serverCount/load
  //m_table format: face, seq, server count, pending int, hasServer 

for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "Before correction m_cost Name: " << key);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
	    /* for (auto const& elem : inner_vec) {
                std::cout << "Element: " << elem << std::endl;
            }*/
        }
    }
}
    
  //  NFD_LOG_DEBUG("new entry for serverhint " << serverhint);
    bool exists = false;
    bool faceExist = false;
    auto& innerVector = m_cost[dataName];
    for (const auto& innerMap : innerVector) {
        if (innerMap.count(newOutface) > 0){
		faceExist = true;
		 NFD_LOG_DEBUG("multiple entry for  " << innerMap.count(newOutface) << " second check" << innerMap.at(newOutface)[2] << " " << serverhint);

	       if(innerMap.at(newOutface)[1] == serverhint ) {
          	  exists = true;
	  	  NFD_LOG_DEBUG("same server hint found...exist! "<< serverhint);
           	  break;
		}
        }
    }
    
    // Insert the entry if it doesn't already exist
    if (!exists && faceExist) {
  //  if (!exists){
	    NFD_LOG_DEBUG("does not exist! server: "<< serverhint << ", face: " << newOutface << ", cost: " << cost);
            innerVector.push_back({{newOutface, {cost, serverhint, 0}}});
	
    }

  
   
for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;

    NFD_LOG_INFO(" correcting m_cost Name: " << key << ", server " << serverhint << ", face: " << newOutface << ", cost: " << cost);

    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;

        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;
                        
	    if(inner_key == newOutface && inner_vec[2] == 1){
                                  break;
	    }
           
	    if(inner_key == newOutface && inner_vec[2] == 0){
               // inner_vec[0] -= 1;
		                  if(inner_vec[1] == serverhint){
                                         NFD_LOG_DEBUG("found serverhint " << serverhint << " updating from " << inner_vec[0] << " to " << cost);
                                         inner_vec[0] = cost; // Update
                                 }
                                  if(inner_vec[1] == 0){ // correction received so, intiliaztion value with servicehint 0 make 0, does not count
                                         NFD_LOG_DEBUG("updating cost of server 0 to 0" );
                                         inner_vec[0] = -2000;
                                 }
            }
	    else if(inner_key != newOutface && inner_vec[2] == 0 && inner_vec[1] == serverhint && inner_vec[1]!=0 && (inner_vec[0]+50>0) && inner_vec[0]!=cost ){
			inner_vec[0] = inner_vec[0] - 50;
	      // inner_vec[0] =inner_vec[0] -10;
	    }

            NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0] << ", " << inner_vec[1] << ", " << inner_vec[2]);
        }
    }
}
NFD_LOG_DEBUG("** leaving beforeSatisfyInterest for " << data.getName());	
    
/*
for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "After correction m_cost Name: " << key);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
	     for (auto const& elem : inner_vec) {
                std::cout << "Element: " << elem << std::endl;
            }
        }
    }
}
*/
             
}


void
uc::afterReceiveNack(const lp::Nack& nack, const FaceEndpoint& ingress,
                                 const shared_ptr<pit::Entry>& pitEntry)
{
int newOutface = boost::numeric_cast<int>(ingress.face.getId());
        NFD_LOG_DEBUG("afterReceiveNack in=" << ingress << " pitEntry=" << pitEntry->getName());      

// Retrieve the block from the NackHeader
//const ndn::Block& block = nack.getHeader();

// Access the value of the block
// Retrieve the Block from the Nack object
//const ndn::Block& block = nack.getHeader();

// Access the value of the block
//uint64_t value = block.value<uint64_t>();

// Retrieve the Nack header
//const ndn::lp::NackHeader& nackHeader = nack.getHeader();

// Retrieve the Block from the Nack header
//const ndn::Block& block = nackHeader.wireEncode();

// Access the value of the block
//uint64_t value = block.value<uint64_t>();
//const uint8_t* valueData = block.value();
//int value = std::atoi((const char *)valueData);
//uint64_t value = ndn::Block::fromBuffer(valueData, block.value_size()).toNumber<uint64_t>();


//	NFD_LOG_DEBUG(" ***************** hello  " << value);
for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "Before Nack m_cost Name: " << newOutface);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
        }
    }
}



// initialize lowest_cost to a very high value; adding penalty to the lowest cost entry since it has the highest prob of getting overloaded
int lowest_cost = INT_MAX;

for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;

    NFD_LOG_DEBUG(" updating m_cost after receiving Nack Name: " << key);

    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;

        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;

            if (inner_key == newOutface && inner_vec[2] == 0 && inner_vec[1]!=0 && (inner_vec[0] + 200) > 0) {
                // update highest_cost if inner_vec[0] is greater
                if (inner_vec[0] < lowest_cost) {
                    lowest_cost = inner_vec[0];
                }
            }

//            NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0] << ", " << inner_vec[1] << ", " << inner_vec[2]);
        }
    }
}

        int hintTemp = 0;
        int penaltyTemp = 0;

// decrement the highest_cost found earlier
bool penaltyAdd = false; 
for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;

    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;
       // int hintTemp = 0;
       // int penaltyTemp = -300;
        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;
            bool fnd = false;
            if (inner_key == newOutface && inner_vec[2] == 0 && inner_vec[0] == lowest_cost 
			    && (inner_vec[0] + 100 > 0) ) {
          //      inner_vec[0] = inner_vec[0]-300;
		penaltyTemp = inner_vec[0];
		fnd = true;
		penaltyAdd = true;
		hintTemp = inner_vec[1];
                break;
	        NFD_LOG_DEBUG("hint<<" << inner_vec[1]);
            }
	   // else if(inner_key != newOutface && penaltyAdd == true && inner_vec[1] == hintTemp){
             //    inner_vec[0] = penaltyTemp;
	   // }

	         //   if(fnd==true && inner_vec[1]!=0){
	         //       break;
	         //   }
        }
	if(penaltyAdd) break;
//	if(penaltyAdd && rand() % 2 == 0) break;
    }
  //  if(penaltyAdd) break;
}


/*
for (auto it = m_cost.begin(); it != m_cost.end(); ++it) {
    auto const& key = it->first;
    auto& vec_of_maps = it->second;

    for (auto it_inner = vec_of_maps.begin(); it_inner != vec_of_maps.end(); ++it_inner) {
        auto& inner_map = *it_inner;
        for (auto it_inner_key = inner_map.begin(); it_inner_key != inner_map.end(); ++it_inner_key) {
            auto& inner_key = it_inner_key->first;
            auto& inner_vec = it_inner_key->second;
            bool fnd = false;
            if (inner_key != newOutface && inner_vec[2] == 0 && inner_vec[1] == hintTemp) {
                inner_vec[0] = penaltyTemp;
            }
        }
    }
}
*/

for (auto const& [key, vec_of_maps] : m_cost) {
    NFD_LOG_DEBUG( "After Nack m_cost Name: " << newOutface);
    for (auto const& inner_map : vec_of_maps) {
        for (auto const& [inner_key, inner_vec] : inner_map) {
           // NFD_LOG_DEBUG("Face: " << inner_key << ", (cost, serverHint, isServer)  ");
           NFD_LOG_DEBUG("Face: " << inner_key << ":(cost, serverhint, isServer) " << inner_vec[0]<< ", "<< inner_vec[1]<<", " <<  inner_vec[2] );
        }
    }
}



/*
  int newOutface = boost::numeric_cast<int>(ingress.face.getId());
  bool found = false;
	std::string dataName = nack.getInterest().getName().get(0).toUri();
  // correction interest format: /update/seq/prefix/serverCount/load
  //m_table format: face, seq, server count, pending int, hasServer

            for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]

             NFD_LOG_DEBUG("** mtable check before nack: dataName " << nack.getInterest().getName() << " face " <<  itr -> second[0] << " pending_int " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4]);

          }

  for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){

        if (itr -> first == dataName && itr -> second[0] == newOutface && itr -> second[4]== 0){
            //NFD_LOG_DEBUG("*** before "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
            found = true;
            //if(itr -> second[3] > 0)
            itr -> second[3] = itr -> second[3] + 1;// decrementing
           // NFD_LOG_DEBUG("*** after "<< itr->first   << " " << itr->second[0] << " "<< itr->second[1] << " " << itr->second[2] << " " << itr->second[3]);
        }
         if(found) break;
  }

              for (auto itr = m_loadTable.begin(); itr != m_loadTable.end(); itr++){ // we need only face and pending interest, [0], [3]

             NFD_LOG_DEBUG("** mtable check after nack: dataName " << nack.getInterest().getName() << " face " <<  itr -> second[0] << " pending_int " << itr -> second[3] << " seq " << itr -> second[1] << " serverCount " << itr -> second[2]  << " hasServer " << itr -> second[4]);

          }
*/
     // this->sendNack(nack.getHeader(), ingress.face, pitEntry);
    // this->rejectPendingInterest(pitEntry);
	
  //this->rejectPendingInterest(pitEntry);
  this->processNack(nack, ingress.face, pitEntry);
  this->setExpiryTimer(pitEntry, 0_ms);
  this->rejectPendingInterest(pitEntry);

}



} // namespace fw
} // namespace nfd
