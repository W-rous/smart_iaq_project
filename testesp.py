import serial
import sqlite3
import sys
from datetime import datetime
port = serial.Serial("/dev/ttyAMA0",baudrate=115200,timeout=3.0)
def readlineCR(port):
        rv = ""
        rv = port.readline()
        return rv

def log_values(sensor_id, co2, temp, hum,battvolt, battlvl, avgamp, nanovolt,measuretime,sleeptime):
	conn=sqlite3.connect('/var/www/lab_app/sensorData.db')
	curs=conn.cursor()
	curs.execute("""INSERT INTO iaq values(datetime('now','localtime'),
	(?),(?), (?), (?), (?), (?), (?), (?), (?),(?))""",((sensor_id, co2, temp, 
	hum, battvolt, battlvl, avgamp, nanovolt,measuretime,sleeptime)))
	conn.commit()
	conn.close() 

while True:

	while port.in_waiting  == False:
		pass
	data = readlineCR(port)
	data1 = data.rstrip()
	if data1 == "getTimeNow":
		tn = datetime.time(datetime.now())
		tnInString = str(tn.hour)
		port.write(tnInString+"\n")
		print("Time sent: " + tnInString)
	elif data1 == "sendData":
		port.write("sendDataOK\n")
		while port.in_waiting == False:
			pass
		data2 = readlineCR(port)
		print(data2)
		a = data2.split("%")
		co2 = int(a[0])
		temp = float(a[1])
		humid = float(a[2])
		battVolt = int(a[3])/1000.00
		battLvl = int(a[4])
		avgAmp = float(a[5])*1000.00
		nanoVolt = int(a[6])/1000.0
		measureTime = int(a[7]) /1000.00
                sleepTime = int(a[8])
		if a > 0:
 			log_values("1", co2, temp, humid, battVolt, battLvl, avgAmp,
			nanoVolt, measureTime,sleepTime)
		else:
			log_values("1", -999, -999,-999,-999,-999,-999,-999,
			-999)
	else:
		print(data)
