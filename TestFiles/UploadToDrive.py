#!/usr/bin/env python

from __future__ import print_function
import os

from apiclient import discovery
from httplib2 import Http
from oauth2client import file, client, tools
from apiclient.http import MediaFileUpload
from time import sleep

while True:
    
    try:
        SCOPES = 'https://www.googleapis.com/auth/drive'
        store = file.Storage('storage.json')
        creds = store.get()
    
        folderID = '1xD4LNtqT1ezVXa-x0KJd7V9tISEm1KIo'
    
    
        if not creds or creds.invalid:
            flow = client.flow_from_clientsecrets('client_secret.json', SCOPES)
            creds = tools.run_flow(flow, store)
        DRIVE = discovery.build('drive', 'v2', http=creds.authorize(Http()))
    
        FILES = (
            ('Log.txt', False),
        )
    
        for filename, convert in FILES:
            metadata = {'title': 'Log.txt', "parents": [{"id": folderID, "kind": "drive#childList"}]}
            #metadata = {'title': filename,
            #            'parents': folderID,}
            res = DRIVE.files().insert(convert=convert, body=metadata,
                    media_body=filename, fields='mimeType,exportLinks').execute()
            if res:
                print('Uploaded "%s" (%s)' % (filename, res['mimeType']))
        sleep(10800) # Seconds
        
    except:
        connected = False
        print("Disconnected")
        while not connected:
            try:
                SCOPES = 'https://www.googleapis.com/auth/drive'
                store = file.Storage('storage.json')
                creds = store.get()
            
                folderID = '1xD4LNtqT1ezVXa-x0KJd7V9tISEm1KIo'
            
            
                if not creds or creds.invalid:
                    flow = client.flow_from_clientsecrets('client_secret.json', SCOPES)
                    creds = tools.run_flow(flow, store)
                DRIVE = discovery.build('drive', 'v2', http=creds.authorize(Http()))
            
                FILES = (
                    ('Log.txt', False),
                )
            
                for filename, convert in FILES:
                    metadata = {'title': 'Log.txt', "parents": [{"id": folderID, "kind": "drive#childList"}]}
                    #metadata = {'title': filename,
                    #            'parents': folderID,}
                    res = DRIVE.files().insert(convert=convert, body=metadata,
                            media_body=filename, fields='mimeType,exportLinks').execute()
                    if res:
                        print('Uploaded "%s" (%s)' % (filename, res['mimeType']))
                sleep(10800) # Seconds
                connected = True
        
            except:
                sleep(5)
            
