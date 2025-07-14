# Electronic Irrigation System

An intelligent automated plant watering system designed for indoor gardening using Microchip SAMD21 microcontroller technology. This hobby project combines embedded systems programming, sensor integration, and user interface design to create a comprehensive plant care solution.

## 🌱 Project Overview

This system monitors soil moisture and temperature conditions, then automatically waters plants based on configurable schedules and sensor thresholds. The project emphasizes modularity and expandability, starting with a development board prototype and evolving toward a custom PCB implementation with wireless connectivity.

## ✨ Features

### Core Functionality
- **Dual Watering Modes**: Time-based scheduling and sensor-triggered watering
- **Real-time Monitoring**: Continuous soil moisture and temperature sensing
- **Manual Override**: Instant watering control and emergency stop
- **User-Configurable**: Customizable watering schedules and sensor thresholds
- **Status Feedback**: Visual indicators and system health monitoring

### Technical Capabilities
- Analog sensor reading with calibration
- Power-efficient operation for battery use
- Modular hardware design for easy expansion
- Cross-platform GUI application

## 🔧 Hardware Components

### Current Implementation
- **Microcontroller**: SAMD21 Curiosity development board
- **Moisture Sensor**: TL555-based analog sensor (3.3-5.5V)
- **Temperature Sensor**: DHT22/DS18B20 (TBD)
- **Pump Control**: Motor driver circuit for water pump
- **Power Supply**: 3.3V/5V compatible design

### Future Enhancements
- Wi-Fi connectivity module (ATWINC1500/ESP8266)
- Solar power capability for outdoor use

## 💻 Software Architecture

### Firmware (C Language)
- **Development Environment**: MPLAB X IDE
- **Real-time sensor data acquisition**
- **Watering algorithm implementation**
- **Communication protocol for GUI interface**
- **Power management routines**

### GUI Application (Python)
- **Framework**: Tkinter for cross-platform compatibility
- **Real-time data visualization**
- **Schedule configuration interface**
- **Manual control panel**
- **System status monitoring**

## 📁 Project Structure

```
irrigation-system/
├── firmware/
│   ├── src/
│   ├── include/
│   └── config/
├── gui/
│   ├── main.py
│   ├── ui/
│   └── utils/
├── hardware/
│   ├── schematics/
│   ├── pcb/
│   └── docs/
├── docs/
└── README.md
```

## 🚀 Getting Started

### Prerequisites
- MPLAB X IDE (latest version)
- Python 3.8+ with tkinter
- SAMD21 development board
- Electronic components (see hardware list)

### Installation
1. Clone the repository
2. Set up MPLAB X project with SAMD21 configuration

### Usage
1. Flash firmware to SAMD21 board
2. Run GUI application: `python gui/main.py`(TBD)
3. Calibrate sensors using the GUI interface
4. Configure watering schedule and thresholds
5. Monitor system operation and plant health

## 🔄 Development Phases

- [x] **Phase 1**: Research & Planning
- [ ] **Phase 2**: Initial Prototype Development
- [ ] **Phase 3**: Testing & Refinement
- [ ] **Phase 4**: PCB Design & Implementation
- [ ] **Phase 5**: GUI Development
- [ ] **Phase 6**: Final Integration & Testing

## 🛠️ Technical Details

### Sensor Specifications
- **Moisture Sensor**: TL555 chip, 0-3.0V output, 2.54mm pitch
- **Temperature Range**: -10°C to +50°C (indoor use)
- **Sampling Rate**: Configurable (default: 1 minute)
- **Calibration**: Multi-point calibration for different soil types

### Communication Protocol
- Serial communication between microcontroller and GUI
- JSON-based message format for data exchange
- Error handling and connection recovery

## 🌐 Future Roadmap

### Phase 2 Enhancements
- [ ] Wi-Fi connectivity implementation
- [ ] Remote monitoring via web interface
- [ ] Cloud data logging and analytics
- [ ] Mobile app development

### Outdoor Adaptation
- [ ] Weather API integration
- [ ] Rain sensor integration
- [ ] Solar power system
- [ ] Weatherproof enclosure design

## 📊 Performance Metrics

- **Power Consumption**: <100mA average (target)
- **Sensor Accuracy**: ±2% moisture, ±0.5°C temperature
- **Response Time**: <5 seconds for manual override
- **Battery Life**: 2-3 weeks (estimated, 3000mAh battery)

## 🤝 Contributing

This is a personal hobby project, but suggestions and improvements are welcome! Feel free to:
- Report bugs or issues
- Suggest new features
- Share improvements or optimizations
- Contribute to documentation

## 📝 Documentation


## 🏷️ Version History

- **v0.1.0** (Planned): Initial prototype release
- **v0.2.0** (Future): PCB implementation
- **v1.0.0** (Future): Full feature release with Wi-Fi

## 👤 Author

Created as a personal hobby project for automated indoor plant care.

---

**Status**: 🚧 In Development | **Started**: March 2025 | **Estimated Completion**: May 2025
