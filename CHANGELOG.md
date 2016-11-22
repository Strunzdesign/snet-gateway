# Change Log
All notable changes to this project will be documented in this file.

## [Unreleased]
- Nothing yet


## [1.1] - 2016-11-22
### Added
- Added this change log CHANGELOG.md
- Added specification of the gateway client protocol
- Added support for the length-based framing mode

### Changed
- The structure of the project was changed
- Renamed all "tool" leftovers to the new "gateway client" scheme
- Changed structure of the project reflecting name change to "gateway access protocol"
- Git framing submodule tag v1.1
- Git hdlcd-devel submodule tag v1.1
- Git snet-devel submodule tag v1.1

### Fixed
- Do not overwrite the destination address on packets sent to a gateway client
- Multiple gateway clients can receive the same payload now
- Fixed masquerading of the addresses of injected s-net packages


## [1.0] - 2016-10-06
### Added
- First tested version without any open issues
- Makes use of git submodules for "externals"
- Works well with s-net(r) BASE release 3.6
- Works well with each version of the HDLC Daemon (HDLCd)

[Unreleased]: https://github.com/Strunzdesign/snet-gateway/compare/v1.1...HEAD
[1.1]: https://github.com/Strunzdesign/snet-gateway/compare/v1.0...v1.1
