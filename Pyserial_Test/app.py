import serial
import time

def monitor_serial(port='/dev/ttyUSB0', baudrate=115200, timeout=1):
    try:
        # Configure serial connection
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=timeout,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
        
        print(f"Connected to {port} at {baudrate} baud")
        
        while True:
            # Read data if available
            if ser.in_waiting > 0:
                try:
                    # Read line and decode
                    line = ser.readline().decode('utf-8').strip()
                    print(f"Received: {line}")
                except UnicodeDecodeError:
                    print("Error decoding message")
                    
    except serial.SerialException as e:
        print(f"Error: {e}")
        
    finally:
        if 'ser' in locals():
            ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    # Change these parameters according to your setup
    PORT = "/dev/ttyUSB0"  
    BAUD_RATE = 115200
    
    monitor_serial(PORT, BAUD_RATE)