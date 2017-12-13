import serial
import time
import datetime
import os
import subprocess
ser = serial.Serial('/dev/ttyACM0')
ser.baudrate = 19200
ser.bytesize=8
ser.parity='N'
ser.stopbits=1
ser.timeout=0
ser.xonxoff=0
ser.rtscts=0
ser.inter_byte_timeout=1






def siirto():
    os.chdir("/home/pi/Desktop/")
    ping=subprocess.call("ping -c 2 139.59.149.168",shell=True)
    if ping==0:
        print("ping was succesful, synchronizing data to server")
        print
        try:
            sisalto= os.listdir("kuvia")
        except IndexError:
            sisalto[0]=0;
            pass
        if len(sisalto):
            print("synchronizing")
            print
            y="sshpass -p puliukko scp -r kuvia/* ledipoika@139.59.149.168:/var/www/html/images/"
            subprocess.call(y,shell=True)
            time.sleep(5)
            x=sisalto[0]
            print("synchronized")
            print
            os.chdir("/home/pi/Desktop/kuvia/")
            subprocess.call(["rm",x])
        else:
            print("The folder was empty or already synchronized")
            print
            time.sleep(2)
    else:
        print("couldn`t ping the server, trying again in 2 seconds")
        print(".....................................................")
        print
        time.sleep(2)

r=0
luku=0
main=0
while main==0:
    tarkiste=ser.read()
    raaka_aika=str(datetime.datetime.now())
    index = raaka_aika.find('.')
    aika = raaka_aika[:index]
    os.chdir("/home/pi/Desktop/kuvia/")
    if "6" in tarkiste:
        while r<2500:
            
            data = open(aika+ '.jpg' ,"wb+")
            data.close()
            z=0
            data = open(aika+ '.jpg',"ab",0)
            while z==0:
                if ser.inWaiting()>0:
                    y=ser.read()
                    ' '.join(format(x, 'b') for x in bytearray(y))
                    data.write(y)
        
        
                else:
                    r=r+1
                    print(r)
                    if r>2500:
                        z=1
                        luku=luku+1
        data.close()
    tarkiste=0
    r=0
    siirto()



    


