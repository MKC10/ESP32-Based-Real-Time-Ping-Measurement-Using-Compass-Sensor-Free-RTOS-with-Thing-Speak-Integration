# 🌐 FreeRTOS-Based Real-Time Ping & Orientation Measurement System

![ESP32 RTOS](docs/images/esp32_rtos_banner.jpg)

> Real-time network latency monitoring combined with 3-axis compass orientation tracking using FreeRTOS multi-tasking on ESP32

[![Platform](https://img.shields.io/badge/platform-ESP32-blue)]()
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)]()
[![Cloud](https://img.shields.io/badge/cloud-ThingSpeak-orange)]()
[![License](https://img.shields.io/badge/license-MIT-brightgreen)]()

---

## 📺 Demo

![System Dashboard](docs/images/thingspeak_dashboard.png)
*Real-time visualization of ping latency and compass orientation on ThingSpeak*

---

## 🎯 What Does It Do?

This embedded system monitors network connectivity and device orientation in real-time using FreeRTOS multi-tasking:

- 🌐 **Network Monitoring**: Pings Google's servers to measure latency (round-trip time)
- 🧭 **Orientation Tracking**: Reads 3-axis magnetic field data from QMC5883L compass
- ☁️ **Cloud Integration**: Uploads data to ThingSpeak for real-time visualization
- ⚡ **Real-Time Processing**: Uses FreeRTOS for deterministic, concurrent task execution
- 🔒 **Thread-Safe**: Semaphore-based synchronization prevents data races

**Why Real-Time Matters:**
Unlike traditional single-threaded Arduino sketches, this system uses FreeRTOS to:
- Execute multiple tasks concurrently with guaranteed timing
- Ensure network monitoring doesn't block sensor readings
- Provide deterministic response times critical for real-time applications

---

## 🧠 Problem Statement

**Traditional Approach Limitations:**
```cpp
void loop() {
    readCompass();      // Blocks for ~100ms
    pingNetwork();      // Blocks for ~500ms  
    uploadToCloud();    // Blocks for ~2000ms
}
// Total cycle time: ~2.6 seconds (no parallelism)
```

**FreeRTOS Solution:**
```cpp
// Task 1: Compass reading every 3s
// Task 2: Network ping every 3s
// Task 3: Cloud upload every 30s
// All running concurrently with precise timing control
```

**Applications:**
- 📡 **IoT Network Monitoring**: Track connectivity quality in remote deployments
- 🚁 **Drone Navigation**: Monitor both network link and orientation
- 🏗️ **Industrial IoT**: Detect equipment tilt/orientation + network health
- 🔬 **Research**: Study network latency patterns with position correlation
- 🎓 **Education**: Learn real-time operating systems and multi-tasking

---

## 🛠️ Hardware Setup

### System Components

| Component | Specification | Purpose |
|-----------|---------------|---------|
| **Microcontroller** | ESP32 DevKit | Dual-core, WiFi-enabled, FreeRTOS support |
| **Compass Module** | QMC5883L | 3-axis magnetometer (I2C interface) |
| **Communication** | WiFi 802.11 b/g/n | Network ping & cloud upload |
| **Power Supply** | 5V USB or 3.7V Li-Po | Portable or bench operation |
| **RTOS** | FreeRTOS | Built into ESP32 Arduino core |

### Pin Connections

```
ESP32 ──────► QMC5883L Compass
GPIO 25 ───── SDA (I2C Data)
GPIO 26 ───── SCL (I2C Clock)
3.3V   ────── VCC
GND    ────── GND
```

### System Architecture

```
┌─────────────────────────────────────────────────┐
│              ESP32 Dual-Core                    │
│  ┌──────────────┐         ┌──────────────┐     │
│  │   Core 0     │         │   Core 1     │     │
│  │  (WiFi Stack)│         │  (FreeRTOS)  │     │
│  └──────────────┘         └──────────────┘     │
└─────────────────────────────────────────────────┘
         │                           │
         ↓                           ↓
┌─────────────────┐         ┌─────────────────┐
│  WiFi Network   │         │  I2C Sensors    │
│  ├─ Ping Task   │         │  └─ Compass     │
│  └─ ThingSpeak  │         └─────────────────┘
└─────────────────┘

         FreeRTOS Scheduler
    ┌──────────────────────────┐
    │  Task 1: pingTask (3s)   │ ──┐
    │  Task 2: CompTask (3s)   │   │── Semaphore-protected
    │  Task 3: ThingSpeak(30s) │ ──┘    shared resources
    └──────────────────────────┘
```

### Hardware Photo

![ESP32 + Compass Setup](docs/hardware/esp32_compass_setup.jpg)

---

## 💻 Software Architecture

### Technology Stack

**Embedded Framework:**
- ESP32 Arduino Core (with FreeRTOS)
- C/C++ programming
- Real-time multi-tasking

**Libraries:**
- `WiFi.h` - WiFi connectivity
- `ESPping.h` - ICMP ping functionality  
- `ThingSpeak.h` - Cloud IoT platform
- `QMC5883LCompass.h` - Magnetometer driver

**RTOS Components:**
- FreeRTOS kernel (built into ESP32)
- Binary semaphore for synchronization
- Task scheduler with priority management

### FreeRTOS Task Design

```
┌────────────────────────────────────────────────────────┐
│                   Task Architecture                    │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Task 1: pingTask                                      │
│  ├─ Priority: 1                                        │
│  ├─ Stack: 2048 bytes                                  │
│  ├─ Period: 3 seconds (vTaskDelay)                     │
│  └─ Function: Ping google.com, measure RTT             │
│                                                        │
│  Task 2: CompTask                                      │
│  ├─ Priority: 1                                        │
│  ├─ Stack: 2048 bytes                                  │
│  ├─ Period: 3 seconds (vTaskDelay)                     │
│  └─ Function: Read QMC5883L (X, Y, Z)                  │
│                                                        │
│  Task 3: thingSpeakTask                                │
│  ├─ Priority: 1                                        │
│  ├─ Stack: 2048 bytes                                  │
│  ├─ Period: 30 seconds (timer-based)                   │
│  └─ Function: Upload to ThingSpeak cloud               │
│                                                        │
│  Synchronization: Binary Semaphore                     │
│  └─ Protects: avg_time_ms, CompassData struct          │
└────────────────────────────────────────────────────────┘
```

### Data Flow

```
┌─────────────┐
│  pingTask   │  Measure RTT
└──────┬──────┘
       │ Lock semaphore
       ↓
┌──────────────┐
│ avg_time_ms  │  Shared variable
└──────┬───────┘
       │ Unlock semaphore
       ↓

┌─────────────┐
│  CompTask   │  Read sensor
└──────┬──────┘
       │ Lock semaphore
       ↓
┌──────────────────┐
│ CompassData {    │  Shared struct
│   x, y, z        │
│ }                │
└──────┬───────────┘
       │ Unlock semaphore
       ↓

┌──────────────────┐
│ thingSpeakTask   │  Every 30s
├──────────────────┤
│ Lock semaphore   │
│ Read all data    │
│ Unlock semaphore │
│ Upload to cloud  │
└──────────────────┘
       ↓
┌──────────────────┐
│   ThingSpeak     │
│   Dashboard      │
│  Field 1: Ping   │
│  Field 2: X      │
│  Field 3: Y      │
│  Field 4: Z      │
└──────────────────┘
```

---

## 📊 FreeRTOS Concepts Demonstrated

### 1. Task Creation (xTaskCreate)

```cpp
// Create three concurrent tasks
xTaskCreate(CompTask, "CompTask", 2048, NULL, 1, NULL);
xTaskCreate(pingtask, "PINGTASK", 2048, NULL, 1, NULL);
xTaskCreate(thingSpeaktask, "thingSpeaktask", 2048, NULL, 1, NULL);

// Parameters explained:
// - Function pointer
// - Task name (for debugging)
// - Stack size in words (2048 = 8KB)
// - Task parameters (NULL = none)
// - Priority (1 = same for all)
// - Task handle (NULL = not stored)
```

### 2. Task Handle (TaskHandle_t)

```cpp
TaskHandle_t pingtaskHandle;  // Stores task control block

// Enables task control operations like:
// - vTaskSuspend(pingtaskHandle)
// - vTaskResume(pingtaskHandle)
// - vTaskDelete(pingtaskHandle)
```

### 3. Task Delays (vTaskDelay)

```cpp
vTaskDelay(pdMS_TO_TICKS(3000));  // Delay for 3 seconds

// Benefits:
// - Task enters BLOCKED state (doesn't consume CPU)
// - Other tasks can run
// - Precise timing (not polling-based)
```

### 4. Semaphores (Binary)

```cpp
SemaphoreHandle_t semaphore;

// In setup():
semaphore = xSemaphoreCreateBinary();
xSemaphoreGive(semaphore);  // Initialize as available

// In tasks:
xSemaphoreTake(semaphore, portMAX_DELAY);  // Lock (wait forever if taken)
// ... critical section (access shared data) ...
xSemaphoreGive(semaphore);  // Unlock

// Prevents race conditions when multiple tasks access:
// - avg_time_ms (ping data)
// - CompassData struct (sensor readings)
```

---

## 🚀 Getting Started

### Prerequisites

**Hardware:**
- ESP32 development board
- QMC5883L compass module
- USB cable
- Jumper wires
- WiFi network access

**Software:**
```bash
# Arduino IDE 2.x or PlatformIO
# ESP32 board support: https://github.com/espressif/arduino-esp32

# Required Libraries (install via Library Manager):
- ESP32 board package
- ThingSpeak by MathWorks
- QMC5883LCompass by MPrograms
- ESPping (built-in with ESP32 core)
```

### Installation Steps

**1. Install ESP32 Board Support:**
```
Arduino IDE → Preferences → Additional Board Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Tools → Board Manager → Search "ESP32" → Install
```

**2. Install Libraries:**
```
Sketch → Include Library → Manage Libraries

Search and install:
- "ThingSpeak" by MathWorks
- "QMC5883LCompass" by MPrograms
```

**3. Create ThingSpeak Account:**
```
1. Sign up at https://thingspeak.com
2. Create new channel with 4 fields:
   - Field 1: Ping (ms)
   - Field 2: Compass X
   - Field 3: Compass Y
   - Field 4: Compass Z
3. Get Channel ID and Write API Key
```

**4. Configure Code:**
```cpp
// Update WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourPassword";

// Update ThingSpeak credentials
unsigned long myChannelID = 123456;  // Your channel ID
const char* myWriteAPIKey = "XXXXXXXXXXXXXXXX";  // Your API key
```

**5. Upload to ESP32:**
```
Tools → Board → ESP32 Dev Module
Tools → Port → [Select your COM port]
Upload button (→)
```

**6. Monitor Output:**
```
Tools → Serial Monitor (115200 baud)

Expected output:
Connecting to WiFi: YourSSID
....
Connected, IP address: 192.168.1.100
Ping to gateway: 25 ms
X= -123.45
Y= 67.89
Z= -45.67
Successfully sent to ThingSpeak channel.
```

---

## 📁 Project Structure

```
esp32-rtos-ping-compass/
│
├── 📄 README.md
├── 📄 LICENSE
│
├── 📁 src/
│   └── main.ino                    # Main firmware
│
├── 📁 docs/
│   ├── hardware/
│   │   ├── circuit_diagram.png
│   │   ├── esp32_pinout.jpg
│   │   └── compass_wiring.jpg
│   ├── images/
│   │   ├── thingspeak_dashboard.png
│   │   └── serial_output.png
│   └── datasheets/
│       ├── ESP32_datasheet.pdf
│       └── QMC5883L_datasheet.pdf
│
├── 📁 examples/
│   ├── basic_ping_test.ino
│   ├── compass_calibration.ino
│   └── thingspeak_test.ino
│
└── 📄 platformio.ini               # For PlatformIO users
```

---

## 🧪 Testing & Validation

### Serial Monitor Output

```
Connecting to WiFi: OnePlus34
.....
Connected, IP address: 192.168.1.105

[Task: pingTask]
Ping success at start
Ping to gateway: 23 ms

[Task: CompTask]
X= -145.23
Y= 234.56
Z= -89.45

[Task: thingSpeakTask - every 30s]
Successfully sent to ThingSpeak channel.

[Continuous monitoring...]
Ping to gateway: 25 ms
X= -143.12
Y= 235.67
Z= -88.90
```

### ThingSpeak Dashboard

**Field 1 - Ping Latency:**
```
┌────────────────────────────┐
│  Ping Response Time (ms)   │
│                            │
│  30 ┤                       │
│  25 ┤  ●─●─●                │
│  20 ┤        ●─●            │
│  15 ┤                       │
│     └───────────────────────│
│      0   10   20   30   40  │
│           Time (minutes)    │
└────────────────────────────┘
```

**Fields 2-4 - Compass Orientation:**
```
Real-time 3D visualization of magnetic field vector
```

---

## ⚙️ Configuration Options

### Adjust Task Periods

```cpp
// In pingtask:
vTaskDelay(pdMS_TO_TICKS(3000));  // Change 3000 to desired delay (ms)

// In CompTask:
vTaskDelay(3000);  // Change to desired delay (ms)

// In thingSpeakTask:
unsigned long timerDelay = 30000;  // Change to upload interval (ms)
```

### Change Ping Target

```cpp
// Ping different server:
bool pingBool = Ping.ping("8.8.8.8");  // Google DNS
// or
bool pingBool = Ping.ping("192.168.1.1");  // Your router
```

### Adjust Task Priorities

```cpp
// Give different priorities (1-5, higher = more important)
xTaskCreate(CompTask, "CompTask", 2048, NULL, 2, NULL);        // Higher
xTaskCreate(pingtask, "PINGTASK", 2048, NULL, 3, NULL);        // Highest
xTaskCreate(thingSpeakTask, "thingSpeakTask", 2048, NULL, 1, NULL); // Lower
```

### I2C Pin Configuration

```cpp
// Default pins 25 (SDA), 26 (SCL)
Wire.begin(25, 26);

// Change if needed:
Wire.begin(21, 22);  // Common alternative pins
```

---

## 📈 Performance Metrics

| Metric | Value | Description |
|--------|-------|-------------|
| **Ping Interval** | 3 seconds | How often network is tested |
| **Compass Interval** | 3 seconds | Sensor reading frequency |
| **Upload Interval** | 30 seconds | ThingSpeak update rate |
| **Typical Ping RTT** | 15-30 ms | To google.com (varies by ISP) |
| **Task Overhead** | <5% CPU | FreeRTOS scheduler efficiency |
| **Memory Usage** | ~3x2KB = 6KB | Total task stack allocation |
| **Power Consumption** | ~160mA @ 3.3V | WiFi active mode |

---

## 🎓 What I Learned

### Real-Time Operating Systems (RTOS)

✅ **Task Management**: Creating, scheduling, and managing concurrent tasks  
✅ **Synchronization**: Using semaphores to protect shared resources  
✅ **Timing Control**: Precise task delays and periodic execution  
✅ **Resource Allocation**: Stack size management and priority assignment  

### Embedded Systems Skills

✅ **Multi-Tasking**: Running network, sensor, and cloud tasks concurrently  
✅ **I2C Communication**: Reading magnetometer sensor data  
✅ **WiFi Networking**: ESP32 WiFi stack and ICMP ping implementation  
✅ **Cloud Integration**: IoT data upload to ThingSpeak platform  

### Software Engineering

✅ **Thread Safety**: Avoiding race conditions with semaphores  
✅ **Code Organization**: Separating concerns into independent tasks  
✅ **Error Handling**: Checking return codes and connection status  
✅ **Debugging**: Using Serial Monitor for real-time diagnostics  

### Challenges Overcome

**Challenge 1: Task Synchronization**  
❌ Problem: Multiple tasks accessing shared variables caused data corruption  
✅ Solution: Implemented binary semaphore to protect critical sections

**Challenge 2: WiFi Stack Blocking**  
❌ Problem: Network operations blocked sensor readings  
✅ Solution: Separated concerns into independent tasks with FreeRTOS

**Challenge 3: ThingSpeak Rate Limiting**  
❌ Problem: Free tier allows only 1 update per 15 seconds  
✅ Solution: Set upload interval to 30 seconds to stay within limits

**Challenge 4: I2C Pin Conflicts**  
❌ Problem: Default I2C pins conflicted with other peripherals  
✅ Solution: Remapped to pins 25/26 using Wire.begin(25, 26)

---

## 🔮 Future Improvements

### Short-Term Enhancements

- [ ] Add OLED display for local visualization (no cloud dependency)
- [ ] Implement compass calibration routine (hard/soft iron correction)
- [ ] Add WiFi reconnection logic for better reliability
- [ ] Log data to SD card as backup when cloud unavailable
- [ ] Add LED indicators for task status (ping success/fail, compass ready)

### Advanced Features

- [ ] **Multi-Server Ping**: Monitor latency to multiple targets simultaneously
- [ ] **GPS Integration**: Correlate location with ping quality (network coverage map)
- [ ] **Predictive Analytics**: Detect network degradation trends
- [ ] **3D Orientation Visualization**: Full Euler angles (roll, pitch, yaw)
- [ ] **Mobile App**: Real-time monitoring via Bluetooth or local web server
- [ ] **Deep Sleep Mode**: Battery optimization with periodic wake-up

### Research Extensions

- [ ] **Network Quality Mapping**: Drive around and map WiFi/LTE coverage with GPS
- [ ] **Compass Calibration Algorithm**: Auto-detect and compensate for magnetic interference
- [ ] **FreeRTOS Advanced**: Queues, mutexes, software timers, task notifications
- [ ] **Edge ML**: On-device anomaly detection for network outages

---

## 📚 Technical Background

### FreeRTOS on ESP32

ESP32 uses a **dual-core** architecture:
- **Core 0**: Dedicated to WiFi stack (managed by Espressif)
- **Core 1**: Available for user tasks (FreeRTOS scheduler)

Tasks created with `xTaskCreate()` run on **Core 1** by default. Use `xTaskCreatePinnedToCore()` for explicit core assignment.

### QMC5883L Magnetometer

3-axis compass sensor measuring Earth's magnetic field:
- **Range**: ±2 gauss (typical)
- **Resolution**: ~0.1 µT
- **Interface**: I2C (0x0D address)
- **Update Rate**: Up to 200Hz

**Applications:**
- Navigation (heading calculation)
- Tilt-compensated compass
- Metal detection
- Magnetic field mapping

### ThingSpeak IoT Platform

Cloud platform for IoT data visualization:
- **Free Tier**: 3 million messages/year
- **Rate Limit**: 1 update every 15 seconds
- **Channels**: Up to 4 public + 4 private
- **Fields**: 8 data fields per channel
- **MATLAB**: Built-in analytics and processing

---

## 🤝 Applications & Use Cases

### 1. Network Quality Monitoring
Deploy multiple devices across a building to create a **WiFi coverage heatmap** with real-time latency data.

### 2. IoT Reliability Testing
Monitor long-term network stability for critical IoT deployments (medical devices, industrial sensors).

### 3. Navigation Systems
Combine compass orientation with GPS for **tilt-compensated** navigation (drones, robots).

### 4. STEM Education
Teach students about:
- Real-time operating systems
- Multi-tasking concepts
- Sensor integration
- Cloud IoT platforms

### 5. Research Projects
- Study correlation between physical orientation and network performance
- Investigate magnetic interference from WiFi routers
- Benchmark ESP32 task switching overhead

---

## 📝 License

MIT License - See [LICENSE](LICENSE) file

---

## 👤 Author

**[Your Name]**
- 🎓 Electrical/Computer Engineering Student @ UAB
- 💼 Interested in: Embedded Systems, RTOS, IoT, Firmware Development
- 🔗 GitHub: [@yourusername](https://github.com/yourusername)
- 💼 LinkedIn: [linkedin.com/in/yourname](https://linkedin.com/in/yourname)
- 📧 Email: your.email@uab.edu

---

## 🙏 Acknowledgments

- **FreeRTOS**: Richard Barry and the FreeRTOS team for the RTOS kernel
- **Espressif**: ESP32 Arduino core and comprehensive documentation
- **MathWorks**: ThingSpeak IoT platform
- **Course Instructor**: [Instructor Name] for teaching Real-Time Systems concepts
- **Open Source Community**: Library maintainers and contributors

---

## 📚 References & Resources

### FreeRTOS Documentation
- [FreeRTOS API Reference](https://www.freertos.org/a00106.html)
- [ESP32 FreeRTOS Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html)

### Hardware Datasheets
- [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [QMC5883L Datasheet](https://datasheet.lcsc.com/lcsc/2004071501_QST-QMC5883L_C192585.pdf)

### Related Projects
- [ESP32 FreeRTOS Examples](https://github.com/espressif/esp-idf/tree/master/examples/system/freertos)
- [ThingSpeak Projects](https://thingspeak.com/projects)

---

## 📊 Citation

```bibtex
@misc{yourname2026rtos,
  title={FreeRTOS-Based Real-Time Ping and Orientation Measurement System},
  author={Your Name},
  year={2026},
  publisher={GitHub},
  note={Real-time network monitoring with compass orientation tracking}
}
```

---

## ⭐ Star This Project

If you found this project useful for learning FreeRTOS or embedded systems, please give it a star!

![GitHub stars](https://img.shields.io/github/stars/yourusername/esp32-rtos-ping-compass?style=social)

---

**Last Updated:** March 2026
