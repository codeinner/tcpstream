#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

#include "packet_processing.h"
#include "vector.h"

#include <time.h>

#define HTONS(s) ((unsigned short)(((s) >> 8) | ((s) << 8)))
#define HTONL(l) (((l) >> 24) | ((l) << 24) | (((l) & 0x00FF0000) >> 8) | (((l) & 0x0000FF00) << 8))

#define ETHERNET_TYPE_IPv4 0x0800
#define PROTOCOL_TCP 6

typedef struct _MAC_HEADER
{
  unsigned char DestinationMac[6];
  unsigned char SourceMac[6];
  unsigned short EtherType;
} MAC_HEADER;

typedef struct _IP_HEADER
{
  unsigned char VersionLength;
  unsigned char Tos;
  unsigned short TotalLength;
  unsigned short Id;
  unsigned short Flags;
  unsigned char Ttl;
  unsigned char Protocol;
  unsigned short Checksum;
  unsigned int SourceIp;
  unsigned int DestinationIp;
} IP_HEADER;

typedef struct _TCP_HEADER
{
  unsigned short SourcePort;
  unsigned short DestinationPort;
  unsigned int Sequence;
  unsigned int Acknowledgment;
  unsigned char NS : 1;
  unsigned char Reserved : 3;
  unsigned char DataOffset : 4;
  unsigned char FIN : 1;
  unsigned char SYN : 1;
  unsigned char RST : 1;
  unsigned char PSH : 1;
  unsigned char ACK : 1;
  unsigned char URG : 1;
  unsigned char ECE : 1;
  unsigned char CWR : 1;
} TCP_HEADER;

typedef enum _State
{
  TcpSynSent,
  TcpSynAckRecieved,
  TcpEstablished
} State;

typedef enum _ParseHttpsResult
{
  MoreData,
  Https,
  NotHttps
} ParseHttpsResult;

typedef enum _Role
{
  Client,
  Server
} Role;

typedef struct _Connection
{
  void* Next;
  State State;
  time_t StateChanged;
  unsigned int SourceSequence;
  unsigned int DestinationSequence;
  ip DestinationIp;
  port DestinationPort;
  ip SourceIp;
  port SourcePort;
  Vector* Outgoing;
  size_t OutgoingAcknowledgedSize;
  Vector* Incoming;
  size_t IncomingAcknowledgedSize;
  void* UserData;
  TcpStreamCallbacks* Callbacks;
} Connection;

typedef struct _PacketProcessor
{
  TcpStreamCallbacks Callbacks;
  Connection* Connections;
} PacketProcessor;

typedef struct _IpFrameInfo
{
  ip Source;
  ip Destination;
  const void* TcpFrame;
  unsigned short TcpFrameSize;
} IpFrameInfo;

#endif