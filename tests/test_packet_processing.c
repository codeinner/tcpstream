#include "test_packet_processing.h"

#include "telnet_cap.h"

#include <packet_processing.h>

#include <stdint.h>
#include <string.h>

typedef struct pcap_hdr_s
{
  uint32_t magic_number;   /* magic number */
  uint16_t version_major;  /* major version number */
  uint16_t version_minor;  /* minor version number */
  int32_t  thiszone;       /* GMT to local correction */
  uint32_t sigfigs;        /* accuracy of timestamps */
  uint32_t snaplen;        /* max length of captured packets, in octets */
  uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s
{
  uint32_t ts_sec;         /* timestamp seconds */
  uint32_t ts_usec;        /* timestamp microseconds */
  uint32_t incl_len;       /* number of octets of packet saved in file */
  uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

size_t OnNewCalled = 0;
size_t OnDataCalled = 0;
size_t OnCloseCalled = 0;
size_t TotalIncoming = 0;
size_t TotalOutgoing = 0;
bool DataMismatch = false;
bool OnNew(ip sourceIp, port sourcePort, ip destinationIp, port destinationPort, void** userData)
{
  ++OnNewCalled;
  return true;
}

bool OnData(void* userData, bool outgoing, const void* data, size_t size)
{
  ++OnDataCalled;
  if (outgoing)
  {
    if (TotalOutgoing + size > sizeof(TelnetOutgoing) || memcmp(data, TelnetOutgoing + TotalOutgoing, size) != 0)
    {
      DataMismatch = true;
    }
    TotalOutgoing += size;
  }
  else
  {
    if (TotalIncoming + size > sizeof(TelnetIncoming) || memcmp(data, TelnetIncoming + TotalIncoming, size) != 0)
    {
      DataMismatch = true;
    }
    TotalIncoming += size;
  }

  return true;
}

void OnClose(void* userData)
{
  ++OnCloseCalled;
}

bool test_packet_processing()
{
  const unsigned char* pcapData = TelnetCap;
  const unsigned char* pcapDataEnd = pcapData + sizeof(TelnetCap);
  const unsigned char* cursor;
  const pcaprec_hdr_t* packet;
  TcpStreamCallbacks callbacks = {OnNew, OnData, OnClose};
  void* instance;

  instance = PacketProcessorInitialize(&callbacks);
  test_assert_neq(instance, NULL);

  size_t packetNumber = 1;
  cursor = pcapData + sizeof(pcap_hdr_t);
  while (cursor + sizeof(pcaprec_hdr_t) < pcapDataEnd)
  {
    packet = (const pcaprec_hdr_t*)cursor;
    cursor += sizeof(pcaprec_hdr_t);

    ProcessEthernetFrame(instance, packet->ts_sec, cursor, packet->incl_len);
    test_assert_eq(DataMismatch, false);

    cursor += packet->incl_len;
    ++packetNumber;
  }

  test_assert_eq(TotalIncoming, sizeof(TelnetIncoming));
  test_assert_eq(TotalOutgoing, sizeof(TelnetOutgoing));

  test_assert_eq(OnNewCalled, 1);
  test_assert_eq(OnDataCalled, 44);
  test_assert_eq(OnCloseCalled, 1);

  PacketProcessorDestroy(instance);

  test_assert_eq(OnNewCalled, 1);
  test_assert_eq(OnDataCalled, 44);
  test_assert_eq(OnCloseCalled, 1);

  return true;
}