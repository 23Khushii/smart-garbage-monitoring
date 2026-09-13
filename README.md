# Smart Garbage Monitoring System

An IoT-based system that monitors garbage bin fill levels in real time using an ultrasonic sensor and NodeMCU (ESP8266), with live status and alerts pushed to the Blynk mobile app.

**Team:** Janvi Bajpai, Khushi Sahu
**Guidance:** Prof. Smita Paithankar — School of Electronics & Communication Engineering, MIT-WPU, Pune

---

## Overview

Traditional waste collection relies on fixed schedules regardless of actual bin fill level, leading to unnecessary collection trips or overflowing bins. This project uses an ultrasonic distance sensor to continuously measure fill level, processes it on a NodeMCU, and pushes real-time status and threshold-based alerts to a Blynk dashboard — enabling collection based on actual need rather than a fixed schedule.

## System Architecture

```text
Ultrasonic Sensor  →  NodeMCU (ESP8266)  →  Fill-Level Calculation  →  Blynk Server  →  User App
 (Distance reading)     (Data processing)      (0–100%, thresholds)     (Alerts)      (Live dashboard)
```

## What's Implemented

- **Real-time distance sensing** via ultrasonic sensor (trigger/echo), sampled every 2 seconds.
- **Fill-percentage calculation** from raw distance, calibrated to bin height, with reading validation to reject out-of-range/noisy sensor values.
- **Three-tier status logic** — Empty / Partially Filled / Near Full / Full — each with distinct handling.
- **De-duplicated alerting**: push notifications fire once per threshold crossing (not repeatedly), and automatically reset once the bin is emptied.
- **Live Blynk dashboard integration** — distance, fill %, status text, and a gauge widget update in real time.
- **Manual controls from the app** — force an immediate sensor reading, or manually reset alert state, both via Blynk virtual pin writes.
- **Physical indicator** — onboard LED lights up when the bin is full, independent of app connectivity.

## Hardware

| Component | Qty | Cost (INR) |
|---|---|---|
| NodeMCU ESP8266 | 1 | 270 |
| Ultrasonic Sensor (HC-SR04) | 1 | — |
| 2-Cell Battery Holder | 1 | 13 |
| Battery | 3 | 30 |
| Jumper Wires | 6 | 12 |

**Total (incl. GST):** ~₹501

## Software Stack

- **Firmware:** Arduino IDE (C++), ESP8266WiFi + Blynk libraries
- **IoT Platform:** Blynk (dashboard, push notifications, virtual pin I/O)

## Repository Structure

```
├── firmware/
│   └── smart_bin_monitor.ino     # Full sensor + Blynk integration logic
├── media/
│   ├── circuit_diagram.png
│   ├── block_diagram.png
│   └── demo.mp4 / demo.jpg       # If available
├── PBL_Report.pdf                # Full project report: components, working, budget
└── README.md
```

## Setup

1. Install the **ESP8266** board package and **Blynk** library in Arduino IDE.
2. In `smart_bin_monitor.ino`, replace the placeholders with your own credentials:
   ```cpp
   char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
   char ssid[] = "YOUR_WIFI_NAME";
   char pass[] = "YOUR_WIFI_PASSWORD";
   ```
3. Adjust `BIN_HEIGHT` and threshold constants to match your physical bin and desired sensitivity.
4. Wire the ultrasonic sensor's Trig/Echo to D1/D2, and flash to the NodeMCU.
5. Set up a Blynk app project with virtual pins V0–V6 matching the widgets described in the code comments.

## References

Full component details, circuit diagram, and working description in `PBL_Report.pdf`.
