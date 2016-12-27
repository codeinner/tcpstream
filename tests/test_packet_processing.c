#include "test_packet_processing.h"

#include "telnet_cap.h"

#include <packet_processing.h>

#include <string.h>

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