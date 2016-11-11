import serial 
import time
import numpy as np
import json
from datetime import datetime
from multiprocessing import Process

#This file is used to save real-time data from the detector. You will have to change the variable ComPort to the 
#	name of the USB port it is plugged into. If the Arduino is not recognized by your computer, make sure you have
#	installed the drivers for the Arduino.

fname = raw_input("Enter file name (eg. test.txt):")
id = raw_input("Enter device ID:")
print("Taking data ...")
print("Press ctl+c to terminate process")

ComPort = serial.Serial('/dev/cu.wchusbserialfa130') # open the COM Port. You need to figure out your port name.

ComPort.baudrate = 9600          # set Baud rate
ComPort.bytesize = 8             # Number of data bits = 8
ComPort.parity   = 'N'           # No parity
ComPort.stopbits = 1  	

file = open(fname, "w",0)

counter = 0
while True:
	data = ComPort.readline()    # Wait and read data 
	counter +=1
	if counter > 3:                   # print the received data
		file.write(str(id) + " "+ str(datetime.now())+" "+data)
	else:
		file.write(data)
xComPort.close()     
file.close()  

