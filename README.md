This is a new cross-mode conversion program that makes use of the MMDVM-Transcoder to offer previously unavailable cross-mode options. The ultimate aim
is to replace the programs currently available in the MMDVM_CM package in a more integrated manner. It will typically sit between the MMDVM Host and
the appropriate mode gateway, or maybe after a mode gateway and another mode gateway, depending on the configuration.

The final version will transcode between all of the following digital and non-digital voice modes.

- D-Star
- DMR
- System Fusion (DN and VW modes)
- P25 phase 1
- NXDN
- FM
- M17

The initial versions will only allow for D-Star, M17, and FM, with System Fusion being possible, depending on the time available before Hamvention 2024.

It builds on 32-bit and 64-bit Linux as well as on Windows using Visual Studio
2019 on x86 and x64. It can optionally control various Displays. Currently
these are:

This software is licenced under the GPL v2 and is primarily intended for amateur and
educational use.
