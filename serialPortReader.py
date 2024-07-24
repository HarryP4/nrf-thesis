import serial
import time

# Define the serial port name and baud rate
port = 'COM6'
baud_rate = 115200  # Match the baud rate with the receiver device

# Open the serial port
try:
    ser = serial.Serial(port, baud_rate)
except serial.SerialException as e:
    print(f"Failed to open serial port: {e}")
    exit()

# Open a file for writing the received data
filename = 'data.csv'  # Replace with the desired filename
with open(filename, 'w') as file:
    startTime = time.time()
    try:
        # Read and write data until interrupted
        while True:
            # Read a line from the serial port
            line = ser.readline().decode().strip()
            curTime = time.time()
            while line:
                try:
                    timestamp = curTime - startTime
                    # Write the line to the file
                    file.write(str(timestamp) + ', ' + line + '\n')
                    file.flush()  # Flush the buffer to ensure data is written immediately
                except ValueError:
                    file.write(str(timestamp) + ', ' + ' ' + '\n')

            file.write(str(timestamp) + ', ' + ' ' + '\n')
    except KeyboardInterrupt:
        pass

# Close the serial port
ser.close()
