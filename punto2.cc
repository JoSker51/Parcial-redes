#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SwitchesExample");

void GenerateTraffic(NodeContainer endDevices, Ipv4InterfaceContainer interfaces, uint16_t port) {
    // Generar tráfico principal
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress(1), port)));
    onoff.SetConstantRate(DataRate("100Mbps")); // Aumentar significativamente la tasa de generación de tráfico
    onoff.SetAttribute("StartTime", TimeValue(Seconds(1.0)));
    onoff.SetAttribute("StopTime", TimeValue(Seconds(10.0)));
    ApplicationContainer apps = onoff.Install(endDevices.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Tráfico de fondo para generar congestión
    OnOffHelper backgroundTraffic("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress(2), port)));
    backgroundTraffic.SetConstantRate(DataRate("50Mbps")); // Aumentar la tasa de tráfico de fondo
    backgroundTraffic.SetAttribute("StartTime", TimeValue(Seconds(2.0)));
    backgroundTraffic.SetAttribute("StopTime", TimeValue(Seconds(9.0)));
    apps = backgroundTraffic.Install(endDevices.Get(1));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(9.0));
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    NodeContainer switches;
    switches.Create(3); // Crear tres switches

    NodeContainer endDevices;
    endDevices.Create(6); // Crear seis dispositivos finales, dos por switch

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("1Gbps")); // Aumentar la tasa de datos
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(5000))); // Reducir el retardo

    // Asignar direcciones IP usando subredes diferentes
    InternetStackHelper stack;
    stack.Install(switches);
    stack.Install(endDevices);

    Ipv4AddressHelper address;
    NetDeviceContainer switchDevices;
    Ipv4InterfaceContainer interfaces;

    // Configurar las subredes para cada enlace
    for (uint32_t i = 0; i < switches.GetN(); ++i) {
        for (uint32_t j = 0; j < endDevices.GetN() / switches.GetN(); ++j) {
            NodeContainer pair = NodeContainer(switches.Get(i), endDevices.Get(i * 2 + j));
            NetDeviceContainer link = csma.Install(pair);
            switchDevices.Add(link);
            std::string base = "10.1." + std::to_string(i * 2 + j + 1) + ".0";
            address.SetBase(base.c_str(), "255.255.255.0");
            interfaces.Add(address.Assign(link));
        }
    }

    // Crear tráfico entre dispositivos finales
    uint16_t port = 9;
    GenerateTraffic(endDevices, interfaces, port);

    // Habilitar FlowMonitor para medir métricas
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();

    // Guardar resultados en archivo .xml
    monitor->SerializeToXmlFile("switches.xml", true, true);

    Simulator::Destroy();
    return 0;
}

