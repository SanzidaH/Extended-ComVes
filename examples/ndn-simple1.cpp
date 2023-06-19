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

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

void SentInterestCallback( uint32_t, shared_ptr<const ndn::Interest> );

void ReceivedDataCallback( uint32_t, shared_ptr<const ndn::Data>,uint32_t);

void ServerUpdateCallback( uint32_t nodeid, std::string server, int serverUtil, std::string service);

std::ofstream tracefile;
std::ofstream tracefile2;
std::vector<std::string> SplitString(std::string strLine);


int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
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

  // Creating nodes
  //
  int client = 2, server = 5, totalnode = 15; 
  NodeContainer nodes;
  nodes.Create(totalnode);
  // Connecting nodes using two links
  PointToPointHelper p2p;

  /*
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(2), nodes.Get(4));
  p2p.Install(nodes.Get(2), nodes.Get(5));
  p2p.Install(nodes.Get(2), nodes.Get(6));
 // p2p.Install(nodes.Get(2), nodes.Get(7));
 // p2p.Install(nodes.Get(3), nodes.Get(8));
  //  p2p.Install(nodes.Get(3), nodes.Get(9));  

  p2p.Install(nodes.Get(3), nodes.Get(7));
  p2p.Install(nodes.Get(1), nodes.Get(11));
  p2p.Install(nodes.Get(11), nodes.Get(8));
//  p2p.Install(nodes.Get(11), nodes.Get(12));
//  p2p.Install(nodes.Get(10), nodes.Get(12));
//  p2p.Install(nodes.Get(11), nodes.Get(10));
*/


  p2p.Install(nodes.Get(0), nodes.Get(2));
  p2p.Install(nodes.Get(2), nodes.Get(3));
  p2p.Install(nodes.Get(2), nodes.Get(4));
  p2p.Install(nodes.Get(2), nodes.Get(5));
//  p2p.Install(nodes.Get(3), nodes.Get(1));
  p2p.Install(nodes.Get(6), nodes.Get(7));
  p2p.Install(nodes.Get(6), nodes.Get(1));
  p2p.Install(nodes.Get(3), nodes.Get(7));
  p2p.Install(nodes.Get(4), nodes.Get(8));
  p2p.Install(nodes.Get(5), nodes.Get(9));
  p2p.Install(nodes.Get(5), nodes.Get(10));
  p2p.Install(nodes.Get(5), nodes.Get(11));
  p2p.Install(nodes.Get(5), nodes.Get(12));
//  p2p.Install(nodes.Get(11), nodes.Get(8));
//  p2p.Install(nodes.Get(11), nodes.Get(12));
//  p2p.Install(nodes.Get(10), nodes.Get(12));
//  p2p.Install(nodes.Get(11), nodes.Get(10));


  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.Install( nodes );
  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  // consumerHelper.SetPrefix("/prefix/"+std::to_string(rand()%3+1));
   
  std::string strcallback;
  std::string n = "";

  for(int i = 0 ; i < 2; i++  ){
	consumerHelper.SetPrefix("/prefix");
  	if(i==1){
		int temp = std::stoi(freq) / 10;
		consumerHelper.SetAttribute("Frequency", StringValue(std::to_string(temp)));
	}else{
	consumerHelper.SetAttribute("Frequency", StringValue(freq)); // 10 interests a second
	}
  	ndnGlobalRoutingHelper.AddOrigins("/update",nodes.Get(i));
        consumerHelper.Install(nodes.Get(i));

	n = std::to_string(i);
  strcallback = "/NodeList/"+n+"/ApplicationList/*/SentInterest";
  Config::ConnectWithoutContext( strcallback, MakeCallback( &SentInterestCallback ) );
  strcallback = "/NodeList/"+n+"/ApplicationList/*/ReceivedData";
  Config::ConnectWithoutContext( strcallback, MakeCallback( & ReceivedDataCallback ) );

  }
/*
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue(freq)); // 10 interests a second
  ndnGlobalRoutingHelper.AddOrigins("/update",nodes.Get(0));
  auto apps = consumerHelper.Install(nodes.Get(0));  
  
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue(freq)); // 10 interests a second
  ndnGlobalRoutingHelper.AddOrigins("/update",nodes.Get(10));
  consumerHelper.Install(nodes.Get(10));

  std::string strcallback;
  std::string n = "0"; 
*/
  // first node
//  apps.Stop(Seconds(5.0)); // stop the consumer app at 10 seconds mark

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Utilization", StringValue(util));
  //producerHelper.Install(nodes.Get(2)); 
/*
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(4));
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(5));
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(6)); 
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(7));
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(8));
  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(9));


  ndnGlobalRoutingHelper.AddOrigins("/prefix/1",nodes.Get(4));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/1",nodes.Get(5));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/1",nodes.Get(6)); 

  ndnGlobalRoutingHelper.AddOrigins("/prefix/2",nodes.Get(4));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/2",nodes.Get(5));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/2",nodes.Get(6)); 

  ndnGlobalRoutingHelper.AddOrigins("/prefix/3",nodes.Get(4));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/3",nodes.Get(5));
  ndnGlobalRoutingHelper.AddOrigins("/prefix/3",nodes.Get(6)); 


  producerHelper.Install(nodes.Get(1)); // last node
  producerHelper.Install(nodes.Get(2)); // last node
  producerHelper.Install(nodes.Get(3)); // last node
  producerHelper.Install(nodes.Get(4)); // last node    
  producerHelper.Install(nodes.Get(5)); // last node 
  producerHelper.Install(nodes.Get(6)); // last node
*/
// int router = totalnode-(client+server); 
// for(int i = totalnode-(client+router); i < totalnode; i++){
 for(int i = 7; i < 13; i++){
	  ndnGlobalRoutingHelper.AddOrigins("/prefix",nodes.Get(i));
	  producerHelper.Install(nodes.Get(i)); // last node
    	  strcallback = "/NodeList/"+std::to_string(i)+"/ApplicationList/*/ServerUpdate";
  				Config::ConnectWithoutContext( strcallback, MakeCallback( & ServerUpdateCallback ) ); 
 }

    // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes();
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();
     // Choosing forwarding strategy
  if(correction == 1 ){
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/ccomves");
  } else{
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/comves");
  }
//  ndn::StrategyChoiceHelper::InstallAll("/update", "/localhost/nfd/strategy/comves"); 
     
     
  //Open trace file for writing
  char trace[100];
//   sprintf( trace, "comves-run-%d.csv", run);/*0.1lf*/
  sprintf( trace, "comves-%d-%d-%d.csv", run, correction,std::stoi(freq));/*0.1lf*/
  tracefile.open( trace, std::ios::out );
  tracefile << "nodeid,event,name,time,server" << std::endl;   

  sprintf( trace, "server-%d-%d-%d-.csv", run, correction,std::stoi(freq));/*0.1lf*/
  tracefile2.open( trace, std::ios::out );
  tracefile2 << "nodeid,service,serviceCount,status"<< std::endl;

  Simulator::Stop(Seconds(20));
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

void ServerUpdateCallback( uint32_t nodeid, std::string server, int serverUtil, std::string service){
  tracefile2 << nodeid << "," << server << "," << serverUtil <<","<<service<< "," << std::fixed << setprecision( 9 ) <<
          ( Simulator::Now().GetNanoSeconds() )/1000000000.0<<  std::endl;
}

//Split a string delimited by space
std::vector<std::string> SplitString(std::string strLine) {
  std::string str = strLine;
  std::vector<std::string> result;
  std::istringstream isstr(str);

  for(std::string str; isstr >> str; )
    result.push_back(str);

  return result;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
