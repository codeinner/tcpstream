# TcpStream

**TcpStream** is a library for assembling user-provided Ethernet packets into TCP streams.  
The library provides high-level API to catch following events:

* TCP stream created
* Data transfer inside specified TCP stream 
* Stream closed

## Introduction

| Directory                  | Description                               |
|----------------------------|-------------------------------------------|
| `src/`                     | Source code of the library                |
| `include/`                 | Includes to use library in your code      |
| `tests/`                   | Simple auto-tests implementation          |
| `example/`                 | Examples of using TcpStream library       |
| `example/tcp_stream_dump/` | Sample application to view TCP streams<br> flowing through specified network interface.<br>PCAP library used to catch ethernet packets |

## Dependencies

**TcpStream** library written in plain C and has no dependencies.

## Usage

1. Include `packet_processing.h`.
2. Create new `Packet Processor` instance by invoking `PacketProcessorInitialize`.  
You need to pass `TcpStreamCallbacks` structure filled with event handlers pointers to catch library's events:
    * `OnNew` handler will be called on every new TCP stream. If you are interested in specific stream return `true` or return `false` to skip that stream.
    * `OnData` will be invoked on every data transfer inside TCP stream. To stop stream processing return `false` or `true` otherwise.
    * `OnClose` signals that TCP stream ended.
3. Start feeding `Packet Processor` with Ethernet frames through `ProcessEthernetFrame`
4. When you are done call `PacketProcessorDestroy` to free resources allocated by `Packet Processor` instance.

Usage sample pseudocode:

```c
  TcpStreamCallbacks callbacks;
  void* packetProcessor;
  const unsigned char* packet;
  size_t size;
  long timestamp;

  callbacks.OnNew = HandlerOnNew;
  callbacks.OnData = HandlerOnData;
  callbacks.OnClose = HandlerOnClose;

  packetProcessor = PacketProcessorInitialize(&callbacks);
  if (packetProcessor == NULL)
  {
    return ERROR_PROCESSOR_INITIALIZE;
  }

  while (<not all packets processed>)
  {
    packet = <get next packet>
    timestamp = <packet timestamp>
    size = <packet size>

    ProcessEthernetFrame(packetProcessor, timestamp, packet, size);
  }

  PacketProcessorDestroy(packetProcessor);

  return 0;
```

## Build

You can create `Release` build simply by invoking
* `make.bat` on Windows hosts
* `make.sh` on Unix-based systems

Note that scripts above will automatically run tests after build.

To make a debug build use `cmake` to build library.  
To manually run tests call `ctest` in cmake output directory.

## Example

The library provided with sample application `tcp_stream_dump`. It uses `PCAP` library to capture ethernet packets from network adapter.
So you need to install:
* On Unix-based OS: `libpcap-dev libpcap0.8 libpcap0.8-dev` packets
* On Windows: [WinPcap](https://www.winpcap.org/install/default.htm)

## License

Source code published under MIT license.