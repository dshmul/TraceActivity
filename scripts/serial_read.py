# Python script used to load in ESP-32 data from serial port
# data is then logged into a .csv file for further analysis 

from xml.etree.ElementTree import tostring
import serial
import csv
import time

# Start timer
current_time = time.time()
new_file_cut_time = (5*60)

# Create initial .csv
iteration = 0
current_file = ("Data_at_" + str(iteration) + ".csv")

# Open port read data line by line
ser = serial.Serial('COM8', 115200)
while True:
    line = ser.read(50)

    # Write to csv
    # Check time
    if (time.time() - current_time > new_file_cut_time):  
        # update time and .csv
        current_time = time.time()
        iteration += 1
        current_file = ("Data_at_" + str(iteration) + ".csv")

    # Output data to current .csv
    with open (current_file, 'a', newline = '') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow([line])
    
    print(line)