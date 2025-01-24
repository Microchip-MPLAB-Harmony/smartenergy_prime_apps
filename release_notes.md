![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# Microchip MPLAB® Harmony 3 Release Notes

## Harmony 3 Smart Energy PRIME application examples v1.1.0-E1

### Development kit and demo application support

The following development kits are used on provided G3 Demo Applications:

- [SAM E70 Xplained Ultra Evaluation Kit](https://www.microchip.com/en-us/development-tool/DM320113)
- [PIC32CXMTG-EK Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV11K09A)
- [PL460 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV13L63A)
- [ATREB215-XPRO-A EXTENSION BOARD](https://www.microchip.com/en-us/development-tool/ATREB215-XPRO-A)

### New Features

Added all the examples needed to run a whole PRIME 1.4 and PRIME 1.3 stack.
Applications updated to use the code from new releases of dependent Harmony 3 modules.
The following table provides a list of available applications, supported platforms and a brief description of functionalities:

| Application | Platform | Description |
| ----------- | -------- | ----------- |
| PHY PLC and Go | [PIC32CX-MTG, SAME70] + PL460-EK | PLC Chat Demo application using PLC PHY API |
| PHY Tester Tool | [PIC32CX-MTG, SAME70] + PL460-EK | PLC PHY application to send/receive frames using Microchip PLC PHY Tester Tool or Python libraries through a Serial Link on development board |
| PHY Tester Hybrid Tool | [PIC32CX-MTG, SAME70] + PL460-EK + ATREB215-XPRO-A | PLC & RF PHY application to send/receive frames using Python libraries through a Serial Link on development board |
| PHY Sniffer Tool | [PIC32CX-MTG, SAME70] + PL460-EK | PHY application to spy traffic on PLC medium and send it for graphical presentation connected to Microchip Hybrid Sniffer Tool |
| PHY Sniffer Hybrid Tool | [PIC32CX-MTG, SAME70] + PL460-EK + ATREB215-XPRO-A | PHY application to spy traffic on PLC & RF media and send it for graphical presentation connected to Microchip Hybrid Sniffer Tool |
| PHY Tx Test Console | [PIC32CX-MTG, SAME70] + PL460-EK | PLC PHY demo application to manage PLC transmissions via serial console |
| PRIME 1.3 Base Node Modem | PIC32CX-MTG + PL460-EK | The PRIME 1.3 Base Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a Base Node with modem capabilies. |
| PRIME 1.4 Base Node Modem | PIC32CX-MTG + PL460-EK + ATREB215-XPRO-A | The PRIME 1.4 Base Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a Base Node with modem capabilies. |
| PRIME Dual Service Modem | PIC32CX-MTG + PL460-EK + ATREB215-XPRO-A | The PRIME Dual Service Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a dual Service Node with modem capabilies. |
| PRIME 1.3 Service Bin | PIC32CX-MTG | The PRIME 1.3 Service Bin is an application example that demonstrates how to configure the PRIME Library for PRIME 1.3. |
| PRIME 1.4 Service Bin | PIC32CX-MTG | The PRIME 1.4 Service Bin is an application example that demonstrates how to configure the PRIME Library for PRIME 1.4. |
| PRIME Bootloader | PIC32CX-MTG | The PRIME Bootloader is an application example that demonstrates how to use the bootloader in a Service Node for firmware upgrade. |

### Known Issues

- PRIME 1.4 frequency hopping not tested.
- PRIME 1.4 Firwmare Upgrade cannot verify signature from a binary file.
- PRIME 1.3 Not tested.
- No documentation provided.

### Development Tools

- [MPLAB® X IDE v6.20](https://www.microchip.com/mplab/mplab-x-ide)
- [MPLAB® XC32 C/C++ Compiler v4.45](https://www.microchip.com/mplab/compilers)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator 5.5.1 or higher
- [Microchip PLC PHY Tester Tool v3.1.3](https://www.microchip.com/en-us/software-library/se_plc_phy_tester_tool)
- [Microchip Hybrid Sniffer v2.0.4](https://www.microchip.com/en-us/software-library/se_plc_sniffer)
- [Microchip PRIME Manager v2.2.4](https://www.microchip.com/en-us/software-library/se_prime_manager)

In order to regenerate source code for any of the applications, you will also need to use the following versions of the dependent modules (see smartenergy_g3_apps/package.yml):

- Harmony smartenergy repository, v1.2.1
  - Harmony gfx repository, v3.13.0
- Harmony smartenergy\_g3 repository, v1.0.1
  - Harmony crypto repository, v4.0.0-E1
  - Harmony net repository, v3.12.0
- Harmony core repository, v3.13.5
- Harmony csp repository, v3.19.1
- Harmony wolfssl repository, v5.6.7-E1
- Harmony wireless\_15\_4\_phy repository, v1.2.0
- Harmony wireless\_pic32cxbz\_wbz repository, v1.2.0
- Harmony usb repository, 3.12.0 for demos requiring USB
- CMSIS-FreeRTOS v10.5.1 (https://github.com/ARM-software/CMSIS-FreeRTOS/tree/v10.5.1) for demos requiring FreeRTOS support


## Harmony 3 Smart Energy PRIME application examples v1.0.0

### Development kit and demo application support

Following table provides number of PRIME application examples available for different development kits.

| Development Kits  | MPLAB X applications | IAR applications | KEIL applications |
|:-----------------:|:-------------------:|:----------------:|:-----------------:|
| [SAM E70 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DM320113) | 4 | 0 | 0 |

### New Features

The following table provides the list of the SE PLC applications:

| Application                 | Platform                        | Description                                                          |
| ------------ | ------------ | ------------ |
| PLC PHY and Go         | SAME70                  | PLC demo using PLC PHY API |
| PHY Tester Tool         | SAME70                  | PLC PHY  application to connect to Microchip PLC PHY Tester Tool |
| PHY Sniffer Tool         | SAME70                  | PLC PHY  application to connect to Microchip PLC PHY Sniffer Tool |
| PHY Tx Test Console         | SAME70                  | PLC PHY demo application to manage PLC transmissions via serial console |

### Known Issues

- None

### Development Tools

- [MPLAB® X IDE v6.00](https://www.microchip.com/mplab/mplab-x-ide)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator (MCC) v5.1.4
- [MPLAB® XC32 C/C++ Compiler v4.10](https://www.microchip.com/mplab/compilers)
- [Microchip PLC PHY Tester Tool v3.1.3](https://www.microchip.com/en-us/software-library/se_plc_phy_tester_tool)
- [Microchip PLC Sniffer v2.0.3](https://www.microchip.com/en-us/software-library/se_plc_sniffer)
