# [DRAFT] Description of the gateway access protocol
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
                           via              access               access
                           serial           protocol             protocol
                                            
          I------------------------------------------------I-----------------I
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
- the legacy *escaping-based framin mode*, that involves a HDLC-like framing and escaping, and
- the preferred *length-based framing mode*, that preceeds each payload with a length field.

Both framing modes are explained in the following. Both use the same TCP port of the gateway.

### Deprecated: the escaping-based framing mode
The *escaping-based framing mode* is the legacy mode that existed before the C++-based gateway software
was created. It is still the only framing mode offered by its predecessor, a Java-based gateway software. Before
having a deeper look into the details of this framing mode, have in mind that it is **DEPRECATED** and that you
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
- Using HDLC-based framing above of TCP adds no benefit but unnecessary complexity. The main
  feature of the escaping scheme of HDLC is the ability to synchronize on a given stream of bytes. As TCP
  operates in a connection-oriented way, such a synchronization is not necessary as the receiver is
  automatically synchronized.
- The escaping scheme requires both the sender and the receiver to have a look of each byte in order to
  escape and deescape it properly. This adds complexity to the handling of the byte stream.
- The worst point to mention is the introduced command/response scheme: for each command frame sent to the
  gateway the gateway must immediately reply with a response frame. This response frame
  must arrive at the gateway client within two seconds in order to avoid timeout issues. Thus, it is
  **not possible** that a gateway stops reading from a TCP connection in cases of a congestion, as that
  would break the existing legacy gateway clients.
- To summarize, the legacy *escaping-based framing mode* is **not capable of flow and congestion control**.

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

For these purposes, both the escaping scheme of HDLC and the command/response scheme were dropped. Instead,
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
- The added overhead is always two bytes per payload.
- As bit 7 of the first byte of a frame is always set, it is impossible that the first byte of a frame becomes `0x7E`.
  This allows coexistence with the *escaping-based framing mode*.
- The length field consists of 12 bits allowing payloads from 0 to 4095 bytes. Thus, the MTU is 4095. Sending empty frames
  is allowed but not recommended.
- The reserved bits **MUST BE** set to zero as long as no specific meaning was assigned to them. A protocol entity is allowed
  to consider a reserved bit that was set by the peer as a violation of the protocol.

Mode of operation:
- This frame format is the same for frames sent to and received from the gateway.
- On any error or protocol violation, the TCP socket is closed.
- It is not allowed to mix *escaping-based frames* and *length-based frames* on the same socket. This would confuse the peer entity
  at the gateway: the gateway observes the mode selected by each gateway client, remembers that mode, and uses it to encapsulate
  outgoing s-net packets. Thus, the first frame sent to the gateway defines the mode, and frames of a different format are
  considered as a violation of the protocol. No frames are sent to a gateway client before the framing mode was determined.
- A protocol entity may safely stop reading from the TCP socket in cases of congestions. TCP intrinsically takes care of
  propagating the congestion to the peer entity. There, trying to send subsequent data causes temporary blocks, which is
  the desired behavior.

## Services of the gateway software at the level of s-net service messages
Independent of the framing format chosen to exchange s-net packets among the gateway and the gateway clients, the
gateway performs routing of s-net packets and offers two services. These services are the *address assignment service* and
the *publish subscribe service*. Both services can be used be each of the gateway clients.

### The Address Assignment Service
The Java-based gateway software implemented an *address assignment service* that a gateway client had to talk to
in order to obtain a valid "s-net short address" (SSA). Here, the very first exchange of s-net packets was tailored
to obtaining a temporary address before any further message exchange was possible. Each gateway client possesses a uniquely
assigned  SSA that it has to use as the source address for all outgoing request messages sent towards the sensor network.
That unique unicast SSA is used by the addressed sensor node in order to create response messages that are addressed exactly
to the gateway client that created the respective request. As a consequence, gateway clients do only receive response
message related to request messages that they sent before; response messages sent to the unicast SSA of a different
gateway client are never delivered.

The C++-based gateway software implements a similar *address assignment service* that is compatible to all legacy gateway clients
that already exist. However, there are some minor differences in terms of handling:
- It is not explicitely required anymore that a gateway client talks to the *address assignment service* at the gateway.
- The address database at the gateway automatically assigns a unique SSA to each gateway client. This does not need any external
  trigger. The *address assignment service* can only be used to query that address, and multiple queries will always result in the
  same SSA.
- The source address of outgoing s-net packets is automatically changed to this unique SSA while passing the gateway (masquerading).
  This assures that response packets sent by sensor nodes will always be routed back to the originating gateway client even if the
  gateway client chose to use a wrong source SSA.
- Thus, a gateway client *may* use a "junk address" for the source SSA of outgoing packets without causing any trouble.
- The only benefit of using the *address assignment service* is to query the SSA that was assigned to a gateway client.
  This step is optional.
- ***WARNING*** If a new gateway client is created that should be able to talk to the legacy Java-based gateway, it **MUST** obtain
  an address as the very first step and then **MUST** use it correctly!

