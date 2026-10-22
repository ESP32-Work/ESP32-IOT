# Pyserial Serial Monitor

A minimal, two-way serial communication example between an ESP32 and a computer using
the Arduino serial monitor on the board side and Python's `pyserial` on the host side.

## Description

The ESP32 sketch sends a heartbeat message once per second and listens for
newline-terminated commands. The Python script connects to the board, prints everything
it receives, and sends a `ping` command every five seconds so the round-trip path is
exercised in both directions.

## Features

- Non-blocking serial input on the ESP32 (no `delay()` in the receive path)
- Fixed-size, bounds-checked command buffer
- Periodic heartbeat from the board
- Periodic `ping` from the host with an echoed reply
- Graceful handling of closed ports, missing devices, and `Ctrl+C`
- No non-standard dependencies beyond `pyserial`

## Prerequisites

### Hardware

- ESP32 development board
- USB cable

### Software

- Arduino IDE with the ESP32 board support package (to flash the sketch)
- Python 3.8 or newer

## Project Structure

```
Pyserial_Test/
├── pyserial/
│   └── pyserial.ino      # ESP32 firmware
├── serial_monitor.py     # Host-side serial monitor
├── requirements.txt      # Python dependencies
└── README.md
```

## Installation

```bash
# From the Pyserial_Test directory
python -m venv .venv

# Windows (PowerShell)
.\.venv\Scripts\Activate.ps1

# Linux / macOS
source .venv/bin/activate

pip install -r requirements.txt
```

## Uploading the Firmware

1. Connect the ESP32 to your computer.
2. Open `pyserial/pyserial.ino` in the Arduino IDE.
3. Select the correct board and port under **Tools**.
4. Upload the sketch.
5. Close the Arduino Serial Monitor before running the Python script, since only one
   program can hold the port at a time.

## Usage

Set the serial port in `serial_monitor.py`:

```python
PORT = "COM4"  # Windows: COM3, COM4, ...
               # Linux:   /dev/ttyUSB0 or /dev/ttyACM0
               # macOS:   /dev/cu.usbserial-*
```

Then run:

```bash
python serial_monitor.py
```

Expected output:

```text
Connected to COM4 at 115200 baud
Sent: ping
Received: ESP32 received: ping
Received: Hello from the microcontroller!
Received: Hello from the microcontroller!
...
```

Press `Ctrl+C` to stop; the port is closed cleanly.

## Serial Protocol

| Direction | Message | Description |
| --- | --- | --- |
| ESP32 → host | `Hello from the microcontroller!` | Heartbeat, sent every 1 second |
| Host → ESP32 | `ping\n` | Command sent every 5 seconds |
| ESP32 → host | `ESP32 received: ping` | Echo of each received line |
| ESP32 → host | `ERROR: Command too long` | Command exceeded 127 characters |

## Configuration

| Setting | Location | Default |
| --- | --- | --- |
| Serial port | `serial_monitor.py` (`PORT`) | `COM4` |
| Baud rate | `serial_monitor.py` (`BAUD_RATE`) | `115200` |
| Read timeout | `serial_monitor.py` (`READ_TIMEOUT`) | `0.2` s |
| Ping interval | `serial_monitor.py` (`next_command_time = now + 5`) | `5` s |
| Heartbeat interval | `pyserial.ino` (`SEND_INTERVAL_MS`) | `1000` ms |
| Max command length | `pyserial.ino` (`inputBuffer[128]`) | 127 chars |

## Troubleshooting

1. **`could not open port 'COMx'`**
   - Verify the port name with `python -m serial.tools.list_ports`.
   - Close the Arduino Serial Monitor or any other program using the port.
   - On Linux, ensure your user is in the `dialout` group.

2. **No output after connecting**
   - Confirm the baud rate matches the sketch (`115200`).
   - Press the board's reset button after the script connects; the script discards
     startup messages during its initial 1.5 second settling period.

3. **`ModuleNotFoundError: No module named 'serial'`**
   - Install dependencies inside the virtual environment:
     `pip install -r requirements.txt`.

4. **Garbled or partial lines**
   - Make sure only one process is reading the port.
   - Check that the USB cable supports data (not charge-only).

## License

This project is part of the ESP32-IOT repository and is licensed under the MIT License.
See the repository-level [LICENSE](../LICENSE) file for details.
