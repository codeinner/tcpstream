#include "packet_processor.h"

#include <stdlib.h>
#include <string.h>

bool ConnectionData(PacketProcessor* instance, Connection* connection, bool client, const void* data, unsigned short size)
{
  Vector* dataStorage = client ? connection->Outgoing : connection->Incoming;
  return VectorAppend(dataStorage, data, size);
}

bool ConnectionFlushData(Connection* connection, bool outgoing)
{
  Vector* bufferedData = !outgoing ? connection->Outgoing : connection->Incoming;
  size_t bufferedDataSize = VectorGetSize(bufferedData);
  bool proceedStreamMonoitoring;

  if (bufferedDataSize == 0)
  {
    return true;
  }

  proceedStreamMonoitoring = connection->Callbacks->OnData(connection->UserData, !outgoing, VectorGetBuffer(bufferedData), bufferedDataSize);
  if (!proceedStreamMonoitoring)
  {
    return false;
  }

  if (!outgoing)
  {
    connection->SourceSequence += bufferedDataSize;
  }
  else
  {
    connection->DestinationSequence += bufferedDataSize;
  }

  VectorClear(bufferedData);

  return true;
}

Connection* AddConnection(PacketProcessor* instance, ip source, ip destination, const TCP_HEADER* tcp)
{
  Connection* connection = (Connection*)(malloc(sizeof(Connection)));
  if (connection == NULL)
  {
    return NULL;
  }

  connection->Next = instance->Connections;
  connection->SourceSequence = HTONL(tcp->Sequence);
  connection->DestinationSequence = 0;
  connection->State = TcpSynSent;
  connection->StateChanged = time(NULL);
  connection->SourceIp = source;
  connection->SourcePort = tcp->SourcePort;
  connection->DestinationIp = destination;
  connection->DestinationPort = tcp->DestinationPort;
  connection->Outgoing = VectorInitialize(1024);
  connection->OutgoingAcknowledgedSize = 0;
  connection->Incoming = VectorInitialize(1024);
  connection->IncomingAcknowledgedSize = 0;
  connection->Callbacks = &instance->Callbacks;

  if (connection->Outgoing != NULL && connection->Incoming != NULL)
  {
    instance->Connections = connection;
    return connection;
  }

  if (connection->Outgoing != NULL)
  {
    VectorUninitialize(connection->Outgoing);
  }
  if (connection->Incoming != NULL)
  {
    VectorUninitialize(connection->Incoming);
  }

  free(connection);
  return NULL;
}

Connection* FindConnection(PacketProcessor* instance, ip source, ip destination, const TCP_HEADER* tcp, bool* client)
{
  for (Connection* current = instance->Connections; current != NULL; current = current->Next)
  {
    if (current->DestinationIp == destination &&
        current->DestinationPort == tcp->DestinationPort &&
        current->SourceIp == source &&
        current->SourcePort == tcp->SourcePort)
    {
      if (client != NULL)
      {
        *client = true;
      }

      return current;
    }
    if (current->DestinationIp == source &&
        current->DestinationPort == tcp->SourcePort&&
        current->SourceIp == destination &&
        current->SourcePort == tcp->DestinationPort)
    {
      if (client != NULL)
      {
        *client = false;
      }

      return current;
    }
  }

  return NULL;
}

void RemoveConnection(PacketProcessor* instance, Connection* connection)
{
  Connection* current;
  void** listReference = (void**)&instance->Connections;

  for (current = instance->Connections; current != NULL; current = current->Next)
  {
    if (current->Next == connection)
    {
      listReference = &current->Next;
      return;
    }
  }
  *listReference = connection->Next;

  if (connection->State == TcpEstablished)
  {
    if (ConnectionFlushData(connection, true))
    {
      ConnectionFlushData(connection, false);
    }
    instance->Callbacks.OnClose(connection->UserData);
  }

  VectorUninitialize(connection->Outgoing);
  VectorUninitialize(connection->Incoming);

  free(connection);
}

bool ProcessTcpHandshakeInit(Connection* connection, bool outgoing, ip source, ip destination, const TCP_HEADER* tcp)
{
  unsigned int nextSequence;

  if (outgoing)
  {
    return false;
  }

  if (!tcp->SYN)
  {
    return false;
  }

  nextSequence = HTONL(tcp->Acknowledgment);
  if (connection->SourceSequence + 1 != nextSequence)
  {
    return false;
  }

  connection->SourceSequence = nextSequence;
  connection->DestinationSequence = HTONL(tcp->Sequence);

  return true;
}

bool ProcessTcpHandshakeFinal(Connection* connection, ip source, ip destination, const TCP_HEADER* tcp, bool outgoing)
{
  unsigned int nextSequence;
  bool shouldMonitor;

  if (!outgoing)
  {
    return false;
  }

  nextSequence = HTONL(tcp->Acknowledgment);
  if (connection->DestinationSequence + 1 != nextSequence)
  {
    return false;
  }

  shouldMonitor = connection->Callbacks->OnNew(source, HTONS(tcp->SourcePort), destination, HTONS(tcp->DestinationPort), &connection->UserData);
  if (!shouldMonitor)
  {
    return false;
  }

  connection->DestinationSequence = nextSequence;

  return true;
}

