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

// ndn-simple.cpp

#include "ns3/core-module.h"

#include "ns3/network-module.h"

#include "ns3/point-to-point-module.h"

#include "ns3/ndnSIM-module.h"

#include "ns3/int64x64.h"
#include <ndn-cxx/lp/tags.hpp>

#include <fstream>

#include <iomanip>

#include <iostream>

#include <stdlib.h>

#include <unordered_map>


namespace ns3 {

void SentInterestCallback( uint32_t, shared_ptr<const ndn::Interest> );

void ReceivedDataCallback( uint32_t, shared_ptr<const ndn::Data>,uint32_t);

void DataCallback( uint32_t, std::string, std::string);


//  typedef void (*NackTraceCallback)( uint32_t,  std::string, shared_ptr<const lp::Nack> nack);
void NackCallback( uint32_t, std::string, shared_ptr<const ndn::lp::Nack>);
void ServerUpdateCallback( uint32_t nodeid, std::string server, int serverUtil, std::string service);

std::ofstream tracefile;
std::ofstream tracefile2;
std::ofstream tracefile3;
std::vector<std::string> SplitString(std::string strLine);

int
main(int argc, char * argv[]) {
    // setting default parameters for PointToPoint links and channels
    //Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
    //Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
    //Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));
    Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));
  int run = 0, duration = 20; 
  int correction = 1;
  std::string freq = "10", util = "15";
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.AddValue("Run", "Run", run);
  cmd.AddValue("freq", "freq", freq);
  cmd.AddValue("dur", "dur", duration);
  cmd.AddValue("util", "util", util);
  cmd.AddValue("cor", "cor", correction);
  cmd.Parse(argc, argv);

    srand(run);
    PointToPointHelper p2p;

    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;

    int clientcount = 0;
    // int basecount = 0;
    ifstream configFile("src/ndnSIM/examples/topologies/topo3.txt", std::ios::in); // Topology file
    std::string strLine, strcallback;
    bool gettingNodeCount = false, buildingNetworkTopo = false, assignServers = false;
    bool assignBases= false, assignClients = false, assignPECs = false;
    NodeContainer nodes;
    int nodeCount = 0;
    std::vector < std::string > netParams;


    if (configFile.is_open()) {

      while (std::getline(configFile, strLine)) {

        // Determine what operation is ongoing while reading the config file
        if (strLine.substr(0, 7) == "BEG_000") {
          gettingNodeCount = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_000") {
          // Create nodes
          gettingNodeCount = false;
          nodes.Create(nodeCount);
          continue;
        }
        if (strLine.substr(0, 7) == "BEG_001") {
          buildingNetworkTopo = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_001") {
          buildingNetworkTopo = false;
          ndn::StackHelper ndnHelper;
          ndnHelper.InstallAll();
          ndnGlobalRoutingHelper.Install(nodes);

          continue;
        }
        if (strLine.substr(0, 7) == "BEG_002") {
          assignServers = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_002") {
          assignServers = false;
          continue;
        }
        if (strLine.substr(0, 7) == "BEG_003") {
          assignBases = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_003") {
          assignBases = false;
          continue;
        }
        if (strLine.substr(0, 7) == "BEG_004") {
          assignClients = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_004") {
          assignClients = false;
          continue;
        }
        if (strLine.substr(0, 7) == "BEG_005") {
          assignPECs = true;
          continue;
        }
        if (strLine.substr(0, 7) == "END_005") {
          assignPECs = false;
          continue;
        }

        if (gettingNodeCount == true) {

          // Getting number of nodes to create
          netParams = SplitString(strLine);
          nodeCount = stoi(netParams[0]);

        } else if (buildingNetworkTopo == true) {

          // Building network topology
          netParams = SplitString(strLine);
          p2p.SetDeviceAttribute("DataRate", StringValue(netParams[2]));
          p2p.SetChannelAttribute("Delay", StringValue(netParams[3]));
          p2p.Install(nodes.Get(std::stoi(netParams[0])), nodes.Get(std::stoi(netParams[1])));

        } else if (assignServers == true) { // || assignPECs == true
          netParams = SplitString(strLine);

          //ndnGlobalRoutingHelper.AddOrigins("/producer" + std::to_string(servercount), nodes.Get(std::stoi(netParams[0])));
          ndnGlobalRoutingHelper.AddOrigins("/prefix", nodes.Get(std::stoi(netParams[0])));
          producerHelper.SetPrefix("/prefix");
          //producerHelper.SetAttribute("Hint", StringValue("/producer" + std::to_string(servercount)));
          producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
            producerHelper.SetAttribute("Utilization", StringValue(util));
			producerHelper.SetAttribute("Capacity", StringValue("100"));
          //producerHelper.SetAttribute("Capacity", UintegerValue(3000));
          producerHelper.Install(nodes.Get(std::stoi(netParams[0])));

    		strcallback = "/NodeList/"+netParams[0]+"/ApplicationList/*/ServerUpdate";
  				Config::ConnectWithoutContext( strcallback, MakeCallback( & ServerUpdateCallback ) ); 


        } else if (assignClients == true) {
		  clientcount++;
          netParams = SplitString(strLine);
          consumerHelper.SetPrefix("/prefix");
          consumerHelper.SetAttribute("Frequency", StringValue(freq)); // 10 interests a second
          ndnGlobalRoutingHelper.AddOrigins("/update",nodes.Get(0));  
          consumerHelper.SetAttribute("StartSeq", IntegerValue( clientcount*91 ));
		  //consumerHelper.SetAttribute("Task", StringValue("/prefix"));
          //consumerHelper.SetAttribute("Hint", StringValue("/client" + std::to_string(clientcount)));
          //ndnGlobalRoutingHelper.AddOrigins("/client", nodes.Get(std::stoi(netParams[0])));
          //ndnGlobalRoutingHelper.AddOrigins("/util", nodes.Get(std::stoi(netParams[0])));
          consumerHelper.Install(nodes.Get(std::stoi(netParams[0])));

          

          std::string strcallback;
          std::string n = netParams[0]; 
   
          strcallback = "/NodeList/"+n+"/ApplicationList/*/SentInterest";
          Config::ConnectWithoutContext( strcallback, MakeCallback( &SentInterestCallback ) );
          strcallback = "/NodeList/"+n+"/ApplicationList/*/ReceivedData";
          Config::ConnectWithoutContext( strcallback, MakeCallback( & ReceivedDataCallback ) ); 
		  
  strcallback = "/NodeList/"+n+"/ApplicationList/*/Data";
  Config::ConnectWithoutContext( strcallback, MakeCallback( & DataCallback ) );

  strcallback = "/NodeList/"+n+"/ApplicationList/*/Nack";
  Config::ConnectWithoutContext( strcallback, MakeCallback( & NackCallback ) );
          

        } else {
          std::cout << "reading something else " << strLine << std::endl;
        }
      } // end while
    } else {
      std::cout << "Cannot open configuration file!!!" << std::endl;
      exit(1);
    }

    configFile.close();

    ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();
      if(correction == 1 ){
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/uc");
  } else{
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/comves");
  }
    
  //Open trace file for writing
  char trace[100];
