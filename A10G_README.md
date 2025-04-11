# a11g-Cloud

* Team Number: T06
* Team Name: Byte Crafter
* Team Members: Tony Yan & Yue Zhang
* GitHub Repository URL: https://github.com/ese5160/final-project-t06-byte-crafter
* Description of test hardware: ROG Zephyrus G14, HUAWEI 14

## 1. OTAFU

1. [fw_CLI](https://drive.google.com/file/d/1AkIwNU6J5DHiIGCX_sEbTWcL2S3Hj_37/view?usp=sharing)

2. Done, codes are under Bootloader and Application files.

## 2. Golden Image

1. [CRC_Correct](https://drive.google.com/file/d/1oC8NIgpK6hW3kwf3ayY1jSHdP63nI2TL/view?usp=sharing)

2. [CRC_Incorrect](https://drive.google.com/file/d/1fnJjOP-l3CE4UHcpJJ2hPr719Hdra46C/view?usp=sharing)

3. Done, codes are under Bootloader and Application files.

## 3. Node-RED Design

### 3.1 General Program Flow between Device and Cloud

Our system integrates a fingerprint scanner and a motor-controlled door to manage both physical access and secure interaction with the LCD display. The system communicates with the cloud to make access decisions based on fingerprint identity and door state.

1. Fingerprint Scanning Triggered:
    1. When a user places their finger on the fingerprint scanner, the system captures the fingerprint ID. Simultaneously, it checks the current state of the door (open or closed). Both the fingerprint ID and the door state are sent to the cloud.

2. Cloud Processing:    
    1. The cloud uses the fingerprint ID to:
         - Determine whether it is a registered normal fingerprint or a registered duress fingerprint.

         - Use the accompanying door state to help infer the user's intention if the fingerprint is normal.

3. Cloud Response:
    1. If it is a duress fingerprint:
        - The cloud immediately issues an alarm, regardless of whether the door is open or closed.

    2. If it is a normal fingerprint:
        - If the door is open, the cloud assumes the user wants to access the LCD display, and sends back permission to proceed.
        - If the door is closed, the cloud assumes the user intends to unlock the door, and sends back permission to open it.

### 3.2 All MQTT Topics


### 3.3 Describe for Each Topic


### 3.4 Divide MCU Application Code into Threads



## 4. Bidirectional Cloud Communication


## 5. Node-RED Implementation


## 6. Percepio Analysis