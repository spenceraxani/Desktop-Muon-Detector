import serial 
import time
import numpy as np
import json
from datetime import datetime
from multiprocessing import Process

#This file is used to save real-time data from the detector. You will have to change the variable ComPort to the 
#	name of the USB port that the desktop muon detectr is plugged into. If the Arduino is not recognized by your computer, make sure you have
#	installed the drivers for the Arduino.

fname = raw_input("Enter file name (eg. test.txt):")
print("Taking data ...")
print("Press ctl+c to terminate process")

#ComPort = serial.Serial('/dev/cu.usbmodem1421') # open the COM Port
#ComPort = serial.Serial('/dev/cu.wchusbserial1410') # open the COM Port
#ComPort = serial.Serial('/dev/cu.wchusbserial1420') # open the COM Port
ComPort = serial.Serial('/dev/tty.wchusbserial1410') # open the COM Port



ComPort.baudrate = 9600          # set Baud rate
ComPort.bytesize = 8             # Number of data bits = 8
ComPort.parity   = 'N'           # No parity
ComPort.stopbits = 1  	

file = open(fname, "w",0)

while True:
	data = ComPort.readline()        # Wait and read data                    # print the received data
	file.write(str(datetime.now())+" "+data)

xComPort.close()     
file.close()  

