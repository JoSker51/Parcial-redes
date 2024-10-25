#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

void RunSimulation(std::string tcpType, std::string outputFileName) {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcpType));
    std::cout << "Configurando simulación con " << tcpType << std::endl;

    // Crear nodos
    NodeContainer nodes;
    nodes.Create(2);

    // Instalar pila de Internet
    InternetStackHelper stack;
    stack.Install(nodes);

    // Crear canales de punto a punto con variaciones
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Asignar direcciones IP
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Crear aplicaciones
    uint16_t port = 9;
    UdpEchoServerHelper server(port);
    ApplicationContainer apps = server.Install(nodes.Get(1));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(20.0));

    UdpEchoClientHelper client(interfaces.GetAddress(1), port);
    client.SetAttribute("MaxPackets", UintegerValue(100));  // Más paquetes
    client.SetAttribute("Interval", TimeValue(Seconds(0.1)));  // Intervalo menor
    client.SetAttribute("PacketSize", UintegerValue(1024));
    apps = client.Install(nodes.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(19.0));

    // Crear tráfico de fondo para generar variaciones
    OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
    onoff.SetConstantRate(DataRate("1Mbps"));
    onoff.SetAttribute("StartTime", TimeValue(Seconds(3.0)));
    onoff.SetAttribute("StopTime", TimeValue(Seconds(18.0)));
    apps = onoff.Install(nodes.Get(0));

    // Habilitar el FlowMonitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // Iniciar simulación
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();

    // Guardar resultados en archivo .xml
    monitor->SerializeToXmlFile(outputFileName, true, true);
    Simulator::Destroy();
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Ejecutar simulación con TcpNewReno
    RunSimulation("ns3::TcpNewReno", "tcp-newreno.xml");

    // Ejecutar simulación con TcpHighSpeed
    RunSimulation("ns3::TcpHighSpeed", "tcp-highspeed.xml");

    return 0;
}

