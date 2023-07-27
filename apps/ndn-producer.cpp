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

#include "ndn-producer.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include <time.h>

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>


#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include <algorithm>

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>
 ///
#include <ndn-cxx/lp/tags.hpp>
#include <ndn-cxx/link.hpp>
#include "ndn-cxx/meta-info.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"

#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include "helper/ndn-fib-helper.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>
#include <fstream>
#include <ndn-cxx/meta-info.hpp>
#include <ndn-cxx/encoding/tlv.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include "common/logger.hpp"
#include <ndn-cxx/lp/tags.hpp>


NS_LOG_COMPONENT_DEFINE("ndn.Producer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Producer);

TypeId
Producer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::Producer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<Producer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&Producer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&Producer::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&Producer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Utilization", "Utilization of service", UintegerValue(15),
                    MakeUintegerAccessor(&Producer::m_util),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&Producer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&Producer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&Producer::m_keyLocator), MakeNameChecker())
      .AddTraceSource("ServerUpdate", "ServerUpdate",
                      MakeTraceSourceAccessor(&Producer::m_serverUpdate),
                      "ns3::ndn::Producer::ServerUpdateTraceCallback")
      .AddAttribute(
            "Capacity",
            "Edge node capacity",
            UintegerValue(100), MakeUintegerAccessor(&Producer::m_capacity),
            MakeUintegerChecker<uint32_t>())
      .AddAttribute(
            "Seed",
            "Seed for run",
            UintegerValue(100), MakeUintegerAccessor(&Producer::m_seed),
            MakeUintegerChecker<uint32_t>())
      .AddAttribute(
            "Hint",
            "Server Hint",
            UintegerValue(0), MakeUintegerAccessor(&Producer::m_hint),
            MakeUintegerChecker<uint32_t>())

                    ;
  return tid;
}

Producer::Producer()
	: m_rand(CreateObject<UniformRandomVariable>())
{
  NS_LOG_FUNCTION_NOARGS();
  
}

// inherited from Application base class.
void
Producer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();
  serviceCount = 0;
  srand( m_seed*1000 );
  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
//  m_utilization = 20;
  m_utilization = rand()%10+20;
  Simulator::Schedule(Seconds(0.02), &Producer::PrintResource,this, 0);
}

void
Producer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
Producer::PrintExpired(std::string name){
        int util = m_util;
        int utilLeft = m_capacity - m_utilization;
          int serverLeft =std::round(static_cast<double>(utilLeft)/m_util);
//	int serverLeft =std::ceil(static_cast<double>(utilLeft)/m_util);
//	int serverLeft =std::floor(static_cast<double>(utilLeft)/m_util);
	 m_serverUpdate(GetNode()->GetId(), name,serverLeft , "Expired");
}

void 
Producer::PrintResource(uint32_t task_util)
{
	int util = m_util;
	int utilLeft = m_capacity - m_utilization;
	//  int serverLeft =std::round(static_cast<double>(utilLeft)/m_util);
int serverLeft =std::floor(static_cast<double>(utilLeft)/m_util);
	//int serverLeft =std::ceil(static_cast<double>(utilLeft)/m_util);
//	int serverLeft = (utilLeft/m_util);
  	 NS_LOG_INFO("Current Resource: " <<  serverLeft);
         m_serverUpdate(GetNode()->GetId(), "", serverLeft , "Current Resource");	
	 Simulator::Schedule(Seconds(0.02), &Producer::PrintResource,this, 0);
}

void
Producer::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);
  if (!m_active)
    return;
//   NS_LOG_INFO("*service count " <<  serviceCount << " m_utilization: " << m_utilization);
//  double utilTemp = (m_utilization/m_capacity);
//  NS_LOG_INFO(m_utilization << " m_utilization ********** m_utilization/100: " << utilTemp);
  //float computeTime = ((float) rand()/RAND_MAX) + utilTemp;
//  double computeTime = utilTemp;
 
//   double computeTime = 0;

//  NS_LOG_INFO("*** computeTime: " << computeTime);
  int util = m_util;
  int utilLeft = m_capacity - m_utilization;
//  int serverLeft =std::round(static_cast<double>(utilLeft)/m_util);
//int serverLeft = (utilLeft/m_util);
//int serverLeft =std::ceil(static_cast<double>(utilLeft)/m_util);
   int serverLeft =std::floor(static_cast<double>(utilLeft)/m_util);
