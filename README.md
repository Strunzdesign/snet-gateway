# A gateway for s-net(r) devices (snet-gateway)
A gateway software to exchange and handle packets of s-net(r) devices via the HDLC Daemon.

This package offers a gateway software to exchange packets with s-net(r) devices of the Fraunhofer-Institute for Integrated Circuits (IIS).
This package connects to the "HDLC Daemon" (HDLCd) that implements the "High-level Data Link Control" protocol (HDLC). The HDLC-Daemon is found here:
- https://github.com/Strunzdesign/hdlcd

This software is intended to be portable and makes use of the boost libraries. It was tested on GNU/Linux (GCC toolchain)
and Microsoft Windows (nuwen MinGW).

## Latest stable release of the s-net(r) gateway software:
- v1.0 from 06.10.2016 
  - Makes use of git submodules for "externals"
  - Works well with s-net(r) BASE release 3.6
  - Works well with each version of the HDLC Daemon (HDLCd)

## Required libraries and tools:
- GCC, the only tested compiler collection thus far (tested: GCC 4.9.3, GCC 6.1)
- Boost, a platform-independent toolkit for development of C++ applications
- CMake, the build system
- Doxygen, for development
- nuwen MinGW, to compile the software on Microsoft Windows (tested: 13.4, 14.0)

## Documentation
- See *online doxygen documentation* at http://strunzdesign.github.io/snet-gateway/
- Read the specification of the *gateway client protocol* at https://github.com/Strunzdesign/snet-gateway/blob/master/doc/protocol.md
- Check the *change log* at https://github.com/Strunzdesign/snet-gateway/blob/master/CHANGELOG.md
- View the *install instructions* at https://github.com/Strunzdesign/snet-gateway/blob/master/INSTALL
- Read the *HOWTO* at https://github.com/Strunzdesign/snet-gateway/blob/master/doc/HOWTO.txt