bool ProcessTcpCommunication(Connection* connection, const TCP_HEADER* tcp, bool outgoing, const unsigned char* payload, unsigned short payloadSize)
{
  unsigned int expectedSequence;
  unsigned int expectedAcknowledgment;
  Vector* bufferedData;

  if (outgoing)
  {
    expectedSequence = connection->SourceSequence + VectorGetSize(connection->Outgoing);
  }
  else
  {
    expectedSequence = connection->DestinationSequence + VectorGetSize(connection->Incoming);
  }

  if (expectedSequence == HTONL(tcp->Sequence))
  {
    bufferedData = outgoing ? connection->Outgoing : connection->Incoming;
    if (!VectorAppend(bufferedData, payload, payloadSize))
    {
      return false;
    }
  }

  if (outgoing)
  {
    expectedAcknowledgment = connection->DestinationSequence + VectorGetSize(connection->Incoming);
  }
  else
  {
    expectedAcknowledgment = connection->SourceSequence + VectorGetSize(connection->Outgoing);
  }

  if (expectedAcknowledgment == HTONL(tcp->Acknowledgment))
  {
    return ConnectionFlushData(connection, outgoing);
  }

  return true;
}

bool ProcessTcpFrame(PacketProcessor* instance, long timestamp, ip source, ip destination, const unsigned char* packet, unsigned short size)
{
  const TCP_HEADER* tcp = (const TCP_HEADER*)packet;
  unsigned short tcpHeaderSize;
  unsigned short tcpPayloadSize;
  const unsigned char* tcpPayload;
  Connection* connection;
  bool success;
  bool outgoing;

  if (sizeof(TCP_HEADER) > size)
  {
    return false;
  }

  tcpHeaderSize = tcp->DataOffset * 4;
  if (tcpHeaderSize > size)
  {
    return false;
  }

  if (tcp->SYN && !tcp->ACK)
  {
    connection = AddConnection(instance, source, destination, tcp);
    return connection != NULL;
  }

  connection = FindConnection(instance, source, destination, tcp, &outgoing);
  if (connection == NULL)
  {
    return true;
  }

  if (!tcp->ACK)
  {
    RemoveConnection(instance, connection);
    return false;
  }

  success = false;
  switch (connection->State)
  {
    case TcpSynSent:
    {
      success = ProcessTcpHandshakeInit(connection, outgoing, source, destination, tcp);
      if (success)
      {
        connection->State = TcpSynAckRecieved;
      }
      break;
    }
    case TcpSynAckRecieved:
    {
      success = ProcessTcpHandshakeFinal(connection, source, destination, tcp, outgoing);
      if (success)
      {
        connection->State = TcpEstablished;
      }
      break;
    }
    case TcpEstablished:
    {
      tcpPayloadSize = size - tcpHeaderSize;
      tcpPayload = (const unsigned char*)tcp + tcpHeaderSize;
      success = ProcessTcpCommunication(connection, tcp, outgoing, tcpPayload, tcpPayloadSize);
      break;
    }
    default:
      break;
  }

  if (!success || tcp->FIN || tcp->RST)
  {
    RemoveConnection(instance, connection);
  }

  return success;
}

bool ProcessIpFrame(long timestamp, const unsigned char* packet, size_t size, IpFrameInfo* info)
{
  const IP_HEADER* ip = (const IP_HEADER*)(packet);
  unsigned char ipHeaderSize;
  unsigned short ipPayloadSize;

  if (sizeof(IP_HEADER) > size)
  {
    return false;
  }

  ipHeaderSize = (ip->VersionLength & 0xF) * 4;
  ipPayloadSize = HTONS(ip->TotalLength);
  if (ipHeaderSize > size)
  {
    return false;
  }

  if (ipPayloadSize > size)
  {
    return false;
  }

  if (ipHeaderSize > ipPayloadSize)
  {
    return false;
  }

  if (ip->Protocol != PROTOCOL_TCP)
  {
    return true;
  }

  info->Source = ip->SourceIp;
  info->Destination = ip->DestinationIp;
  info->TcpFrame = packet + ipHeaderSize;
  info->TcpFrameSize = ipPayloadSize - ipHeaderSize;

  return true;
}

void* PacketProcessorInitialize(const TcpStreamCallbacks* callbacks)
{
  PacketProcessor* instance = (PacketProcessor*)malloc(sizeof(PacketProcessor));

  if (instance == NULL)
  {
    return NULL;
  }

  memcpy(&instance->Callbacks, callbacks, sizeof(TcpStreamCallbacks));
  instance->Connections = NULL;

  return instance;
}

void PacketProcessorDestroy(void* packetProcessor)
{
  PacketProcessor* instance = (PacketProcessor*)packetProcessor;
  Connection* connection = instance->Connections;
  Connection* next;

  while (connection != NULL)
  {
    next = connection->Next;
    RemoveConnection(instance, connection);
    connection = next;
  }

  free(packetProcessor);
}

bool ProcessEthernetFrame(void* packetProcessor, long timestamp, const unsigned char* packet, size_t size)
{
  const MAC_HEADER* mac = (MAC_HEADER*)packet;
  PacketProcessor* instance;
  IpFrameInfo ipInfo;

  if (sizeof(MAC_HEADER) > size)
  {
    return false;
  }

  if (mac->EtherType != HTONS(ETHERNET_TYPE_IPv4))
  {
    return false;
  }

  instance = (PacketProcessor*)packetProcessor;

  if (!ProcessIpFrame(timestamp, packet + sizeof(MAC_HEADER), size - sizeof(MAC_HEADER), &ipInfo))
  {
    return false;
  }

  return ProcessTcpFrame(instance, timestamp, ipInfo.Source, ipInfo.Destination, ipInfo.TcpFrame, ipInfo.TcpFrameSize);
}