ESP32-S3-EdgeAI-EV-Analytics


🚗 Edge AI System for EV Trip Analytics:
On-Device WLTP Class 3B Processing and
Battery Temperature Prediction

This project presents a high-performance edge computing framework developed on the esp32-s3 microcontroller. It is designed to process electric vehicle telemetry at 1 hz to provide immediate feedback on driving efficiency and battery safety without relying on cloud servers.





🌟 Key Technical Features
Dual-Core Processing: The system uses core 0 specifically for background network tasks like wifi management and mqtt publishing. Core 1 is dedicated to high-speed physics calculations and machine learning inference.



Predictive Thermal Management: Features a tinyml neural network that predicts battery pack temperature 120 seconds into the future.



High Precision: The AI model is extremely accurate with a mean absolute error of only 0.09 degrees celsius.



Real-Time Dashboard: Sends data summaries in json format via mqtt to a node-red dashboard for driver visualization.





📊 Research Methodology and Data
Standardized Input: The system is tested using the wltp class 3b driving cycle, which is the modern european standard for speed-time profiles.



1 Hz Telemetry Engine: Processes 10 key parameters every second, including vehicle speed, acceleration, road slope, and battery current.



On-Device Calculations: Locally computes energy consumption in wh/km, regenerative braking efficiency, and coasting performance.



Robust Testing: Successfully delivered 1801/1801 messages during testing with zero data loss.


🛠️ Tech Stack and Analytics
Firmware: Written in c++ using the arduino framework and esp-idf multitasking.


Machine Learning: Developed with edge impulse and executed using tensorflow lite micro.


Data Protocols: Uses mqtt for telemetry and littlefs for on-device csv data storage.



Scoring Logic: Calculates a weighted driver behavior score based on energy (35%), regeneration (25%), temperature (15%), current (15%), and coasting (10%) .



📂 Repository Organization
/EV_EdgeAI_Main: Contains the main arduino firmware and the full edge impulse sdk folders.

/Node-RED: Includes the flows.json file required to set up the vehicle analytics dashboard.

/Data: Contains the wltp datasets used for inference and the files used for model training.


Developed as part of a Master's Thesis project at Lappeenranta-Lahti University of Technology (LUT), Finland.
