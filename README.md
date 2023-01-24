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

Configure AWS IoT Thing:
  - Modify the following definitions in [aws_clientcredential.h](amazon-freertos/demos/include/aws_clientcredential.h):
    - `clientcredentialMQTT_BROKER_ENDPOINT`: Remote Host Address (AWS IoT->Settings in AWS IoT console)
    - `clientcredentialIOT_THING_NAME`: Thing Name (AWS IoT->Manage->Things->Name in AWS IoT console)
  - Modify the following definitions in [aws_clientcredential_keys.h](amazon-freertos/demos/include/aws_clientcredential_keys.h):
    - `keyCLIENT_CERTIFICATE_PEM`: Client Certificate
    - `keyCLIENT_PRIVATE_KEY_PEM`: Client Private Key

Configure WiFi Access Point (when connecting via WiFi):
  - Modify the following definitions in [socket_startup.c](Socket/WiFi/socket_startup.c):
    - `SSID`:          WiFi Access Point SSID
    - `PASSWORD`:      WiFi Access Point Password
    - `SECURITY_TYPE`: WiFi Access Point Security

Build
-----
1. Prerequisites:
   - [CMSIS-Toolbox 1.1.0](https://github.com/Open-CMSIS-Pack/cmsis-toolbox/releases/tag/1.1.0) or later
   - Arm Compiler 6.18 or later
   - CMSIS packs listed in [Demo.csolution.yml](Demo.csolution.yml)  
     Packs can be installed by executing the following `csolution` and `cpackget` commands:
     ```
     csolution list packs -s Demo.csolution.yml -m >packs.txt
     cpackget add -f packs.txt
     ```

2. Create `.cprj` project using `csolution`:  
   `csolution convert -s Demo.csolution.yml -c Demo.<build-type>+<target-type>`  
     - `<build-type>:  Debug | Release`
     - `<target-type>: IP-Stack | WiFi | AVH`
3. Build `.cprj` project using `cbuild`:  
   `cbuild Demo.<build-type>+<target-type>.cprj`

Program
--------
- Download the executable file (.axf) to the microcontroller using a programmer or Drag-and-drop programming if available.
>Note: not required for Virtual Hardware.

Run
---
- Connect and configure the debugger.
- Run the application and view messages in a debug printf or terminal window.
>Note: click on Target links above for target specific information.

MQTT messages can be viewed in the [**AWS IoT console**](https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html).

CI Testing
----------
To build and run this application with a CI workflow on GitHub the following steps are required. For details refer to [Run AMI with GitHub Actions](https://arm-software.github.io/AVH/main/infrastructure/html/run_ami_github.html).

1. **Amazon Web Service (AWS) account** with:
    - Amazon EC2 (elastic cloud) access
    - Amazon S3 (storage) access
    - Registration to access AVH Amazon Machine Image [AVH AMI](https://aws.amazon.com/marketplace/search/results?searchTerms=Arm+Virtual+Hardware)
    - User role setup for scripted API access

2. **GitHub**:
    - Fork this repository with at least _Write_ access rights
    - Store the AWS account configuration (obtained in step 1) as
    [GitHub Secrets](https://docs.github.com/en/actions/security-guides/encrypted-secrets) - **AWS Access** values in the forked repository
    
3. **AWS IoT Thing**:
    - Use the [AWS IoT console](https://console.aws.amazon.com/iotv2/) to create a thing, download its certificates, create a policy, and attach the policy to the thing
    - Store this configuration as [GitHub Secrets](https://docs.github.com/en/actions/security-guides/encrypted-secrets) - **IoT Cloud Access** values in the forked repository

**GitHub Secrets - Values**

The following (secret) configuration values need to be added to the repositories [Secret store](../../settings/secrets/actions):

Secret Name                    | Description
:------------------------------|:--------------------
**AWS Access**                 | **Settings and credentials to access AWS services for running Arm Virtual Hardware**
`AWS_IAM_PROFILE`              | The [IAM Instance Profile](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/iam-roles-for-amazon-ec2.html) associated with the AVH EC2 instance granting it access to required AWS resources.
`AWS_ASSUME_ROLE`              | The AWS access role to be assumed for AWS access.
`AWS_S3_BUCKET_NAME`           | The name of the [S3 storage bucket](https://docs.aws.amazon.com/AmazonS3/latest/userguide/creating-buckets-s3.html) to be used for temporary data storage by Arm Virtual Hardware.
`AWS_DEFAULT_REGION`           | The data center region for running new AVH AMI. For example `eu-west-1`.
`AWS_SECURITY_GROUP_ID`        | The id of the [VPC security group](https://docs.aws.amazon.com/vpc/latest/userguide/VPC_SecurityGroups.html) to add the EC2 instance to. Shall have format `sg-xxxxxxxx`.
`AWS_SUBNET_ID`                | The id of the [VPC subnet](https://docs.aws.amazon.com/vpc/latest/userguide/working-with-vpcs.html#view-subnet) to connect the EC2 instance to. Shall have format `subnet-xxxxxxxx`.
**IoT Cloud Access**           | **Settings and credentials required to connect an [AWS IoT Thing](https://github.com/MDK-Packs/Documentation/tree/master/AWS_Thing)**
`CLIENT_CERTIFICATE_PEM`       | Client (device) certificate
`CLIENT_PRIVATE_KEY_PEM`       | Client (device) private key
`IOT_THING_NAME`               | Client  (device) name
`MQTT_BROKER_ENDPOINT`         | MQTT broker host name
