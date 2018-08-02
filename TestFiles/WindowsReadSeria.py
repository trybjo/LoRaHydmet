#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jun 18 04:51:11 2018
@author: pi
"""

import serial
from time import sleep


# ser = serial.Serial("/dev/ttyACM0", 9600)
ser = serial.Serial("COM7", 9600)

sleep(0.1)

dataFromArduino = ser.readline().decode()


while True:
    
    f = open(r'''C:\Users\Andreas\Documents\Agder_Energi_S2018\Rekkeviddetest hav\Log.txt''', "a")
    f.write(dataFromArduino)
    f.close()
    print(dataFromArduino)

    while True:
        newDataFromArduino = ser.readline().decode()
        if dataFromArduino != newDataFromArduino:
            dataFromArduino = newDataFromArduino
            break

