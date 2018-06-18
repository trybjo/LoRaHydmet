# -*- coding: utf-8 -*-
"""
Created on Fri Jun 15 13:10:41 2018

@author: Andreas
"""

# Need to use telnet. You must first enable telnet

import socket
import sys

# Library for encryption/decryption
from Crypto.Cipher import AES

# Need a symmetric key. Same key used to encrypt and decrypt.
# The same key can be generated in both server and client if the same passphrase and initialization vector (IV) are used.
# Both the key and the message must be either 16, 24 or 32 bytes long.

def encryptMessage(message):
    obj = AES.new("This is a key12h", AES.MODE_CBC, "This is an IV456")
    ciphertext = obj.encrypt(message)
    return ciphertext

def decryptMessage(encryptedMessage):
    obj2 = AES.new("This is a key12h", AES.MODE_CBC, "This is an IV456")
    message = obj2.decrypt(encryptedMessage)
    return message




# The host is here blank. This means that we can listen on any interface we choose.
host = " "

# Choose a free port
port = 8888

# Can use try: and except: like in the client, but not necessary.

# Create a TCP socket using IPv4
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("Socket is created")

# Try to bind the server to the host and the port specified.
# Need the extra () because we're passing in a data structure.
try:
    s.bind((host,port))
except socket.error:
    print("Binding failed")
    sys.exit()
    
print("Socket has been bounded")

# We can now listen for incomming connections.
# The argument is how many connections can be qued. Here the 11th connection will be rejected.
s.listen(10)
print("Socket is now listening")

# The connection variable and the address variable stores the IP address and the port of the client that is trying to connect.
# It will also accept the connection.
conn, addr = s.accept()

# addr[0] is the IP address, and addr[1] is the port number.
print("Connected with " + addr[0] + " on port " + str(addr[1]))

# Create an ininite loop where you listen for data from the client.
while 1:
    # We can recieve data from the client if it tries to send something.
    # Specify how many bytes you want to recieve. Here 1 kilobyte is used.
    data =  conn.recv(1024)
    # print(data.decode())
    print("\nEncrypted data:")
    print(data)
    print("\nDecrypted data:")
    print(decryptMessage(data).decode())
    
    # Break out of the loop if the connection is broken.
    if not data:
        break;

    # We can send this message onwards to all the connected clients.
    #telconn.sendall(data)

# Close the socket connection between this server and the client.
conn.close()
print("Lost connection to the client. The connection is now closed. The server will also close now.")

# Close the socket itself.
s.close()