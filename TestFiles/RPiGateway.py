# -*- coding: utf-8 -*-
"""
Created on Fri Jun 15 09:10:14 2018

@author: Andreas
"""

import socket
import sys
# from time import sleep

# Library for encryption/decryption
from Crypto.Cipher import AES

# Need a symmetric key. Same key used to encrypt and decrypt.
# The same key can be generated in both server and client if the same passphrase and initialization vector (IV) are used.
# The key and must be either 16, 24 or 32 bytes long.
# The message must be a multiple of 16.
# Use message.decode() to remove b' ' from the message.

def encryptMessage(message):
    obj = AES.new("This is a key12h", AES.MODE_CBC, "This is an IV456")
    ciphertext = obj.encrypt(message)
    return ciphertext

def decryptMessage(encryptedMessage):
    obj2 = AES.new("This is a key12h", AES.MODE_CBC, "This is an IV456")
    message = obj2.decrypt(encryptedMessage)
    return message


# We need a host and a port to connect to. host is the IP address.
# IP address must be inside " ".
host = "10.10.212.9"
port = 8888

# AF_INET means that we're using IP version 4. SOCK_STREAM means that we're using TCP.
# Try to create socket.
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error:
    print("Failed to connect")
    sys.exit();

# If the code have passed the "test", it means that the socket has been created.
print("Socket has been created")

    
# If we've gotten this far in the code, we now have the IP address.
print("IP Address: " + host)

# Connect to the port on this IP. 
# The argument is passed into a data structure that the connect function uses. We therefore need an extra ()
s.connect((host, port))

# print("Socket connected to " + host + " on port " + port)


    
# Send a message/data to the server.
Message = "Hemmelig melding"
Message = encryptMessage(Message)

# We will now try to send this message on the socket. This function will send the whole message.
# .encode() converts the string into a byte array.
try:
    # s.sendall(Message.encode())
    s.sendall(Message)
except socket.error:
    print("Did not send successfully")
    sys.exit()
    
print("Message sent successfully")

# Get the reply. This will be all the data that are sent back. (Here up to 4096 bytes).
reply = s.recv(4096)

print(reply)

# Close the socket connection
s.close()