## Table of Contents

* [Introduction](#introduction)
* [Navigating the Repository](#navigating-the-repository)
* [Required Tools](#required-tools)
* [Code Examples List](#code-examples-list)
* [References](#references)

# Introduction
This repository contains examples and demos for PSoC 6 MCU family of devices, a single chip solution for the emerging IoT devices. PSoC 6 MCU bridges the gap between expensive, power hungry application processors and low‑performance microcontrollers (MCUs). The ultra‑low‑power, dual-core architecture of PSoC 6 MCU offers the processing performance needed by IoT devices, eliminating the tradeoffs between power and performance.

Cypress provides a wealth of data at [www.cypress.com](http://www.cypress.com/) to help you select the right PSoC device and effectively integrate it into your design. Visit our [PSoC 6 MCU](http://www.cypress.com/products/32-bit-arm-cortex-m4-psoc-6) webpage to explore more about PSoC 6 MCU family of device.
Feel free to explore through the code example source files and let us innovate together!

# Navigating the Repository

This repository provides examples demonstrating low power implementation using PSoC 6, mechanisms to field-upgrade the device firmware using various types of bootloaders, Watchdog implementation using PSoC 6 etc.
If you are new to developing projects with PSoC 6 MCU, we recommend you to refer the [PSoC 6 Getting Started GitHub](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Getting-Started) page which can help you familiarize with device features and guides you to create a simple PSoC 6 design with PSoC Creator IDE. For other block specific design please visit the following GitHub Pages:
#### 1. [Analog Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Analog-Designs)
#### 2. [Digital Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Digital-Designs)
#### 3. [BLE Connectivity Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-BLE-Connectivity-Designs)
#### 4. [System-Level Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-System-Level-Designs)
#### 5. [PSoC 6 Pioneer Kit Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Pioneer-Kits)
#### 6. [PSoC 6 MCU based RTOS Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-RTOS-Based-Design)
#### 7. [Audio Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Audio-Designs)
You can use these block level examples to guide you through the development of a system-level design using PSoC 6 MCU. All the code examples in this repository comes with well documented design guidelines to help you understand the design and how to develop it. The code examples and their associated documentation are in the Code Example folder in the repository.

# Required Tools

## Software
### Integrated Development Environment (IDE)
To use the code examples in this repository, please download and install
[PSoC Creator](http://www.cypress.com/products/psoc-creator)

## Hardware
### PSoC 6 MCU Development Kits
* [CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit](http://www.cypress.com/documentation/development-kitsboards/psoc-6-ble-pioneer-kit).

* [CY8CKIT-062 PSoC 6 WiFi-BT Pioneer Kit](http://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit). 

**Note** Please refer to the code example documentation for selecting the appropriate kit for testing the project

## Code Example List

### Bootloaders
#### 1. CE213093 - PSoC 6 MCU Basic Bootloaders
These examples demonstrate basic bootloading with PSoC® 6 MCU. This includes downloading an application from a host and
installing it in device flash, and then transferring control to that application.
#### 2. CE216767 – PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity Bootloader
This example demonstrates simple over the air (OTA) bootloading with a PSoC® 6 MCU with Bluetooth Low Energy (BLE)
connectivity. This includes downloading an application from a host, installing it in device flash, and then transferring control to
that application. The downloaded application demonstrates several basic Bluetooth services. It is based on CE215121 BLE HID
Keyboard, with small changes to support the Bootloader SDK
#### 3. CE220959 – PSoC 6 MCU with BLE Bootloader Using External Memory
This example demonstrates over the air (OTA) bootloading with a PSoC 6 MCU with Bluetooth Low Energy (BLE) connectivity
using an external memory. The application is downloaded into the external memory, verified, and afterwards copied into the
internal flash memory for execution.
#### 4. CE220960 – PSoC 6 MCU BLE Upgradable Stack Bootloader
This example demonstrates over the air (OTA) bootloading with a PSoC 6 MCU with Bluetooth Low Energy (BLE)
connectivity. The BLE stack code is shared between applications to reduce flash usage. The bootloader may download
updates to the BLE stack or to the application.
#### 5. CE221984 – PSoC 6 MCU Dual-Application Bootloader
This code example demonstrates a PSoC® 6 MCU Bootloader with two applications. Either application can be downloaded
from a host through an I2C communication channel and installed in device flash. The bootloader then transfers control to one
of the applications, in either basic mode or factory default (“golden image”) mode.

### Direct Memory Access (DMA)
#### 1. CE218552 – PSoC 6 MCU: UART to Memory Buffer Using DMA
This example demonstrates how a PSoC 6 DMA channel transfers data received from the UART to a buffer in memory. When
the buffer is filled, a second DMA channel drains the buffer to the UART, to be transmitted.
#### 2. CE218553 – PSoC 6 MCU: PWM Triggering a DMA Channel
This code example demonstrates how to route trigger signals in PSoC 6. In this code example, PSoC Creator is used to
configure the trigger multiplexer. This is demonstrated using a PWM trigger routed to a DMA channel. The PWM is connected
to an LED to implement a variable intensity. The PWM also triggers the DMA in every cycle. The DMA is used to update the
PWM duty cycle to create a breathing effect on the LED. 
#### 3. CE219940 – PSoC 6 MCU Multiple DMA Concatenation
This example demonstrates the use of multiple concatenated DMA channels to manipulate memory, with no CPU usage. The
incoming data from the UART is packed into 5-byte packets and stored in a memory array, along with a timestamp from the
RTC. When four packets of data are stored, they are echoed back to the UART using DMA.

### GPIO
#### 1. CE220263 – PSoC 6 MCU GPIO Pins Example
This example demonstrates multiple methods of configuring, reading, writing, and generating interrupts with PSoC 6 General
Purpose Input/Output (GPIO) pins. Both the PSoC Creator schematic Pins Component and PDL GPIO driver methods are shown.

### Interrupt
#### 1. CE219339 – PSoC 6 MCU – MCWDT and RTC Interrupts (Dual CPU)
PSoC 6 MCU has dual CPUs: an Arm Cortex-M4 (CM4) and a Cortex-M0+ (CM0+). This example uses a PSoC 6 MCU
multi-counter watchdog timer (MCWDT) and a real-time clock (RTC) to generate the interrupts. The interrupts are assigned to
separate CPUs. 
#### 2. CE219521 – PSoC 6 MCU - GPIO Interrupt
This example demonstrates how to configure a GPIO to generate an interrupt using PSoC 6 MCU. The example also shows
how the GPIO interrupt can be used to wake the CPU from Deep Sleep low-power mode.

### Memory
#### 1. CE220120 – PSoC 6 MCU: Blocking Mode Flash Write
This example demonstrates how to write to the flash memory of a PSoC 6 MCU device. In this example, the flash write API
function blocks the caller until the write is completed.
#### 2. CE220823 – PSoC 6 MCU SMIF Memory Write and Read Operation
This example writes and reads 256 bytes of data to external memory using SMIF quad mode. The example also checks the
integrity of read data against written data.
#### 3. CE221122 – PSoC 6 MCU Non-blocking Flash Write
These examples implement the flash write using polling and interrupt to complete the flash write operation.
#### 4. CE222460 - SPI F-RAM Access Using PSoC 6 MCU SMIF
CE222460 provides a code example that implements the SPI host controller on PSoC 6 MCU using the SMIF Component and
demonstrates accessing different features of the SPI F-RAM. The result is displayed by driving the status LED (RGB) which
turns green when the result is a pass, and turns red when the result is a fail. The code example also enables the UART
interface to connect to a PC to monitor the result.
#### 5. CE222967 – Excelon™-Ultra QSPI F-RAM Access Using PSoC 6 MCU SMIF 
CE222967 provides a code example that implements the QSPI host controller on the PSoC 6 MCU device using the SMIF
Component and demonstrates accessing different features of the QSPI F-RAM™. The result is displayed by driving the status
LED (RGB), which turns green when the result is a pass, and turns red when the result is a fail. The code example also
enables the UART interface to connect to a PC to monitor the result.


## References
#### 1. PSoC 6 MCU
PSoC 6 bridges the gap between expensive, power hungry application processors and low‑performance microcontrollers (MCUs). The ultra‑low‑power PSoC 6 MCU architecture offers the processing performance needed by IoT devices, eliminating the tradeoffs between power and performance. The PSoC 6 MCU contains a dual‑core architecture, with both cores on a single chip. It has an Arm® Cortex®‑M4 for high‑performance tasks, and an Arm® Cortex®‑M0+ for low-power tasks, and with security built-in, your IoT system is protected.
To learn more on the device, please visit our [PSoC 6 MCU](http://www.cypress.com/products/32-bit-arm-cortex-m4-psoc-6) webpage.

####  2. PSoC 6 MCU Learning resource list
##### 2.1 PSoC 6 MCU Datasheets
Device datasheets list the features and electrical specifications of PSoC 6 families of devices: [PSoC 6 MCU Datasheets](http://www.cypress.com/search/all?f%5B0%5D=meta_type%3Atechnical_documents&f%5B1%5D=resource_meta_type%3A575&f%5B2%5D=field_related_products%3A114026)
##### 2.2 PSoC 6 MCU Application Notes
Application notes are available on the Cypress website to assist you with designing your PSoC application: [A list of PSoC 6 MCU ANs](http://www.cypress.com/psoc6an)
##### 2.3 PSoC 6 MCU Component Datasheets
PSoC Creator utilizes "components" as interfaces to functional Hardware (HW). Each component in PSoC Creator has an associated datasheet that describes the functionality, APIs, and electrical specifications for the HW. You can access component datasheets in PSoC Creator by right-clicking a component on the schematic page or by going through the component library listing. You can also access component datasheets from the Cypress website: [PSoC 6 Component Datasheets](http://www.cypress.com/documentation/component-datasheets)
##### 2.4 PSoC 6 MCU Technical Reference Manuals (TRM)
The TRM provides detailed descriptions of the internal architecture of PSoC 6 devices:[PSoC 6 MCU TRMs](http://www.cypress.com/psoc6trm)

## FAQ

### Technical Support
Need support for your design and development questions? Check out the [Cypress Developer Community 3.0](https://community.cypress.com/welcome).  

Interact with technical experts in the embedded design community and receive answers verified by Cypress' very best applications engineers. You'll also have access to robust technical documentation, active conversation threads, and rich multimedia content. 

You can also use the following support resources if you need quick assistance:
##### Self-help: [Technical Support](http://www.cypress.com/support)
##### Local Sales office locations: [Sales Office](http://www.cypress.com/about-us/sales-offices)
