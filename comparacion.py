import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import numpy as np

def parse_flow_monitor(file_name):
    tree = ET.parse(file_name)
    root = tree.getroot()
    stats = []

    for flow in root.findall(".//FlowStats/Flow"):
        flow_id = flow.get('flowId')
        delay_sum = float(flow.get('delaySum').rstrip('ns')) / 1e9  # Convertir ns a segundos
        jitter_sum = float(flow.get('jitterSum').rstrip('ns')) / 1e9  # Convertir ns a segundos
        tx_bytes = int(flow.get('txBytes'))
        rx_bytes = int(flow.get('rxBytes'))
        tx_packets = int(flow.get('txPackets'))
        rx_packets = int(flow.get('rxPackets'))
        lost_packets = int(flow.get('lostPackets'))
        stats.append({
            'flow_id': flow_id,
            'delay_sum': delay_sum,
            'jitter_sum': jitter_sum,
            'tx_bytes': tx_bytes,
            'rx_bytes': rx_bytes,
            'tx_packets': tx_packets,
            'rx_packets': rx_packets,
            'lost_packets': lost_packets
        })

    print(f"Parsed {len(stats)} flows from {file_name}")
    return stats

def plot_metrics(stats1, stats2, title):
    flow_ids = np.arange(len(stats1))  # Usar np.arange para el índice de flujo
    delay_sums1 = [stat['delay_sum'] for stat in stats1]
    delay_sums2 = [stat['delay_sum'] for stat in stats2]
    jitter_sums1 = [stat['jitter_sum'] for stat in stats1]
    jitter_sums2 = [stat['jitter_sum'] for stat in stats2]
    tx_bytes1 = [stat['tx_bytes'] for stat in stats1]
    tx_bytes2 = [stat['tx_bytes'] for stat in stats2]

    print("Delay sums for Hubs:", delay_sums1)
    print("Delay sums for Switches:", delay_sums2)
    print("Jitter sums for Hubs:", jitter_sums1)
    print("Jitter sums for Switches:", jitter_sums2)
    print("TX bytes for Hubs:", tx_bytes1)
    print("TX bytes for Switches:", tx_bytes2)

    width = 0.35  # Ancho de las barras

    fig, axs = plt.subplots(3, figsize=(12, 12))

    # Delay Sum
    axs[0].bar(flow_ids - width/2, delay_sums1, width, label='Hubs', color='blue')
    axs[0].bar(flow_ids + width/2, delay_sums2, width, label='Switches', color='orange')
    axs[0].set_title('Delay Sum')
    axs[0].set_ylabel('Seconds')
    axs[0].legend()

    # Jitter Sum
    axs[1].bar(flow_ids - width/2, jitter_sums1, width, label='Hubs', color='blue')
    axs[1].bar(flow_ids + width/2, jitter_sums2, width, label='Switches', color='orange')
    axs[1].set_title('Jitter Sum')
    axs[1].set_ylabel('Seconds')
    axs[1].legend()

    # Tx Bytes
    axs[2].bar(flow_ids - width/2, tx_bytes1, width, label='Hubs', color='blue')
    axs[2].bar(flow_ids + width/2, tx_bytes2, width, label='Switches', color='orange')
    axs[2].set_title('Tx Bytes')
    axs[2].set_ylabel('Bytes')
    axs[2].legend()

    plt.tight_layout()
    plt.suptitle(title, y=1.05)
    plt.savefig('Punto2Grafica.png')
    plt.close()

# Parsear los XMLs
stats_hubs = parse_flow_monitor('hubs.xml')
stats_switches = parse_flow_monitor('switches.xml')

# Generar la gráfica comparativa y guardarla en una imagen
plot_metrics(stats_hubs, stats_switches, 'Comparación de Redes con Hubs y Switches')