//int utilLeft2 = m_capacity - (m_utilization + m_util);
//int serverLeft2 =std::floor(static_cast<double>(utilLeft2)/m_util);
 // if((m_utilization + util) > m_capacity){  
  if(serverLeft < 1){
	  //  computeTime = 5;
    NS_LOG_INFO("OVERLOADED");
    double ctime = 2.00;
    m_serverUpdate(GetNode()->GetId(), interest->getName().toUri(), serverLeft , "OVERLOADED");
    m_serverUpdate(GetNode()->GetId(), std::to_string(ctime), serverLeft, "Computation time");
    Simulator::Schedule(Seconds(2), &Producer::PrintExpired,this, interest->getName().toUri());
    SendNack(interest);
  }else{ 
   // m_serverUpdate(GetNode()->GetId(), interest->getName().toUri() , serviceCount, "Data Processing");	  
    serviceCount++;	 
    m_utilization += util;
    utilLeft = m_capacity - m_utilization;
   // serverLeft = (utilLeft/m_util);
   // int serverLeft =std::ceil(static_cast<double>(utilLeft)/m_util);
  int serverLeft =std::floor(static_cast<double>(utilLeft)/m_util);
    // serverLeft =std::round(static_cast<double>(utilLeft)/m_util);
    m_serverUpdate(GetNode()->GetId(), interest->getName().toUri() , serverLeft, "Data Processing");
    NS_LOG_INFO(interest->getName() << "  m_utilization "<< m_utilization  << " will be increased by " << util);
    //m_utilization += util;
   // computeTime = (m_utilization/m_capacity);
    
    double computeTime = static_cast<double>(m_utilization) / m_capacity;
    float diff = 0.0009-0.0001;
     NS_LOG_INFO("diff:"<<diff);
     computeTime = computeTime + (((float) rand() / RAND_MAX) * diff) + 0.0001;
     m_serverUpdate(GetNode()->GetId(), std::to_string(computeTime), serverLeft, "Computation time");
//    if ((computeTime*100) < 30){
//	    computeTime = 0.2;
//    }

  //  double remaining = m_capacity - m_utilization;
  //  double computeTime =static_cast<double>(remaining) /static_cast<double>(m_utilization);
    NS_LOG_INFO("*service count " <<  serviceCount << " m_utilization: " << m_utilization << " m_capacity: " << m_capacity << " computeTime "<< computeTime);
//   NS_LOG_INFO(m_utilization << " m_utilization ********** m_utilization/m_capacity: " << computeTime);
    NS_LOG_INFO(interest->getName() << " Task started processing:  m_utilization increased to: " << m_utilization << ", resource will be released by " << util << "  after " << computeTime << " sec");
    Simulator::Schedule (Seconds(computeTime), &Producer::OnReceiveInitialRequest, this, interest, util);
   // utilDecrease[dataName] = util;
   // Simulator::Schedule(Seconds(computeTime), &EdgeServerProactive::DecreaseUtil, this, util);
  }
  
 // Simulator::Schedule(Seconds(ttc), &Producer::OnReceiveInitialRequest, this);
  
}


void
Producer::SendNack(shared_ptr<const Interest> interest)
{
  NS_LOG_INFO("node(" << GetNode()->GetId() << ") is overloaded. Responding with NACK: " << interest->getName());
  lp::Nack nack(std::move(*interest));
//  nack.setReason(lp::NackReason::NONE);
  // Create a Block object with value "4"
//  ndn::Block block(4);
// Create a NackHeader object using the block
// ndn::lp::NackHeader nackHeader(block);
 // nack.setHeader(nackHeader);
  nack.setReason(lp::NackReason::NONE);
  m_transmittedNacks(make_shared<lp::Nack>(nack), this, m_face);
  m_appLink->onReceiveNack(nack);

  NS_LOG_INFO("NackSent " << interest->getName()<<", reason:Overloaded");
}

void
Producer::OnReceiveInitialRequest(shared_ptr<const Interest> interest, uint32_t task_util)
{

  m_utilization = m_utilization - task_util;
  NS_LOG_INFO("task completed: m_utilization " << m_utilization << "  decreased by " << task_util);
  
  Name dataName(interest->getName());
  // dataName.append("hello");
  // dataName.appendVersion();

 
  
  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
  
  std::shared_ptr<::ndn::MetaInfo> metainf = make_shared<::ndn::MetaInfo>();
 // const Interest* metaInfo;  
   // serviceCount--;
  
    serviceCount--;
    int utilLeft = m_capacity - m_utilization; 
    int serverLeft =std::floor(static_cast<double>(utilLeft)/m_util);
  //  int serverLeft =std::ceil(static_cast<double>(utilLeft)/m_util);
   // int serverLeft = (utilLeft/m_util); 
   // int serverLeft =std::round(static_cast<double>(utilLeft)/m_util);
    // NS_LOG_INFO("service count " <<  serviceCount);
   // std::string pop_field = " " + std::to_string(serviceCount);


    NS_LOG_INFO("** serverLeft  " <<  serverLeft);
    std::string pop_field = std::to_string(serverLeft) + " " + std::to_string(m_hint);

   // NS_LOG_INFO("service count " <<  serviceCount);
   // std::string pop_field = " " + std::to_string(serviceCount);
    
    //updateBuffer();
    Block metainf_block = ::ndn::encoding::makeStringBlock(150, pop_field);

    std::string temp = ::ndn::encoding::readString(metainf_block);
    metainf->addAppMetaInfo(metainf_block);
    data->setMetaInfo(*metainf);
//  m_serverUpdate(GetNode()->GetId(), interest->getName().toUri(), serviceCount , "Data Correction"); 
    m_serverUpdate(GetNode()->GetId(), interest->getName().toUri(), serverLeft , "Data Correction");
//ndn::MetaInfo info;
  //info.setFreshnessPeriod(m_freshnessPeriod);
  
  //auto mi = make_shared<MetaInfo>();
  //MetaInfo mi = ::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::MetaInfo, 20); 
  //interest->setParameters(reinterpret_cast<const unsigned char *>(m_hint.toUri().c_str()), m_hint.toUri().size());
  //Name m_hint;
 // m_hint = "20";
  //data->setMetaInfo(static_cast< ::ndn::tlv::MetaInfo>(255));
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  data->setSignatureInfo(signatureInfo);

  ::ndn::EncodingEstimator estimator;
  ::ndn::EncodingBuffer encoder(estimator.appendVarNumber(m_signature), 0);
  encoder.appendVarNumber(m_signature);
  data->setSignatureValue(encoder.getBuffer());

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);

  

}

} // namespace ndn
} // namespace ns3
