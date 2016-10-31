# [DRAFT] Description of the gateway client protocol
This document describes the protocol to talk to an s-net gateway entity. In this document and in the
source code, such software is referred to as a "gateway client".


                                                                        +------------------+
                    +---+         +--------+                        +---| Gateway client 1 |
         /\ /\      |   |---------| HDLCd  O---+                   /    +------------------+
      +-+  +  +-----+  /          +--------+    \    +----------+ /
     /                +---+                      +---|          |/      +------------------+
    |   Wireless sensor   |                          | Gateway  O-------| Gateway client 2 |
    |       network    +--+                      +---|          |\      +------------------+
     \  ** BIG AREA ** |          +--------+    /    +----------+ \        ....
      +--+  +---+      |----------| HDLCd  O---+                   \    +------------------+
         /  \    \    /           +--------+                        +---| Gateway client N |
        +----+    +--+                                                  +------------------+
        
                       I----------I        I---------I          I-------I
                           HDLC             HDLCd                Gateway
                           via              access               client
                           serial           protocol             protocol
                                            
          I------------------------------------------------------------------I
                                Exchange of s-net packets


The gateway opens a port and allows gateway clients to connect to it. The amount of gateway clients
that are allowed to connect to a gateway is not explicitely limited, however, there are other schemes such as the
address space that may become a limitation. Each gateway client that connects to the same port of a gateway
entity receives the same stream of frames. If one gateway client connects later than another one, all frames
that were already delivered to the first gateway client are not repeated for the later one. Furthermore,
data sent by a gateway client to gateway are not "mirrored" to the other gateway clients. Thus, one gateway
client may send a request message to the sensor network and evaluate the corresponding response easily, while
the other gateway clients receive the response only and are not aware of the earlier request message.

## TCP-based communication
Communication among a gateway client and a gateway is based on a TCP socket that is always initiated by a
gateway client and accepted by a gateway. Such TCP connections may exist for a long time span and have
to be closed by the respective gateway client. A gateway does never close such connections with the exception
of a shutdown of the gateway or a detected violation of the TCP-based framing protocol.

### TCP payload
TCP itself offers **only** a reliable and in-sequence stream of bytes. The kinds of data that have
to be exchanged among gateways and gateway clients are *frames*, i.e., groups of bytes that have a specific format
and length and effectively contain s-net packets. Delivering frames via the byte-oriented scheme demands
for a framing scheme that allows the peer of a TCP connection do determine where an s-net packet starts
and how long it is.

For this purpose, the s-net gateway offers two modes of framing, which are described in the next section.

## Framing modes
The C++-based gateway software offers two framing modes:
- the legacy *escaping-based mode*, that involves a HDLC-like framing and escaping, and
- the preferred *length-based mode*, that preceeds each payload with a length field.

Both modes are explained in the following. Both use the same TCP port of the gateway.

### Deprecated: the escaping-based framing mode
The *escaping-based* mode is the legacy mode that existed before the C++-based gateway software
was created. It is still the only protocol offered by its predecessor, a Java-based gateway software. Before
having a deeper look into the details of this protocol, have in mind that it is **DEPRECATED** and that you
**SHOULD NOT** use it for any new gateway client! It has some severe design issues as depicted in the following.

The idea of how to assemble frames from a stream of bytes was borrowed from the "High-level Data-Link
Control" protocol (HDLC). HDLC is a layer-two protocol that is able to detect frames in a stream of
bits or bytes tailored to serial communications. However, as TCP already offers a reliable service,
this added a needless amount of complexity.

The framing scheme uses the *frame delimiter* `0x7E` of HDLC to mark begin and end of each frame. As the
specific value of the frame delimiter may also be part the payload, an escaping scheme has to be performed.
That escaping scheme introduces multiple values of bytes with special meanings that also have to be escaped
if they appear within the payload. As a consequence, the amount of bytes on the medium is increased,
and the receiver must have a look at each byte in order to check it for a specific meaning.

