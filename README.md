# Thesis 9.8/10
* Project Owner: Tu Hoang
* Project Manager: Hoang Tu
* Developer: Huu Tu
* Developer: AWDang
* Hardware: AWDang
* Tester: Hoo Too
* Architecture: Tu Huu

# Secu-re-liable Is All You Need

Secu-re-liable is the latest version of **SRA**, an AIoT-based healthcare system that I initially developed a few months ago. This version focuses on enhancing system performance, eliminating redundant features, and optimizing the data transmission process between the gateway and server. Additionally, the **DVGW** has been redesigned to extend its applicability beyond healthcare, supporting smart home systems, IoT devices, and any sensor-based networks requiring secure and efficient data transfer.

## What's New?
- **Enhanced Reliable & Performance**: Introduced two new frame structures to support key exchange and data transmission efficiently.
- **Optimized Key Generation**: Removed SHA-256 from the key generation process to reduce computational overhead.
- **Improved Process Management**: Replaced the polling approach with a state-machine-based approach for better system efficiency.

## The Frames

It is essentially to implement frame for each transmission process to ensure reliable in data packet. In this framework, I used three different types of frame to handle three types of transmission Sensor -> Gateway, Key exchange, Gateway -> Server. 

### Uart-Frame (U-Frame)

In sensor layer (here I'm using STM32F4), every sensor data collected by STM32 will be packed into U-Frame before sending to ESP32 gateway. It contains Header and Trailer to identify which device is sending and CRC for error checking.

![ảnh](https://github.com/user-attachments/assets/d7750a0c-8f89-4af8-9a51-24539cf6f483)

### Key-exchange Frame (KE Frame)

The ECDH key-exchange will use KE Frame for public key exchange between gateway and server.

![ảnh](https://github.com/user-attachments/assets/cdeb42f6-c24c-4fe0-9290-5d1c0389d57d)

### Data-Server Frame (DS Frame)

For sending data to server (of course it will be encrypted), DS Frame will be used to guarantee securely transmission. 

![ảnh](https://github.com/user-attachments/assets/cb4034ac-95f7-4d11-b019-ca7408f4d63a)

## The state-machine 

### Handshake state-machine

This state machine will be used for key exchange. If any state failed, it will return to state 1 and start again. The key exchange only occur every specific interval (here I setted key exchange will be occured every 10 times of data transmission)

![ảnh](https://github.com/user-attachments/assets/29325b79-9955-481a-a3d4-04148543af68)

### Transmission state-machine

![ảnh](https://github.com/user-attachments/assets/f5b0b92b-4098-46c1-9548-6d662487ced5)







