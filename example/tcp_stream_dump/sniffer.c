#include "sniffer.h"

#include "log.h"

pcap_if_t* GetDevices()
{
  char error[PCAP_ERRBUF_SIZE];
  pcap_if_t* devices;

  if (pcap_findalldevs(&devices, error) == -1)
  {
    LOG_ERROR("pcap_findalldevs failed: %s", error);
    return NULL;
  }

  if (devices == NULL)
  {
    LOG_ERROR("No devices found to sniff");
  }

  return devices;
}

void FreeDevices(pcap_if_t* devices)
{
  pcap_freealldevs(devices);
}

pcap_if_t* FindDeviceByIndex(pcap_if_t* devices, int deviceIndex)
{
  for (unsigned int index = 0; devices != NULL; ++index, devices = devices->next)
  {
    if (index == deviceIndex)
    {
      return devices;
    }
  }

  return NULL;
}

pcap_t* OpenTcpIpDevice(pcap_if_t* device, int timeoutMs)
{
  char error[PCAP_ERRBUF_SIZE];
  pcap_t* session;
  const unsigned int netmask = 0x00000000;
  char packet_filter[] = "ip and tcp";
  struct bpf_program fcode;

  session = pcap_open_live(device->name, 65536, 1, timeoutMs, error);
  if (session == NULL)
  {
    LOG_ERROR("Failed to open device: %s", error);
    return NULL;
  }

  if (pcap_datalink(session) != DLT_EN10MB ||
      pcap_compile(session, &fcode, packet_filter, 1, netmask) < 0 ||
      pcap_setfilter(session, &fcode) < 0)
  {
    LOG("Error setting the filter");
    pcap_close(session);
    return NULL;
  }

  return session;
}

void CloseTcpIpDevice(pcap_t* session)
{
  pcap_close(session);
}

bool WaitNextEthernetPacket(pcap_t* session, long* timestamp, const unsigned char** buffer, size_t* size)
{
  struct pcap_pkthdr* packet;
  if (pcap_next_ex(session, &packet, buffer) != 1)
  {
    return false;
  }

  *timestamp = packet->ts.tv_sec;
  *size = packet->len;
  return true;
}