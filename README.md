AWS coreMQTT Mutual Authentication Demo
=======================================

This demo application connects to **AWS MQTT broker** using TLS with mutual authentication between the client and the server.
It demonstrates the subscribe-publish workflow of MQTT.

Visit [*coreMQTT mutual authentication demo*](https://docs.aws.amazon.com/freertos/latest/userguide/mqtt-demo-ma.html) for further information.

Please note, that [*properly configured thing*](https://docs.aws.amazon.com/iot/latest/developerguide/iot-moisture-create-thing.html) is required to
successfully run the demo application.

Targets:
--------
  - NXP [IMXRT1050-EVKB](./Board/IMXRT1050-EVKB/README.md) Board (`target-type: IP-Stack`, using FreeRTOS+TCP over Ethernet)
  - STMicroelectronics [B-U585I-IOT02A](./Board/B-U585I-IOT02A/README.md) Board (`target-type: WiFi`, using on-board WiFi module)
  - [Arm Virtual Hardware for Corstone-300](./Board/AVH_MPS3_Corstone-300/README.md) (`target-type: AVH`, using VSocket)

Configure
---------
Locate the following files (in subdirectory `amazon-freertos/demos/include`):
- `aws_clientcredential.h`
- `aws_clientcredential_keys.h`.

Modify the following definitions:
- `clientcredentialMQTT_BROKER_ENDPOINT`: Remote Host Address (AWS IoT->Settings in AWS IoT console)
- `clientcredentialIOT_THING_NAME`: Thing Name (AWS IoT->Manage->Things->Name in AWS IoT console)
- `clientcredentialWIFI_SSID`: WiFi Access Point SSID (when connecting via WiFi, can be empty otherwise)
- `clientcredentialWIFI_PASSWORD`: WiFi Access Point Password (when connecting via WiFi, can be empty otherwise)
- `keyCLIENT_CERTIFICATE_PEM`: Client Certificate
- `keyCLIENT_PRIVATE_KEY_PEM`: Client Private Key


Build
-----
1. Prerequisites:
   - [CMSIS-Toolbox 0.10.2](https://github.com/Open-CMSIS-Pack/devtools/releases/tag/tools%2Ftoolbox%2F0.10.2)
   - Arm Compiler 6.18 or later
2. Create `.cprj` project using `csolution`:  
   `csolution convert -s Demo.csolution.yml -c Demo.<build-type>+<target-type>`  
     - `<build-type>:  Debug | Release`
     - `<target-type>: IP-Stack | WiFi | AVH`
3. Build `.cprj` project using `cbuild`:  
   `cbuild Demo.<build-type>+<target-type>.cprj`

Run
---
- Connect and configure the debugger.
- Run the application and view messages in a debug printf or terminal window.
>Note: click on Target links above for target specific information.

MQTT messages can be viewed in the [**AWS IoT console**](https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html).
