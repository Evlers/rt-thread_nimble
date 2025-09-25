## RT-Thread NimBLE

<img src="http://mynewt.apache.org/img/logo.svg" width="250" alt="Apache Mynewt">

## Overview
<a href="https://github.com/apache/mynewt-nimble/actions/workflows/build_targets.yml">
  <img src="https://github.com/apache/mynewt-nimble/actions/workflows/build_targets.yml/badge.svg">
<a/>

<a href="https://github.com/apache/mynewt-nimble/actions/workflows/build_ports.yml">
  <img src="https://github.com/apache/mynewt-nimble/actions/workflows/build_ports.yml/badge.svg">
<a/>

<a href="https://github.com/apache/mynewt-nimble/actions/workflows/newt_test_all.yml/badge.svg">
  <img src="https://github.com/apache/mynewt-nimble/actions/workflows/newt_test_all.yml/badge.svg">
<a/>

<p>

Apache NimBLE is an open-source Bluetooth 5.4 stack (both Host & Controller)
that completely replaces the proprietary SoftDevice on Nordic chipsets. It is
part of [Apache Mynewt project](https://github.com/apache/mynewt-core).

Feature highlight:
  - Support for 251 byte packet size.
  - Support for all 4 roles concurrently - Broadcaster, Observer, Peripheral and Central
  - Support for up to 32 simultaneous connections.
  - Legacy and SC (secure connections) SMP support (pairing and bonding).
  - Advertising Extensions.
  - Periodic Advertising.
  - Coded (a.k.a. Long Range) and 2M PHYs.
  - Bluetooth Mesh.

Details about NimBLE can be found in the [NimBLE README](./nimble/README.md).<br>

## Using

### Add this repository
- Clone the repository to the `packages` or `libraries` directory in the RT-Thread project.
- In the `libraries` or `packages` folder in the RT-Thread project, include `Kconfig` file for `NimBLE` in its Kconfig files.
- For example, include `NimBLE` in the `libraries` directory:
```Kconfig
menu "External Libraries"
    source "$RTT_DIR/../libraries/rt-thread_nimble/Kconfig"
endmenu
```