The messages of the *address assignment service* were reverse-engineered by evaluation of the messages exchanged between the
Java-based gateway software and all existing legacy gateway clients. The address assignment scheme involves a two-way-handshake
consisting of an *address assignment request* followed by an *address assignment reply*.

#### The Address Assignment Request
The *address assignment request* is sent by a gateway client to the gateway. An exemplary byte stream of such an *address assignment
request* is depicted in the following. It is safe to always use exactly these bytes. It has a UUID field set to zeros which is okay:

    00 00 3f ff 3f fc 00 af ae 00 10 25 00 00 00 00 00 00

Besides some other fields specific to s-net the following parameters are relevant:
- The source SSA must be set to `0x3FFF` (`UNASSIGNED_ADDR`)
- The destination SSA must be set to `0x3FFC` (`MULTICAST_ADDRESS_DATABASE`)
- The source service ID must be set to `0xAF`
- The destination service ID must be set to `0xAE`
- The application-layer token must be set to `0x10`
- The application-layer payload contains 7 bytes:
  - `0x25`: bit 5 set = from NON-WSN node, lower nibble = size of the UUID minus 1 -> 6 Bytes UUID. Use `0x25`!
  - Six bytes UUID of the gateway client, e.g., a random number or zeros

#### The Address Assignment Reply
An exemplary byte stream of an *address assignment reply* sent back by the gateway:

    00 00 40 00 3f f8 00 ae af 00 20 05 00 00 00 00 00 00 00 40 01

The relevant fields contain these values:
- The source SSA will always be `0x4000` (`MULTICAST_GATEWAY`)
- The destination SSA will always be `0x3FF8` (`MULTICAST_DIRECT_CHILDREN`)
- The source service ID will always be `0xAE`
- The destination service ID will always be `0xAF`
- The application-layer token will always be `0x20`
- The application-payload always contains 10 bytes:
  - `0x05`: lower nibble = size of the UUID minus 1 -> 6 Bytes UUID (expect nothing else)
  - The six bytes of the UUID copied from the address request message, all zeros here
  - `0x00`: the status code (`0x00` means OK, expect nothing else)
  - Two bytes containing the unique SSA assigned to the gateway client (> `0x4000`)

A gateway client just needs to parse the last two bytes in order to obtain the assigned SSA. In the above mentioned
example the assigned SSA is `0x4001`, which is the lower bound of the address space assigned to gateway clients.
Feel free to use that SSA as the source address for all s-net packets sent towards the sensor network. However,
keep in mind that a gateway client *MUST* use that address in order to be compatible with the deprecated Java-based
gateway software. Thus, it is a good advice to ***implement and follow this scheme***.

### The Publish-Subscribe Service
T.b.d.: still just a bunch of text fragments and ideas!

For all s-net packets that are sent by nodes of the sensor network that are not addressed to one of the unicast SSAs
assigned to the gateway clients. Relevant addresses are `0x4000` (`MULTICAST_GATEWAY`), `0xFFFE` (`WIRED_ADDR`), and
`0xFFFF` (???), but there might be more. Messages with such a destination address are "spontaneous" messages that are not replies
to previous requests sent by one of the gateway clients. Instead, the nature of such packets is that they are sent
without an external trigger to anybody who is interested. To avoid flooding such unsolicited packets to each of the
gateway clients, the gateway relays them *only* to gateway clients that explicitely subscribed to them before.
The purpose of the *publish-subscribe service* is to manage these subscriptions.

    RECV: SrcSSA=0x4001, DstSSA=0x4000, ARQ=0, SrcSrv=0x00, DstSrv=0xb0, Token=0x10, with   1 bytes payload: ae
    00 00 40 01 40 00 00 00 b0 00 10 ae 
    SENT: SrcSSA=0x4000, DstSSA=0x4001, ARQ=0, SrcSrv=0xb0, DstSrv=0xb0, Token=0x11, with   2 bytes payload: ae 00
    00 00 40 00 40 01 00 b0 b0 00 11 ae 00 
    RECV: SrcSSA=0x4001, DstSSA=0x4000, ARQ=0, SrcSrv=0x00, DstSrv=0xb0, Token=0x10, with   1 bytes payload: 10
    00 00 40 01 40 00 00 00 b0 00 10 10 
    SENT: SrcSSA=0x4000, DstSSA=0x4001, ARQ=0, SrcSrv=0xb0, DstSrv=0xb0, Token=0x11, with   2 bytes payload: 10 00
    00 00 40 00 40 01 00 b0 b0 00 11 10 00

- 255 service IDs: `0x00` to `0xFE`. `0xFF` is a wildcard to subscribe to all services.
- One subscription exchange per service ID, multiple subscriptions are possible.
- To revoke a subscription subscribe to `0xFF`. After this, rebuild the database by sending the reduced set of
  subscriptions again. If all subscriptions were already set, a subscription will trigger a reset of the database
  before this new subscription is applied.
