# 🚗 Edge AI System for EV Trip Analytics: On-Device WLTP Class 3B Processing and Battery Temperature Prediction

This project presents a high-performance edge computing framework developed on the esp32-s3 microcontroller. It is designed to process electric vehicle telemetry at 1 Hz to provide immediate feedback on driving efficiency and battery safety without relying on cloud servers.

![System Architecture](images/WLTP_Block_Diagram.png)

## 🌟 Key Technical Features

* Dual-Core Architecture: Core 0 handles wifi connectivity and asynchronous mqtt telemetry. Core 1 executes real-time physics calculations and tinyml inference.
* Predictive Thermal Management: Features a neural network that predicts battery pack temperature 120 seconds into the future.
* High Precision: The ai model is extremely accurate with a mean absolute error (mae) of 0.09 degrees celsius.
* Real-Time Dashboard: Sends data summaries in json format via mqtt to a node-red dashboard for driver visualization.

![Real-time Vehicle Dashboard](images/WLTP_Dasboard.jpg)

## 📊 Research Methodology and Data

* Standardized Input: The system is tested using the WLTP Class 3b driving cycle, which represents urban, suburban, and highway phases.
* 1 Hz Telemetry Engine: Processes 10 key parameters every second, including speed, acceleration, and battery current.
* On-Device Event Detection: Detects coasting segments based on near-zero acceleration and current thresholds.

![WLTP Speed and Coasting Points](images/WLTP_Coasting.png)

## 📈 Analytics and Performance Results

* Energy Consumption: Calculates instantaneous and cumulative net energy, matching offline matlab analysis.
* Thermal Accuracy: On-device inference matches actual battery trends throughout the 30-minute WLTP Class 3b trip.
* System Integrity: Lossless 1 Hz telemetry delivery confirmed with 1801 out of 1801 messages received successfully.

![Energy Consumption Trends](images/WLTP_Cons.png)
![Battery Temperature Prediction Accuracy](images/WLTP_Temp_Pred.png)
![Telemetry Integrity Verification](images/WLTP_1801.jpg)

## 🛠️ Tech Stack

* Frameworks: ESP-IDF (task multitasking) and Arduino.
* AI and ML: Edge Impulse (tinyml).
* Protocols: MQTT (telemetry), JSON (data format), and LittleFS (on-device storage)
* Scoring Logic: Weighted score based on energy (35%), regeneration (25%), temperature (15%), current (15%), and coasting (10%).

## 📂 Repository Organization

* /EV_EdgeAI_Main: Contains the main arduino firmware and integrated edge impulse sdk folders.
* /Node-RED: Includes the flows.json file required to set up the vehicle analytics dashboard.
* /Data: Contains the WLTP datasets and training csv files.
* /images: Technical graphs and dashboard screenshots from the research paper.

---
Developed as part of a Master's Thesis project at Lappeenranta-Lahti University of Technology (LUT), Finland.
