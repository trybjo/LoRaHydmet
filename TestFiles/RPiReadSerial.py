#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jun 18 04:51:11 2018

@author: pi
"""

import serial
from time import sleep


ser = serial.Serial("/dev/ttyACM0", 9600)

sleep(0.1)

dataFromArduino = ser.readline().decode()


while True:
    
    f = open("/home/pi/Documents/Log.txt", "a")
    f.write(dataFromArduino)
    f.close()
    print(dataFromArduino)

    while True:
        newDataFromArduino = ser.readline().decode()
        if dataFromArduino != newDataFromArduino:
            dataFromArduino = newDataFromArduino
            break




    
'''
Melding = bytearray(32)
    
    # Get the last line from the Log.txt file. This is the last recieved message from the nodes.
with open("/home/pi/Documents/Log.txt", "r") as f:
    lines = f.read().splitlines()
    last_line = lines[-1]
    print(last_line)
        
    # Get the length of the last line.
    print(len(last_line))
    print(type(last_line))
    
    a = "Hello"
    print(len(a))
    print(type(a))
    
    a+= "0"*9
    print(a)
    print(len(a))
    print(type(a))
'''