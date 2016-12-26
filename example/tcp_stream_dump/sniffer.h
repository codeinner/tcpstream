#ifndef SNIFFER_H
#define SNIFFER_H

#include <pcap.h>
#include <stdbool.h>

pcap_if_t* GetDevices();
void FreeDevices(pcap_if_t* devices);
pcap_if_t* FindDeviceByIndex(pcap_if_t* devices, int deviceIndex);
pcap_t* OpenTcpIpDevice(pcap_if_t* device, int timeoutMs);
void CloseTcpIpDevice(pcap_t* session);
bool WaitNextEthernetPacket(pcap_t* session, long* timestamp, const unsigned char** buffer, size_t* size);

#endif