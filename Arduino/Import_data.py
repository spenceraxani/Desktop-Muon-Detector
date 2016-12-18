import serial 
import time
import numpy as np
import json
from datetime import datetime
from multiprocessing import Process

#   This file is used to save real-time data from the detector. You will have to change the variable ComPort to the 
#	name of the USB port it is plugged into. If the Arduino is not recognized by your computer, make sure you have
#	installed the drivers for the Arduino (Arduino Nano requires the CH340 driver for MAC).

fname = raw_input("Enter file name (eg. test.txt):")
id = raw_input("Enter device name:")
print("Taking data ...")
print("Press ctl+c to terminate process")

ComPort = serial.Serial('/dev/cu.wchusbserial1410') # open the COM Port

ComPort.baudrate = 9600          # set Baud rate
ComPort.bytesize = 8             # Number of data bits = 8
ComPort.parity   = 'N'           # No parity
ComPort.stopbits = 1  	

file = open(fname, "w",0)

counter = 0
while True:
	data = ComPort.readline()    # Wait and read data 
	if counter == 0:
		file.write("######################################################################\n")
		file.write("### Desktop Muon Detector \n")
		file.write("### Questions? saxani@mit.edu \n")
		file.write("### Comp_time Counts Ardn_time[ms] Amplitude[mV] SiPM[mV] Deadtime[ms]\n")
		file.write("### Device Name: "+str(id)+"\n")
		file.write("######################################################################\n")
	file.write(str(datetime.now())+" "+data)
	counter +=1
	
xComPort.close()     
file.close()  

