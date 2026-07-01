![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# MPLAB® Harmony 3 Smart Energy PRIME application examples

MPLAB® Harmony 3 is an extension of the MPLAB® ecosystem for creating embedded firmware solutions for Microchip 32-bit SAM and PIC® microcontroller and microprocessor devices. Refer to the following links for more information.

- [Microchip 32-bit MCUs](https://www.microchip.com/design-centers/32-bit)
- [Microchip 32-bit MPUs](https://www.microchip.com/design-centers/32-bit-mpus)
- [Microchip MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide)
- [Microchip MPLAB® Harmony](https://www.microchip.com/mplab/mplab-harmony)
- [Microchip MPLAB® Harmony Pages](https://microchip-mplab-harmony.github.io/)

This repository contains the MPLAB® Harmony 3 Smart Energy PRIME BN and PRIME SN application examples.

- [Release Notes](./release_notes.md)
- [License](./License.md) 
# Documentation

Click [here](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=MPLAB_Harmony_Smart_Energy_PRIME_Applications&redirect=true) to view the online documentation of code examples hosted in this repository.
To view the documentation offline, follow these steps:
 - Download the publication as a zip file from [here](https://onlinedocs.microchip.com/download/GUID-B0EED1DD-60EE-41BF-A319-00EF8C6E51CB?type=webhelp).
 - Extract the zip file into a folder.
 - Navigate to the folder and open **index.html** in a web browser of your choice.
 
# Contents Summary

| Folder     | Description                             |
| ---        | ---                                     |
| apps       | Contains PRIME PHY, bootloader and full-stack example applications. |

# Code Examples

The following applications are provided to demonstrate the typical use cases of PRIME at both PHY and full stack levels.

| Name               | Description |
| ----               | ----------- |
| [PHY PLC and Go](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_PLC_Go_PRIME&redirect=true) | This example is intended to show a simple application running on top of the PRIME-PLC PHY layer. |
| [PHY Sniffer Tool](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_Sniffer&redirect=true) | The PHY Sniffer is an application example that uses the PHY layer to monitor PLC frames in the PRIME-PLC network and send them via USI serialization. |
| [Hybrid PHY Sniffer Tool](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_Sniffer&redirect=true) | The Hybrid PHY Sniffer is an application example that uses the PHY layers to monitor PLC and RF frames in the PRIME-Hybrid network and send them via USI serialization. |
| [PHY Tester Tool](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_Tester_Tool&redirect=true) | The PHY Tester tool is an application example that allows checking the complete performance of the Microchip PRIME-PLC PHY Layer on PLC boards. |
| [Hybrid PHY Tester Tool](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_Tester_Tool&redirect=true) | The Hybrid PHY Tester tool is an application example that allows checking the complete performance of the Microchip PRIME-PLC and IEEE 802.15.4 PHY Layers on Hybrid PLC-RF boards. |
| [PLC PHY Tx Test Console](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_PHY_TX_Console&redirect=true) | The PLC PHY Tx Test Console is an application example that demonstrates the complete transmission performance of the Microchip PRIME-PLC PHY Layer, avoiding timing limitations in the PC host. |
| [PRIME 1.3 Base Modem](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Base_Modem&redirect=true) | The PRIME 1.3 Base Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a Base Node with modem capabilities. |
| [PRIME 1.4 Base Modem](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Base_Modem&redirect=true) | The PRIME 1.4 Base Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a Base Node with modem capabilities. |
| [PRIME 1.4 Service Modem](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Modem&redirect=true) | The PRIME 1.4 Service Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a Service Node (both application and stack) with modem capabilities oriented for low resource platform. |
| [PRIME Dual Service Modem](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Dual_Modem&redirect=true) | The PRIME Dual Service Modem is an application example that demonstrates how to configure the PRIME Stack and its serial interface as a dual Service Node with modem capabilities. |
| [PRIME Dual Service Metering Demo](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Dual_Metering&redirect=true) | The PRIME Dual Service Metering Demo implements a metering application and a PRIME dual Service Node, capable of joining a PRIME network and replying to IEC 61334-4-32 requests. It demonstrates how to send metrology data through the IEC 61334-4-32 connection. |
| [PRIME 1.3 Service Bin](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Bin&redirect=true) | The PRIME 1.3 Service Bin is an application example that demonstrates how to configure the PRIME Library for PRIME 1.3. |
| [PRIME 1.4 Service Bin](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Bin&redirect=true) | The PRIME 1.4 Service Bin is an application example that demonstrates how to configure the PRIME Library for PRIME 1.4. |
| [PRIME Bootloader](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Bootloader&redirect=true) | The PRIME Bootloader is an application example that demonstrates how to use the bootloader in a Service Node for firmware upgrade. |
| [PRIME Bootloader in External Memory](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=SE_PRIME_apps_Service_Bootloader_External_Memory&redirect=true) | The PRIME Bootloader in External Memory is an application example that demonstrates how to use a Service Node bootloader that stores the firmware upgrade image in an external memory. |

____

[![License](https://img.shields.io/badge/license-Harmony%20license-orange.svg)](https://github.com/Microchip-MPLAB-Harmony/smartenergy_prime_apps/blob/master/Microchip_SLA001.md)
[![Commit activity](https://img.shields.io/github/commit-activity/y/Microchip-MPLAB-Harmony/smartenergy_prime_apps.svg)](https://github.com/Microchip-MPLAB-Harmony/smartenergy_prime_apps/graphs/commit-activity)
[![Contributors](https://img.shields.io/github/contributors-anon/Microchip-MPLAB-Harmony/smartenergy_prime_apps.svg)]()

____

[![Developer Help](https://img.shields.io/badge/Youtube-Developer%20Help-red.svg)](https://www.youtube.com/MicrochipDeveloperHelp)
[![Developer Help](https://img.shields.io/badge/XWiki-Developer%20Help-torquiose.svg)](https://developerhelp.microchip.com/xwiki/bin/view/software-tools/harmony/)
[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/user/MicrochipTechnology)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/microchip-technology)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/microchiptechnology/)
[![Follow us on Twitter](https://img.shields.io/twitter/follow/MicrochipTech.svg?style=social)](https://twitter.com/MicrochipTech)

[![](https://img.shields.io/github/stars/Microchip-MPLAB-Harmony/smartenergy_prime_apps.svg?style=social)]()
[![](https://img.shields.io/github/watchers/Microchip-MPLAB-Harmony/smartenergy_prime_apps.svg?style=social)]()