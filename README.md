# Embedded Driver Assistance System (E-DAS)

An embedded C project built on the STM32F411RE microcontroller that acts as a self-contained driver assistance system. Designed to be powered via a standard 9-12V vehicle auxiliary outlet, E-DAS continuously monitors environmental and vehicle metrics, provides real-time visual feedback, and logs diagnostic data to an SD card for long-term analysis.

## Features

* **Proximity Detection:** Uses an ultrasonic sensor with input-capture hardware timers to alert the driver of objects < 10cm away.
* **Impact & Acceleration Monitoring:** Interfaces with an ICM-20689 6-axis accelerometer via I2C and DMA to detect harsh braking, unsafe driving (>0.50g), and impacts (>1.50g).
* **Ambient Light Sensing:** Utilizes a photodiode and a custom transimpedance amplifier to measure cabin lux levels via ADC, prompting the driver to turn on headlights in low-light conditions (<300 lux).
* **Temperature Monitoring:** Reads pulse-train data from an LMT01 sensor using a custom comparator circuit and hardware external triggers to warn of uncomfortable cabin temperatures (>30°C).
* **GPS Tracking:** Parses NMEA 0183 strings via UART from a NEO-6M module for real-time location, speed, and heading data.
* **Data Logging:** Uses the FatFS middleware over SPI to log time-stamped system events and warnings to a MicroSD card.
* **Interactive UI:** Features a non-blocking, interrupt-driven 4x3 keypad and tactile button interface to navigate menus rendered on a 0.91" I2C OLED display.

## Hardware Architecture

The system utilizes custom power regulation (5V and 3.3V rails) and signal conditioning to ensure clean data acquisition. 

**Core Components:**
* **MCU:** STM32F411RE Nucleo-64
* **Sensors:** HC-SR04 (Distance), ICM-20689 (Acceleration), LMT01 (Temperature), SFH203 (Light), NEO-6M (GPS)
* **Peripherals:** 0.91" SSD1306 OLED, MicroSD Card Module, 4x3 Matrix Keypad, Status LEDs

## Software Architecture

The firmware is designed for high reliability and responsiveness, prioritizing hardware-level offloading to keep the CPU free for logic and UI rendering.

* **Cooperative Main Loop & Interrupts:** Core sensor sampling is driven by External Interrupts (EXTI), Timers, and Direct Memory Access (DMA). The main loop handles state machine logic, UI rendering, and SD card logging.
* **Non-Blocking Debouncing:** Mechanical button bounces are mitigated using a SysTick-based time-delay verification within the EXTI callback routine, ensuring zero CPU blocking.
* **Menu Finite State Machine (FSM):** The OLED interface is managed via a node-based FSM, allowing seamless traversal between sensor readouts, data-entry forms, and system diagnostic screens.
* **UART Command Interface:** A secondary USART line (57600 baud) allows a host PC to request system status, dump SD card logs, or manually override alarm conditions using a custom `@command&` protocol.

## Pinout Summary

| Pin | Peripheral | Connected To |
| :--- | :--- | :--- |
| **PA0** | ADC1 IN0 | Light sensor op-amp output |
| **PA1** | EXTI1 | Accelerometer INT pin |
| **PA2** | USART2 TX | ST-Link |
| **PA3** | USART2 RX | ST-Link |
| **PA6** | TIM3 CH1 PWM | LED D4 |
| **PA7** | TIM3 CH2 PWM | LED D5 |
| **PA8** | EXTI8 | Button S1 |
| **PA9** | TIM1 CH2 PWM | LED D2 |
| **PA10** | EXTI10 | Button S2 |
| **PA11** | GPIO OUT | Keypad column 1 |
| **PA15** | TIM2 ETR | Temperature sensor output |
| **PB1** | TIM3 CH4 PWM | LED D3 |
| **PB2** | GPIO OUT | Keypad column 0 |
| **PB3** | EXTI3 | Button S3 |
| **PB4** | EXTI4 | Button S4 |
| **PB7** | TIM4 CH2 IC | HC-SR04 Echo input |
| **PB8** | I2C1 SCL | OLED SSD1306 / Accelerometer SCL |
| **PB9** | I2C1 SDA | OLED SSD1306 / Accelerometer SDA |
| **PB10** | SPI2 SCK | SD card module CLK |
| **PB12** | EXTI12 | Keypad row 0 |
| **PB13** | EXTI13 | Keypad row 1 |
| **PB14** | EXTI14 | Keypad row 2 |
| **PB15** | EXTI15 | Keypad row 3 |
| **PC2** | SPI2 MISO | SD card module MISO |
| **PC3** | SPI2 MOSI | SD card module MOSI |
| **PC4** | GPIO OUT | Keypad column 2 |
| **PC5** | GPIO OUT | SD card module CS |
| **PC6** | USART6 TX | GPS NEO-6M RX |
| **PC7** | USART6 RX | GPS NEO-6M TX |
| **PC10** | GPIO OUT | HC-SR04 Trigger output |
| **E5V** | Power Input | 5V |


## Hardware Schematic

<img width="630" height="433" alt="SCHEMATIC" src="https://github.com/user-attachments/assets/2cc595e5-6539-44a9-8aa6-61e71eca1ece" />

### Full Documentation
For an in-depth breakdown of the hardware calculations, component selection, and software flowcharts, please refer to the complete **[E-DAS Design Report](link_to_your_pdf.pdf)** included in this repository.
