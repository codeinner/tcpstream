#include "errors.h"
#include "log.h"
#include "sniffer.h"

#include <packet_processing.h>
#include <vector.h>

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

size_t streamId = 0;

typedef struct _Stream
{
  size_t Id;
  size_t TotalSent;
  size_t TotalReceived;
} Stream;

bool OnNew(ip sourceIp, port sourcePort, ip destinationIp, port destinationPort, void** userData)
{
  Stream* stream = malloc(sizeof(Stream));
  if (stream == NULL)
  {
    return false;
  }

  stream->Id = streamId++;
  stream->TotalSent = 0;
  stream->TotalReceived = 0;

  LOG("Stream #%d started", stream->Id);

  *userData = (void*)stream;
  return true;
}

bool OnData(void* userData, bool outgoing, const void* data, size_t size)
{
  Stream* stream = (Stream*)userData;

  LOG("Stream #%d client %s %d bytes", stream->Id, outgoing ? "sent" : "received", size);

  if (outgoing)
  {
    stream->TotalSent += size;
  }
  else
  {
    stream->TotalReceived += size;
  }

  return true;
}

void OnClose(void* userData)
{
  Stream* stream = (Stream*)userData;

  LOG("Stream #%d closed. Total %d bytes sent and %d bytes received", stream->Id, stream->TotalSent, stream->TotalReceived);

  free(stream);
}

int CaptureDeviceTraffic(pcap_t* session, volatile bool* stopFlag)
{
  TcpStreamCallbacks callbacks;
  void* packetProcessor;
  const unsigned char* packet;
  size_t size;
  long timestamp;

  callbacks.OnNew = OnNew;
  callbacks.OnData = OnData;
  callbacks.OnClose = OnClose;

  packetProcessor = PacketProcessorInitialize(&callbacks);
  if (packetProcessor == NULL)
  {
    return ERROR_PROCESSOR_INITIALIZE;
  }

  while (*stopFlag == false)
  {
    if (WaitNextEthernetPacket(session, &timestamp, &packet, &size))
    {
      ProcessEthernetFrame(packetProcessor, timestamp, packet, size);
    }
  }

  PacketProcessorDestroy(packetProcessor);

  return 0;
}

int RunTcpSniffer(int deviceIndex, volatile bool* stopFlag)
{
  const int timeoutMs = 1000;
  pcap_if_t* devices;
  int result;

  devices = GetDevices();
  if (devices == NULL)
  {
    return ERROR_NO_DEVICES;
  }

  pcap_if_t* device = FindDeviceByIndex(devices, deviceIndex);
  if (device != NULL)
  {
    pcap_t* session = OpenTcpIpDevice(device, timeoutMs);
    if (session != NULL)
    {
      result = CaptureDeviceTraffic(session, stopFlag);
      CloseTcpIpDevice(session);
    }
  }
  else
  {
    result = ERROR_DEVICE_NOT_FOUND;
  }

  FreeDevices(devices);

  return result;
}

int PrintUsage(int argc, char* argv[])
{
  pcap_if_t* devices;

  devices = GetDevices();
  if (devices == NULL)
  {
    return ERROR_NO_DEVICES;
  }

  LOG("Network devices list:");
  for (unsigned int index = 0; devices != NULL; ++index, devices = devices->next)
  {
    LOG("%d: %s (%s)", index, devices->name, devices->description);
  }

  LOG("\n"
      "Please select network device from the list above to start traffic monitoring\n"
      "\n"
      "%s <device_number>", argv[0]);

  FreeDevices(devices);

  return 0;
}

volatile bool stopFlag;

void SignalIntHandler(int signal)
{
  if (signal == SIGINT)
  {
    stopFlag = true;
  }
}

int main(int argc, char* argv[])
{
  int deviceIndex;

  if (argc != 2)
  {
    return PrintUsage(argc, argv);
  }

  stopFlag = false;
  signal(SIGINT, SignalIntHandler);

  sscanf(argv[1], "%d", &deviceIndex);

  return RunTcpSniffer(deviceIndex, &stopFlag);
}
