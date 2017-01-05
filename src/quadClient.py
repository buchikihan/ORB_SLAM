import socket
import time
import subprocess
import os
import threading

class slamClient(object):
    def __init__(self):
        #start the connection
        host = '192.168.42.1'
        port = 4200
        self.s = socket.socket()
        self.s.connect((host,port))
        #set the state file
        stateFile = open('state.txt','wb')
        stateFile.write('pause')
        stateFile.close()
        #start SLAM algorithm and wait for it to start
        SLAM = subprocess.Popen(['./Examples/Monocular/mono_tum &'],shell=True,stdin=None,stdout=open(os.devnull,'w'))#,stdout=open(os.devnull,'wb'))
        #os.system("./Examples/Monocular/mono_tum &")
        time.sleep(15)

    def enterInput(self):
        while True:
            state = raw_input('->')
            self.s.send(state)
            time.sleep(0.5)
            if state == 'exit':
                stateFile = open('state.txt','wb')
                stateFile.write('exit')
                stateFile.close()
                break
        self.s.close()

class imageServer(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        
    def run(self):
        self.sName = socket.socket()
        self.sName.bind(('',42001))
        self.sName.listen(1)
        self.connName, addrName = self.sName.accept()
        self.sData = socket.socket()
        self.sData.bind(('',42000))
        self.sData.listen(1)
        self.connData, addrData = self.sData.accept()
        while True:
            fileName = self.connName.recv(1024)
            if not fileName:
                break
            time.sleep(0.01)
            data = self.connData.recv(1024)
            dataFile = data
            while data:
                data = self.connData.recv(1024)
                dataFile = dataFile+data
            self.connName.send('done')
            f = open('/home/jono/src/ORB_image_folder/streaming_images/'+fileName,'wb')
            f.write(dataFile)
            f.close()
        self.connName.close()
        self.connData.close()

#fileTransfer = imageServer()
#fileTransfer.start()
slamSM = slamClient()
slamSM.enterInput()

