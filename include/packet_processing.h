#ifndef PACKET_PROCESSING_H
#define PACKET_PROCESSING_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned int ip;
typedef unsigned short port;

/*
  To start handling of TCP streams invoke:

  1 PacketProcessorInitialize for new Packet Processor instance.
  2 ProcessEthernetFrame for each ethernet frame on a wire.
  3 PacketProcessorDestroy to clean-up and free Packet Processor instance.
*/

/*
  TcpStreamNewCallback invoked for each new TCP connection.

  void** userData - pointer to void* value optionally set by callback handler 
                    to identify the stream inside TcpStreamDataCallback and
                    TcpStreamCloseCallback handlers.

  Return:
    true:
      TCP stream will be monitored.
      
      TcpStreamDataCallback will be invoked on each data transfer occurend 
      on the stream.
      
      TcpStreamCloseCallback will be invoked on TCP connection close.
    
    false:
      TCP stream will be skipped.
      No callbacks will be invoked in future for the stream.
*/
typedef bool(*TcpStreamNewCallback)(ip sourceIp, port sourcePort, ip destinationIp, port destinationPort, void** userData);

/*
  TcpStreamDataCallback invoked for each data transfer on the monitored stream.

  Return:
    true:
      Proceed receving stream data updated.

    false:
      Unsubscribe from receiving stream data. 
      TcpStreamCloseCallback will be called immediately after that.
*/
typedef bool(*TcpStreamDataCallback)(void* userData, bool outgoing, const void* data, size_t size);

/*
  TcpStreamCloseCallback invoked on TCP stream close.
*/
typedef void(*TcpStreamCloseCallback)(void* userData);

typedef struct _TcpStreamCallbacks
{
  TcpStreamNewCallback OnNew;
  TcpStreamDataCallback OnData;
  TcpStreamCloseCallback OnClose;
} TcpStreamCallbacks;

/*
  Returns new Packet Processor instance or NULL on error.
*/
void* PacketProcessorInitialize(const TcpStreamCallbacks* callbacks);

/*
  Returns true if no errors occured or false otherwise.
*/
bool ProcessEthernetFrame(void* packetProcessor, long timestamp, const unsigned char* packet, size_t size);

/*
Returns true if no errors occured or false otherwise.
*/
void PacketProcessorDestroy(void* packetProcessor);

#endif