//   sprintf( trace, "comves-run-%d.csv", run);/*0.1lf*/
  sprintf( trace, "comves-%d-%d-%d.csv", run, correction,std::stoi(freq));/*0.1lf*/
  tracefile.open( trace, std::ios::out );
  tracefile << "nodeid,event,name,time,server" << std::endl;   

  sprintf( trace, "server-%d-%d-%d-.csv", run, std::stoi(freq), correction);/*0.1lf*/
  tracefile2.open( trace, std::ios::out );
  tracefile2 << "nodeid,service,serviceCount,status"<< std::endl;

  sprintf( trace, "client-%d-%d-%d-.csv", run, std::stoi(freq), correction);/*0.1lf*/
  tracefile3.open( trace, std::ios::out );
  tracefile3 << "nodeid,event,service,time"<< std::endl;


  Simulator::Stop(Seconds(70));
//  ns3::ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
  Simulator::Run();
  Simulator::Destroy();

    return 0;
  }

void SentInterestCallback( uint32_t nodeid, shared_ptr<const ndn::Interest> interest){
  tracefile << nodeid << ",sent," << interest->getName().getSubName(0,2).toUri().erase(0,1) << "," << std::fixed << setprecision( 9 ) <<
          ( Simulator::Now().GetSeconds() ) << std::endl;
}

void ReceivedDataCallback( uint32_t nodeid, shared_ptr<const ndn::Data> data, uint32_t ttc ){

         tracefile << nodeid << ",received," <<  data->getName().getSubName(0,2).toUri().erase(0,1) << "," << std::fixed << setprecision( 9 ) <<
         ((Simulator::Now().GetSeconds()))  << std::endl;
		 
}

void DataCallback( uint32_t nodeid, std::string name, std::string event ){


//         tracefile << nodeid <<"," << event << "," << name << "," << std::fixed << setprecision( 9 ) <<
//         ((Simulator::Now().GetSeconds()))  << std::endl;

         tracefile3 << nodeid << "," << event << "," <<  name << "," << std::fixed << setprecision( 9 ) <<
         ((Simulator::Now().GetSeconds()))  << std::endl;

}

void NackCallback( uint32_t nodeid, std::string name,  shared_ptr<const ndn::lp::Nack> nack ){


//         tracefile << nodeid <<"," << event << "," << name << "," << std::fixed << setprecision( 9 ) <<
//         ((Simulator::Now().GetSeconds()))  << std::endl;

         tracefile3 << nodeid << "," << nack->getReason() << "," <<  nack->getInterest().getName().toUri() << "," << std::fixed << setprecision( 9 ) <<
         ((Simulator::Now().GetSeconds()))  << std::endl;

}

void ServerUpdateCallback( uint32_t nodeid, std::string server, int serverUtil, std::string service){
  tracefile2 << nodeid << "," << server << "," << serverUtil <<","<<service<< "," << std::fixed << setprecision( 9 ) <<
          ( Simulator::Now().GetNanoSeconds() )/1000000000.0<<  std::endl;
}

  //Split a string delimited by space
std::vector < std::string > SplitString(std::string strLine) {
    std::string str = strLine;
    std::vector < std::string > result;
    std::istringstream isstr(str);
    for (std::string str; isstr >> str;)
      result.push_back(str);

    return result;
  }

} // namespace ns3

int
main(int argc, char * argv[]) {
  return ns3::main(argc, argv);
}
