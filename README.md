# Spresense Multi-IMU Add-on Board Test

A development playground for Sony Spresense's new Multi-IMU Add-on Board.

## Overview

The Sony Spresense is a compact development board with Sony's CXD5602 microcontroller. The Multi-IMU Add-on Board extends its capabilities with motion sensing.

## Project Structure

```
├── Arduino IDE
│   ├── Spresense_IMU_Angle - Calculates angles and controls servo
│   ├── Spresense_IMU_Raw - Outputs raw IMU data in CSV format
│   └── Spresense_IMU_ROS - Publishes data to ROS topics
├── Node-RED
│   ├── flows - Example flows for WSL Ubuntu and ROS integration
│   └── images - Screenshots and documentation
└── ROS
    ├── images - Visualization screenshots
    └── README.md - ROS integration and USB setup instructions
```

## Features

- Raw IMU data acquisition
- Angle calculation using complementary filter
- Servo control based on IMU angles
- ROS integration through rosserial
- Node-RED visualization with WSL Ubuntu integration

## Sketches

### Spresense_IMU_Angle
Calculates orientation angles and controls a servo motor based on the X-axis angle.

### Spresense_IMU_Raw
Outputs raw sensor data in fixed-width CSV format for analysis.

### Spresense_IMU_ROS
Publishes IMU data to the ROS topic `/imu/data_raw`.

## Node-RED Integration

For WSL Ubuntu integration details, see the [Node-RED README](Node-RED/README.md).

## ROS Integration

Connects to ROS using rosserial. Required packages:

```bash
sudo apt install ros-noetic-rosserial-python ros-noetic-rosserial-arduino
```

For USB connection setup and other details, see the [ROS README](ROS/README.md).

## License

MIT License - See the LICENSE file for details.
