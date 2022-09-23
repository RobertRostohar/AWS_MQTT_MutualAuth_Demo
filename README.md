AWS coreMQTT Mutual Authentication Demo
=======================================

This demo application connects to **AWS MQTT broker** using TLS with mutual authentication between the client and the server.
It demonstrates the subscribe-publish workflow of MQTT.

Visit [*coreMQTT mutual authentication demo*](https://docs.aws.amazon.com/freertos/latest/userguide/mqtt-demo-ma.html) for further information.

Please note, that [*properly configured thing*](https://docs.aws.amazon.com/iot/latest/developerguide/iot-moisture-create-thing.html) is required to
successfully run the demo application.

Configuration
-------------
In uVision open Project window, expand `Target` and `demos_include` group and locate files:
- `aws_clientcredential.h`
- `aws_clientcredential_keys.h`.

Modify the following definitions:
- `clientcredentialMQTT_BROKER_ENDPOINT`: Remote Host Address (AWS IoT->Settings in AWS IoT console)
- `clientcredentialIOT_THING_NAME`: Thing Name (AWS IoT->Manage->Things->Name in AWS IoT console)
- `clientcredentialWIFI_SSID`: WiFi Access Point SSID (when connecting via WiFi, can be empty otherwise)
- `clientcredentialWIFI_PASSWORD`: WiFi Access Point Password (when connecting via WiFi, can be empty otherwise)
- `keyCLIENT_CERTIFICATE_PEM`: Client Certificate
- `keyCLIENT_PRIVATE_KEY_PEM`: Client Private Key

Once the application is configured you can:
- Build the application.
- Connect and configure the debugger.
- Run the application and view messages in a debug printf or terminal window.

MQTT messages can be viewed in the [**AWS IoT console**](https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html).
