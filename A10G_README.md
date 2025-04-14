# a10g-cloud

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
When a user places their finger on the fingerprint scanner, the system captures the fingerprint ID. Simultaneously, it checks the current state of the door (open or closed). Both the fingerprint ID and the door state are sent to the cloud.

2. Cloud Processing:
   1. The cloud uses the fingerprint ID to:
      1. Determine whether it is a registered normal fingerprint or a registered duress fingerprint.
      2. Use the accompanying door state to help infer the user's intention if the fingerprint is normal.

3. Cloud Response:
   1. If it is a duress fingerprint:
      1. The cloud immediately issues an alarm, regardless of whether the door is open or closed.
   2. If it is a normal fingerprint:
      1. If the door is open, the cloud assumes the user wants to access the LCD display, and sends back permission to proceed.
      2. If the door is closed, the cloud assumes the user intends to unlock the door, and sends back permission to open it.

### 3.2 List all the Topics System will Use

| **Topic Name**              | **Direction**          | **Payload Format**       | **Description** |
|-----------------------------|------------------------|--------------------------|------------------|
| `a10g/access/request`       | Device → Cloud         | `{ "finger_id": int, "door_open": bool }`                          | Sent from the MCU to the cloud when a fingerprint is scanned. Indicates which finger and door state. |
| `a10g/access/response`      | Cloud → Device         | `{ "access_granted": bool, "operation": "door" \| "lcd" }`         | Response from the cloud indicating whether access is granted and what type of operation to allow.   |
| `a10g/alert/duress`         | Cloud → Dashboard UI   | `"Duress fingerprint detected!"`                                   | Alert message when a duress fingerprint is detected. No response is sent back to the device.         |
| `a10g/register/fingerprint` | Device → Cloud         | `{ "new_finger_id": int, "user_name": string }`                    | Sent during fingerprint enrollment. Used to update cloud-side fingerprint records.                   |
| `a10g/delete/fingerprint`   | Cloud → Device         | `{ "finger_id": int }`                              | Command from cloud to instruct the device to delete a specific fingerprint.   

### 3.3 Describe for Each Topic

| **Topic Name**               | **Published By**        | **Subscribed By**             |
|-----------------------------|--------------------------|-------------------------------|
| `a10g/access/request`       | Device (MCU)             | Cloud (Node-RED)              |
| `a10g/access/response`      | Cloud (Node-RED)         | Device (MCU)                  |
| `a10g/alert/duress`         | Cloud (Node-RED)         | Web Dashboard / Alert Client |
| `a10g/register/fingerprint` | Device (MCU)             | Cloud (Node-RED)              |

### 3.4 Divide MCU Application Code into Threads

**Thread Responsibilities:**

| Thread Name           | Responsibility                                                                 |
|-----------------------|---------------------------------------------------------------------------------|
| `FingerprintThread`   | Scans fingerprint and sends result (finger ID, door state) to `MQTTClientThread`. |
| `MQTTClientThread`    | Handles MQTT communication. Publishes fingerprint data, receives access decisions from cloud. |
| `AccessControlThread` | Acts on cloud decisions: unlocks door or enables LCD based on received operation. |
| `RegistrationThread`  | Handles fingerprint enrollment: captures new fingerprint and sends data to cloud. |

**Inter-Thread Communication:**

| From                  | To                    | Data/Trigger                                                  | Method          |
|-----------------------|------------------------|---------------------------------------------------------------|-----------------|
| `FingerprintThread`   | `MQTTClientThread`     | `{ "finger_id": int, "door_open": bool }`                     | Queue           |
| `MQTTClientThread`    | `AccessControlThread`  | `{ "access_granted": bool, "operation": "door" \| "lcd" }`    | Queue or flags  |
| `AccessControlThread` | `RegistrationThread`   | Local flag or signal to begin enrollment                      | Semaphore/Event |

## 4. Bidirectional Cloud Communication

1. [Bidirectional_Cloud_Communication_Video](https://drive.google.com/file/d/1v-Nby18BvzXBgqDTXp8fBck5pqPXnUZp/view?usp=sharing)

2. [Node-RED Source Code](/Node-RED/flows.json)

3. [Node-RED UI](http:104.211.2.174:1880/ui)

4. Done, code is in WifiHandler.c

## 5. Node-RED Implementation

## 6. Percepio Analysis
