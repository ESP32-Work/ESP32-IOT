import time
import serial

PORT = "COM12"  # Change this to your board's port
BAUD_RATE = 115200
READ_TIMEOUT = 0.2

def monitor_serial():
    try:
        with serial.Serial(
            port=PORT,
            baudrate=BAUD_RATE,
            timeout=READ_TIMEOUT,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
        ) as ser:
            print(f"Connected to {PORT} at {BAUD_RATE} baud")
            # Some boards reset when the serial port opens.
            time.sleep(1.5)
            # Discard startup messages already buffered.
            ser.reset_input_buffer()
            next_command_time = time.monotonic()
            while True:
                # Send a command every five seconds.
                now = time.monotonic()
                if now >= next_command_time:
                    command = "ping\n"
                    ser.write(command.encode("utf-8"))
                    ser.flush()
                    print(f"Sent: {command.strip()}")
                    next_command_time = now + 5
                # Read one newline-terminated response.
                raw_line = ser.readline()
                if raw_line:
                    try:
                        line = raw_line.decode("utf-8").rstrip("\r\n")
                        print(f"Received: {line}")
                    except UnicodeDecodeError:
                        print(f"Received non-UTF-8 data: {raw_line!r}")
    except serial.SerialException as error:
        print(f"Serial communication error: {error}")
    except KeyboardInterrupt:
        print("\nSerial monitoring stopped.")

if __name__ == "__main__":
    monitor_serial()