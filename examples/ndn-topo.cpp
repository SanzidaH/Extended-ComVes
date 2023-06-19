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


void SentInterestCallback( uint32_t, shared_ptr<const ndn::Interest> );

void ReceivedDataCallback( uint32_t, shared_ptr<const ndn::Data>,uint32_t);

std::ofstream tracefile;


int
main(int argc, char* argv[])
{

  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));
  int run = 0;
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.AddValue("Run", "Run", run);
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-6-node.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/comves");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumer1 = Names::Find<Node>("s1");
  Ptr<Node> consumer2 = Names::Find<Node>("s2");

  Ptr<Node> producer1 = Names::Find<Node>("d1");
  Ptr<Node> producer2 = Names::Find<Node>("d2");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second

  // on the first consumer node install a Consumer application
  // that will express interests in /dst1 namespace
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.Install(consumer1);

  // on the second consumer node install a Consumer application
  // that will express interests in /dst2 namespace
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.Install(consumer2);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /dst1 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins("/prefix", producer1);
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer1);

  // Register /dst2 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst2 namespace
  ndnGlobalRoutingHelper.AddOrigins("/prefix", producer2);
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer2);
  
  
  std::string strcallback;
  std::string n = "0";
  
  strcallback = "/NodeList/"+n+"/ApplicationList/*/SentInterest";
  Config::ConnectWithoutContext( strcallback, MakeCallback( &SentInterestCallback ) );
  strcallback = "/NodeList/"+n+"/ApplicationList/*/ReceivedData";
  Config::ConnectWithoutContext( strcallback, MakeCallback( & ReceivedDataCallback ) );

  
    // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes();
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();
     // Choosing forwarding strategy
//  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/comves");
//  ndn::StrategyChoiceHelper::InstallAll("/update", "/localhost/nfd/strategy/comves"); 
     
     
  //Open trace file for writing
  char trace[100];
  sprintf( trace, "comves-run-%d.csv", run);/*0.1lf*/
  tracefile.open( trace, std::ios::out );
  tracefile << "nodeid,event,name,time,server" << std::endl;   
     
  Simulator::Stop(Seconds(5.0));
  //ns3::ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
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
