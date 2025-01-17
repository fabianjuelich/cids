/*
 * Copyright (c) 2009 The Boeing Company
 * Modifications Copyright (c) 2025 Fabian Jülich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

// #define PRELIMINARY

#define FRIIS "ns3::FriisPropagationLossModel"
#define FIXED_RSS "ns3::FixedRssLossModel"
#define THREE_LOG_DISTANCE "ns3::ThreeLogDistancePropagationLossModel"
#define TWO_RAY_GROUND "ns3::TwoRayGroundPropagationLossModel"
#define NAKAGAMI "ns3::NakagamiPropagationLossModel"

#define UDP_IP_HEADER_SIZE_BYTES 28

#define DURATION_MAX 120
#define DURATION_STEP 1
#define DISTANCE_MAX 251
#define DISTANCE_STEP 5

NS_LOG_COMPONENT_DEFINE("CidsProject");

#ifdef PRELIMINARY
std::ofstream outputFile("pre_results.csv");
#else
std::ofstream outputFile("results.csv");
#endif

void LogResults(const std::string &modelName, double distance, double duration, double signalStrength, double throughput) {
    if (outputFile.is_open()) {
        outputFile << modelName << "," << distance << "," << duration << ","  << signalStrength << "," << throughput << "\n";
    }
}

double signalStrength = 0;

void SnifferRx(Ptr<Packet const> packet, unsigned short x, WifiTxVector wtv, MpduInfo mi, SignalNoiseDbm snd, unsigned short y)
{
    signalStrength = snd.signal;
}

static void
GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval)
{
    if (pktCount > 0)
    {
        socket->Send(Create<Packet>(pktSize));
        Simulator::Schedule(pktInterval,
                            &GenerateTraffic,
                            socket,
                            pktSize,
                            pktCount - 1,
                            pktInterval);
    }
    else
    {
        socket->Close();
    }
}

int
main(int argc, char* argv[])
{
    uint32_t packetSize{1450}; // bytes
    uint32_t dataRate{75};    // mbps
    bool channelBonding = true; // combining channels
    std::string model = ("ns3::FriisPropagationLossModel");
    double distance{1}; // meters
    #ifdef PRELIMINARY
    double duration{0}; // seconds
    #else
    double duration{60};
    #endif
    double interPacketInterval{(((double)packetSize + UDP_IP_HEADER_SIZE_BYTES) * 8) / (double)(dataRate * 1e6)};
    uint32_t numPackets{(uint32_t)(duration / interPacketInterval)};
    bool verbose{false};

    CommandLine cmd(__FILE__);
    cmd.AddValue("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue("dataRate", "data rate of the udp traffic", dataRate);
    cmd.AddValue("channelBonding", "use channel bonding", channelBonding);
    cmd.AddValue("model", "propagation model to use", model);
    cmd.AddValue("distance", "distance between nodes in meters", distance);
    cmd.AddValue("duration", "simulation time in seconds", duration);
    cmd.AddValue("interval", "interval between packets", interPacketInterval);
    cmd.AddValue("numPackets", "number of packets generated", numPackets);
    cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
    cmd.Parse(argc, argv);

    // Propagation models to evaluate
    std::vector<std::string> propagationModels = {
        FRIIS,
        FIXED_RSS,
        THREE_LOG_DISTANCE,
        TWO_RAY_GROUND,
        NAKAGAMI
    };

    // CSV table
    outputFile << "propagationModel,distance,duration,signalStrength,throughput" << std::endl;

    // Simulation experiments
    #ifndef PRELIMINARY
    double distance_{distance};
    for (const auto &model : propagationModels) {
        distance = distance_;   // reset
    #endif
        double throughput = 0.0;
        do {
            NodeContainer c;
            c.Create(2);

            // The below set of helpers will help us to put together the wifi NICs we want
            WifiHelper wifi;
            if (verbose)
            {
                WifiHelper::EnableLogComponents(); // Turn on all Wifi logging
            }
            wifi.SetStandard(WIFI_STANDARD_80211n);

            YansWifiPhyHelper wifiPhy;
            wifiPhy.Set("RxGain", DoubleValue(1));
            wifiPhy.Set("TxGain", DoubleValue(1));
            wifiPhy.Set("ChannelSettings", StringValue(std::string("{0, ") + (channelBonding ? "40, " : "20, ") + "BAND_5GHZ" + ", 0}"));
            wifiPhy.Set("TxPowerStart", DoubleValue(10));
            wifiPhy.Set("TxPowerEnd", DoubleValue(10));

            YansWifiChannelHelper wifiChannel;
            wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
            if (model == FIXED_RSS) wifiChannel.AddPropagationLoss(model, "Rss", DoubleValue(-80));
            else if (model == TWO_RAY_GROUND) wifiChannel.AddPropagationLoss(model, "Frequency", DoubleValue(5e9), "HeightAboveZ", DoubleValue(1));
            else wifiChannel.AddPropagationLoss(model);
            wifiPhy.SetChannel(wifiChannel.Create());

            // Add a mac
            WifiMacHelper wifiMac;

            // Set it to adhoc mode
            wifiMac.SetType("ns3::AdhocWifiMac");
            NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

            MobilityHelper mobility;
            Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
            positionAlloc->Add(Vector(0.0, 0.0, 0.0));
            positionAlloc->Add(Vector(distance, 0.0, 0.0));
            mobility.SetPositionAllocator(positionAlloc);
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(c);

            InternetStackHelper internet;
            internet.Install(c);

            Ipv4AddressHelper ipv4;
            NS_LOG_INFO("Assign IP Addresses.");
            ipv4.SetBase("10.1.1.0", "255.255.255.0");
            Ipv4InterfaceContainer i = ipv4.Assign(devices);

            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            Ptr<Socket> recvSink = Socket::CreateSocket(c.Get(0), tid);
            InetSocketAddress local = InetSocketAddress(i.GetAddress(0), 80);
            recvSink->Bind(local);

            Ptr<Socket> source = Socket::CreateSocket(c.Get(1), tid);
            InetSocketAddress remote = InetSocketAddress(i.GetAddress(0), 80);  // Use unicast address because broadcast doesn't work for Flow Monitor
            source->SetAllowBroadcast(true);
            source->Connect(remote);

            // Tracing
            wifiPhy.EnablePcap("wifi-simple-adhoc", devices);

            // Output what we are doing
            NS_LOG_UNCOND("" << model << ", " << distance << "m, " << duration << "s");

            #ifdef PRELIMINARY
            // Recalculate with new duration
            numPackets = (uint32_t)(duration / interPacketInterval);
            #endif

            Simulator::ScheduleWithContext(source->GetNode()->GetId(),
                                        Seconds(1.0),
                                        &GenerateTraffic,
                                        source,
                                        packetSize,
                                        numPackets,
                                        Time{Seconds(interPacketInterval)});

            // Flow monitor
            Ptr<FlowMonitor> flowMonitor;
            FlowMonitorHelper flowHelper;
            flowMonitor = flowHelper.InstallAll();

            // Tracing the signal strength
            Ptr<NetDevice> dev = devices.Get(0);
            Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(dev);
            Ptr<WifiPhy> phy = wifiDev->GetPhy();
            phy->TraceConnectWithoutContext("MonitorSnifferRx", MakeCallback(&SnifferRx));

            Simulator::Stop(Seconds(duration));
            Simulator::Run();
            Simulator::Destroy();

            // Calculating the throughput
            FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();
            if (!stats.empty()) {
                auto flow = stats.rbegin();
                throughput = flow->second.rxBytes * 8.0 / duration / 1e6; // Mbps
            }

            LogResults(model, distance, duration, signalStrength, throughput);

            #ifdef PRELIMINARY
            duration += DURATION_STEP;
            #else
            distance += DISTANCE_STEP;
            #endif

        }
        #ifdef PRELIMINARY
        while (duration <= DURATION_MAX);
        #else
        while (throughput > 0 && distance <= DISTANCE_MAX);
        #endif
    #ifndef PRELIMINARY
    }
    #endif

    outputFile.close();

    return 0;
}