Furthermore, this flavor of the access protocol implements a command/response scheme. Each frame contains
a two-byte field indicating the frame type. There are frame types for session management, for data
transmission and to acknowledge received data frames.

This scheme has some serious design issues:
- Using HDLC-based framing above of TCP adds minimal benefit but lots of unnecessary complexity. The main
  feature of the escaping scheme of HDLC is the ability to synchronize on a given stream of bytes. As TCP
  operates in a connection-oriented way, such a synchronization is not necessary as the receiver is
  automatically synchronized.
- The escaping scheme requires both the sender and the receiver to have a look of each byte in order to
  escape and deescape it properly. This adds complexity to the handling of the byte stream.
- The worst point to mention is the introduced command/response scheme: for each data frame sent to the
  gateway the gateway must immediately reply with an acknowledgement frame. This acknowledgement frame
  must arrive at the gateway client within two seconds in order to avoid timeout issues. Thus, it is
  **not possible** that a gateway stops reading from a TCP connection in cases of a congestion, as that
  would break the existing legacy gateway clients.
- To summarize, the legacy protocol is **not capable of flow and congestion control**.

***WARNING*** The *escaping-based framing mode* is **DEPCRECATED**. Please, **DO NOT** consider it for
any development of subsequent gateway clients! Always use the *length-based framing mode* instead!


### Recommended: the length-based framing mode
The *length-based framing mode* is the successor of the *escaping-based framing mode* and is the recommended
mode of operation. In contrast to its predecessor, it does not reimplement services that are already offered
by the underlying transport scheme, i.e., by TCP. Thus, it does not add an inefficient escaping scheme.

The only service that this protocol adds to TCP is the *framing service*. The requirements were:
- It must be simple to implement,
- it must be efficient,
- it must be able to share the same listener socket with the  *escaping-based framing mode*.
- Last but not least, it must be capable of **flow and congestion control**.

For these purposes, both the escaping scheme of HDLC and the command/response mode were dropped. Instead,
each frame starts with a length field of a fixed size that denotes the amount of bytes required to complete
the frame ("the size of the payload"). For coexistence with the legacy mode, it must assured that the first
byte of such a frame **MUST NEVER** become the HDLC frame delimiter `0x7E`, which would result in wrong parsing.

    ++-----------------------------------------------++----------------------++---------------------------++
    || Frame identifier                              || Frame length, contd. || Frame payload (variable)  || 
    ++-------------+------------+--------------------++----------------------++---------------------------++
    || Bit 7       | Bits 6...4 | Bits 3...0         || Bits 7...0           || 0...4095 bytes            ||
    || Must be "1" | Reserved   | Upper length field || Lower length field   || Payload: one s-net packet ||
    ++-------------+------------+--------------------++----------------------++---------------------------++

This frame format has the following properties:
- The added overhead is two bytes per frame.
- As bit 7 of the first byte of a frame is always set, it is impossible that the first byte of a frame becomes `0x7E`.
  This allows coexistence with the *escaping-based framing mode*.
- The length field consists of 12 bits allowing payloads from 0 to 4095 bytes. Thus, the MTU is 4095. Sending empty frames
  is allowed but not recommended.
- The reserved bits **MUST BE** set to zero as long as no specific meaning was assigned to them. A protocol entity is allowed
  to consider a reserved bit that was set by the peer as a violation of the protocol.

Mode of operation:
- This frame format is the same for frames sent to and received from the gateway.
- On any error or protocol violation, the TCP socket is closed.
- It is not allowed to mix escaped frames and length-based frames on the same socket. This would confuse the peer entity
  at the gateway: the gateway observes the mode selected by each gateway client, remembers that mode, and uses it to encapsulate
  outgoing s-net packets. Thus, the first frame sent to the gateway defines the mode, and frames of a different format are
  considered as a violation of the protocol. No frames are sent to a gateway client before the framing mode was determined.
- A protocol entity may safely stop reading from the TCP socket in cases of congestions. TCP intrinsically takes care of
  propagating the congestion to the peer entity. There, trying to send subsequent data causes temporary blocks, which is
  the desired behavior.
