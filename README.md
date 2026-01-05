# forklift-downtime-monitoring
# Forklift Downtime Monitoring System

An IoT-based forklift tracking and downtime monitoring system designed to improve operational visibility and efficiency in industrial warehouse environments.

This project focuses on identifying non-productive time, analyzing forklift utilization, and supporting data-driven operational decisions through real-time monitoring and historical analysis.

---

## Project Background

In industrial warehouses, forklifts are critical assets. However, limited visibility into their operational status often leads to:
- Inefficient utilization
- Hidden downtime
- Difficulty distinguishing productive vs. non-productive states

This project addresses these challenges by introducing a lightweight IoT-based tracking system that captures forklift status data and transforms it into actionable operational insights.

---

## System Overview

The system monitors forklift operational states and provides both real-time and historical views of utilization and downtime.

**Key concepts include:**
- Tracking forklift status (active, idle, parked)
- Measuring and categorizing downtime
- Visualizing operational performance through dashboards

---

## Architecture Overview

The system follows a modular architecture:

- **IoT Layer**
  - Forklift-mounted tracking devices (e.g. BLE / sensor-based tags)
  - Periodic status broadcasting

- **Data Ingestion Layer**
  - Gateway or anchor devices
  - Data preprocessing and filtering
  - Message-based data transmission

- **Processing & Logic Layer**
  - Status classification logic
  - Downtime calculation rules
  - Priority-based downtime handling

- **Visualization Layer**
  - Real-time status dashboards
  - Historical utilization and downtime analysis

---

## Core Features

- Real-time forklift status monitoring
- Downtime detection and categorization
- Utilization analysis based on operational states
- Dashboard-based visualization for management
- Separation of productive vs. non-productive time
- Support for multiple forklifts and asset categories

---

## Downtime Logic (Conceptual)

Downtime is not treated as a single static state.  
Instead, the system distinguishes between different non-productive conditions, such as:

- Parked but available
- Idle without task assignment
- Priority downtime triggered by operational context

This logic allows more accurate interpretation of forklift usage patterns compared to simple on/off tracking.

---

## Scalability & Extensibility

The system is designed with scalability in mind:

### Horizontal Scalability
- Supports multiple forklifts by replicating virtual entities
- Each forklift is represented independently in the data model
- New forklifts can be added without redesigning core logic

### Fleet Categorization
- Different forklift models (e.g. electric, reach trucks, stackers) can be categorized
- Model-specific attributes and rules can be applied
- Enables comparative analysis across forklift types

### Logic Extension
- Downtime rules can be refined or extended
- Additional operational states can be introduced
- Priority-based logic can be adapted to different warehouse workflows

### Integration Potential
- Can be integrated with:
  - Warehouse Management Systems (WMS)
  - Maintenance planning systems
  - Energy or sustainability analytics platforms

---

## Use Cases

- Warehouse operations optimization
- Fleet utilization analysis
- Identification of hidden inefficiencies
- Decision support for asset allocation
- Input data for sustainability and energy efficiency initiatives

---

## Project Status

This repository represents a functional prototype and concept validation based on real operational requirements.

Future improvements may include:
- Advanced analytics and forecasting
- Integration with external enterprise systems
- Enhanced visualization and reporting
- Machine learning–based pattern detection

---

## Author

Developed by **Duxing Li**  
ICT (Information & Communication Technology)  
IoT & Energy Systems

---

## License

MIT License

