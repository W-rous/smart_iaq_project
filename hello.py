from flask import Flask, request
from flask import render_template
import time
import datetime
app = Flask(__name__)
app.debug = True
@app.route("/")
def hello():
   return render_template('hello.html',message="Hello World!")
@app.route("/past",methods=['GET'])
def past():
   from_date_str = request.args.get('from',time.strftime("%Y-%m-%d 00:00"))
   to_date_str = request.args.get('to',time.strftime("%Y-%m-%d %H:%M"))
   range_h_form = request.args.get('range_h')
   range_h_int = "nan"
   try:
        range_h_int = int(range_h_form)
   except:
        print "range_h_form not a number"
   if isinstance(range_h_int,int):
        time_now = datetime.datetime.now()
        time_from = time_now - datetime.timedelta(hours = range_h_int)
        time_to = time_now
        from_date_str = time_from.strftime("%Y-%m-%d %H:%M")
        to_date_str = time_to.strftime("%Y-%m-%d %H:%M")
   import sqlite3
   conn=sqlite3.connect('/var/www/lab_app/sensorData.db')
   curs=conn.cursor()
   curs.execute("SELECT rDatetime, co2 FROM iaq WHERE rDatetime BETWEEN ? AND ?", (from_date_str,to_date_str))
   co2sAll = curs.fetchall()
   curs.execute("SELECT rDatetime, temp FROM iaq WHERE rDatetime BETWEEN ? AND ?", (from_date_str,to_date_str))
   tempsAll = curs.fetchall()
   curs.execute("SELECT rDatetime, humid FROM iaq WHERE rDatetime BETWEEN ? AND ?", (from_date_str,to_date_str))
   humidsAll = curs.fetchall()
   conn.close()
   return render_template("fypfinal_past.html",co2All = co2sAll,tempAll = tempsAll, humidAll = humidsAll,co2All_items=len(co2sAll),tempAll_items=len(tempsAll),humidAll_items=len(humidsAll),from_date=from_date_str,to_date=to_date_str)
@app.route("/home")
def home():
   import sqlite3
   conn=sqlite3.connect('/var/www/lab_app/sensorData.db')
   curs=conn.cursor()
   curs.execute("SELECT co2 FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   co2s = curs.fetchone()
   curs.execute("SELECT temp FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   temps = curs.fetchone()
   tempsMod = temps[0]/1.00
   curs.execute("SELECT humid FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   humids = curs.fetchone()
   humidsMod = humids[0]/1.00
   curs.execute("SELECT rDatetime FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   rDatetimes = curs.fetchone()
   rDatetimes = rDatetimes[0]
   rDatetimesSlice = rDatetimes[11:-6]
   rDatetimesSlice = int(rDatetimesSlice)
   if rDatetimesSlice >= 0 and rDatetimesSlice < 12 :
	a = " am"
   else:
	a = " pm"
   rDatetimes = rDatetimes + a
   curs.execute("SELECT avga FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   avgas = curs.fetchone()
   avgasMod = avgas[0]/1.00
   curs.execute("SELECT measuretime FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   measuretimes = curs.fetchone()
   measuretimesMod = measuretimes[0]/1.00
   curs.execute("SELECT battvol FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   battvols = curs.fetchone()
   battvolsMod = battvols[0]/1.00
   curs.execute("SELECT battlvl FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   battlvls = curs.fetchone()
   curs.execute("SELECT nanovolt FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   nanovolts = curs.fetchone()
   nanovoltsMod = nanovolts[0]/1.00
   curs.execute("SELECT sleeptime FROM iaq ORDER BY rDatetime DESC LIMIT 1")
   sleeptimes = curs.fetchone()
   conn.close()
   return render_template("fypfinal.html",co2 = co2s[0], temp = tempsMod, humid = humidsMod, rDatetime = rDatetimes,avga=avgasMod,measuretime=measuretimesMod,battvol=battvolsMod,battlvl=battlvls[0],nanovolt=nanovoltsMod,sleeptime=sleeptimes[0]) 

def validate_date(d):
   try:
	datetime.datetime.strptime(d,'%Y-%m-%d %H:%M')
	return True
   except ValueError:
	return False

if __name__ == "__main__":
   app.run(host='0.0.0.0',port=8080)
