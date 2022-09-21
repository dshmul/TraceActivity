# Python script used to load in ESP-32 data from serial port
# data is then logged into a .csv file for further analysis 

from datetime import datetime
import serial
import csv
import time
import serial.tools.list_ports

##### PORT DETECTION #####
def get_ports():
    ports = serial.tools.list_ports.comports()
    return ports

def findESP(portsFound):
    commPort = 'None'
    numConnections = len(portsFound)

    for i in range(0, numConnections):
        port = portsFound[i]
        strPort = str(port)

        if 'Silicon Labs CP210x' in strPort:
            splitPort = strPort.split(' ')
            commPort = (splitPort[0])

    return commPort

##### START SERIAL READING #####
# Start timer
current_time = time.time()
new_file_cut_time = (5*60)

# Create initial .csv
iteration = 0
current_file = ("Data_at_" + str(iteration) + ".csv")
# Write Header Row
with open (current_file, 'a', newline = '') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["CHANNEL", "RSSI", "Request MAC", "Time Stamp"])

# Use detect port functions
foundPorts = get_ports()
connectPort = findESP(foundPorts)

if connectPort != 'None':
    ser = serial.Serial(connectPort, 115200)
    print('Connected to ' + connectPort)
else:
    print('Connection Issue! Set Port Manually')
    ser = serial.Serial('COM6', 115200)

while True:
    line = ser.readline()
    line = str(line).replace('CHAN=', "")
    line = str(line).replace('RSSI=', '')
    line = str(line).replace('Request MAC=', '')
    line = str(line).replace('b\'', '')
    line = str(line).replace('\\r\\n\'', '')
    line = line + ", " + (str(datetime.now()))

    # Write to csv
    # Check time
    if (time.time() - current_time > new_file_cut_time):  
        # update time and .csv
        current_time = time.time()
        iteration += 1
        current_file = ("Data_at_" + str(iteration) + ".csv")
        # Write Header Row
        with open (current_file, 'a', newline = '') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["CHANNEL", "RSSI", "Request MAC", "Time Stamp"])

    # Output data to current .csv
    with open (current_file, 'a', newline = '') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow([line])
    
    print(line)