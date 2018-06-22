# -*- coding: utf-8 -*-
import socket
from time import sleep
import time

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
port = 8889

# AF_INET means that we're using IP version 4. SOCK_STREAM means that we're using TCP.
# Try to create socket.
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

# If the code have passed the "test", it means that the socket has been created.
print("Socket has been created")

    
# If we've gotten this far in the code, we now have the IP address.
print("IP Address: " + host)

# Connect to the port on this IP. 
# The argument is passed into a data structure that the connect function uses. We therefore need an extra ()
s.connect((host, port))
#print("Socket connected to " + host + " on port " + port)

# A variable that keeps track of whether or not were connected to the server.
connected = True



while True:
    
    try:
        
        # A variable that keeps track of how many packages that are not sent if there is a connection error.
        # -1 means that everything is on track. -2 means that one is on holdfor sending. -3 mens two are on hold etc.
        logIndex = -1
        
        # If the lastSentMessage file is empty, send the current file and take it from there.
        with open("/home/pi/Documents/LastSentMessage.txt", "r") as l:
            lastSentMessage = l.read()
            if lastSentMessage != '':
        
                # Find out what the last sendt message is.
                with open("/home/pi/Documents/LastSentMessage.txt", "r") as l:
                    lines = l.read().splitlines()
                    lastSentMessage = lines[-1]
                    lastSentMessage = lastSentMessage.strip()
                    
                # Find out how far back in the log that message is (in case messages haven't been sent because of connection loss).
                foundLastSentMessageInLog = False
                while foundLastSentMessageInLog == False:
                    with open("/home/pi/Documents/Log.txt", "r") as f:
                        lines = f.read().splitlines()
                        messageFromLog = lines[logIndex]
                    if messageFromLog == lastSentMessage:
                        foundLastSentMessageInLog = True
                        f.close()
                    else:
                        logIndex += -1
                        f.close()
            
            # If the lastSentMessage file is empty, logIndex is set to -2 to avoid bug in next for loop.
            else:
                logIndex = -2
                
        for i in range(logIndex+1, 0, +1):
            with open("/home/pi/Documents/Log.txt", "r") as f:
                lines = f.read().splitlines()
                currentMessage = lines[i]
                sleep(1)
            Message = currentMessage + " "*(32-len(currentMessage))
            encryptedMessage = encryptMessage(Message)
            s.sendall(encryptedMessage)
            s.settimeout(5)
            svar = s.recv(1024)
            print(svar.decode())
            sleep(1)
            l = open("/home/pi/Documents/LastSentMessage.txt", "w")
            l.write(Message)
            l.close()
            lastSentMessage = Message.strip()
            
 
        while True:
            with open("/home/pi/Documents/Log.txt", "r") as f:
                lines = f.read().splitlines()
                new_last_line = lines[-1]
                if currentMessage != new_last_line:
                    break
           
           
           
        
    except:
        connected = False
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Connection lost. Reconnecting...")
        while not connected:
            # Try to reconnect, otherwise sleep for two seconds.
            try:
                s.connect((host, port))
                connected = True
                print("Re-connected")
            except socket.error:
                sleep(2)
       
        
        
      
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
'''        
        
        
    
        # Get the last line from the Log.txt file. This is the last recieved message from the nodes.
        with open("/home/pi/Documents/Log.txt", "r") as f:
            lines = f.read().splitlines()
            last_line = lines[-1]
            
        # The message needs to be a multiple of 16.
        # Here open spaces are added on behind the message in order to become 32 bytes.
        Message = last_line + " "*(32-len(last_line))
        
        # Encrypt the message.
        encryptedMessage = encryptMessage(Message)
        
        # Send the encrypted message.
        s.sendall(encryptedMessage)
        
        # Check if the last line in Log.txt is equal to the message last sent.
        # If equal, keep checking till it changes.
        while True:
            with open("/home/pi/Documents/Log.txt", "r") as f:
                lines = f.read().splitlines()
                new_last_line = lines[-1]
                if last_line != new_last_line:
                    break
        
        
    except:
        connected = False
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Connection lost. Reconnecting...")
        while not connected:
            # Try to reconnect, otherwise sleep for two seconds.
            try:
                s.connect((host, port))
                connected = True
                print("Re-connected")
            except socket.error:
                sleep(2)
                
       
''' 


# If connection is lost, try to reconnect
#if(socket.error):
#    s.connect((host, port))


#except socket.error:
#    print("Did not send successfully")
#    sys.exit()
  
# Get the reply. This will be all the data that are sent back. (Here up to 4096 bytes).
#reply = s.recv(4096)

#print(reply)

# Close the socket connection
#s.close()
#
#sys.exit()