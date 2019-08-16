#include <FirebaseESP8266.h>
#include <FirebaseESP8266HTTPClient.h>

////#include <Firebase.h>
//#include <FirebaseArduino.h>
////#include <FirebaseCloudMessaging.h>
////#include <FirebaseError.h>
////#include <FirebaseHttpClient.h>
////#include <FirebaseObject.h>


// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266WiFi.h>//menghubungkan dengan wifi
#include <ESP8266HTTPClient.h>//mengirimkan HTTP response
#include <ESP8266WebServer.h>//berperan serbagai server

#define FIREBASE_HOST "https://project1-f35fa.firebaseio.com/" 
#define FIREBASE_AUTH "KR4OHNN8JkdKyaCJb1qipLurRf1EcVQb1VWe8wZI" 


//setup webserver dan httpclient
ESP8266WebServer server(80);//port 80 = http
String ssid="risti",pass="pagiyangmiringkekanan";
RTC_DS3231 rtc;
HTTPClient client;


//Define FirebaseESP8266 data object
FirebaseData firebaseData;

//setup variabel
int totalData=-1;
int led1=2,led2=15,stateAlarm=1;
int brightnessMaxLED1=1023,brightnessMaxLED2=1023;
int totalBrightnessMax=brightnessMaxLED1+brightnessMaxLED2;
float brightnessSimulasi=0,brightnessReal=0;
float intervalSimulasi=0,intervalReal=0;
int currentTotalBrightness=0,totalDurasi=0;
int currentBrightnessLED1=0,currentBrightnessLED2=0;
int jam=-1,menit=-1;
float jeda=0,jedaReal=0;//durasi/maks;


void handleMatikanAlarm(){
  Serial.println("Matikan alarm is called");
//  currentTotalBrightness = 0;
//  currentBrightnessLED1=0;
//  currentBrightnessLED2=0;
//  totalDurasi=0;
//  jam=-1;
//  menit=-1;
//  stateAlarm=1;
//  analogWrite(led1,0);
//  analogWrite(led2,0);

  //ngitung deviasi/selisih waktu
  DateTime now = rtc.now();
  char hourTimeBuffer[5]="";
  char minuteTimeBuffer[5]="";
  char secondTimeBuffer[5]="";
  sprintf(hourTimeBuffer,"%02d",now.hour());
  sprintf(minuteTimeBuffer,"%02d",now.minute());
  sprintf(secondTimeBuffer,"%02d",now.second());
  int hourNow = atoi(hourTimeBuffer);
  int minuteNow = atoi(minuteTimeBuffer);
  int secondNow = atoi(secondTimeBuffer);
  int selisihJam = hourNow-jam;
  int selisihMenit = minuteNow-menit;
  int selisihDetik = secondNow;
  int totalSelisih = selisihJam*3600+selisihMenit*60+selisihDetik;


  //ngirim data ke firebase 
  if (Firebase.setInt(firebaseData, "status_alarm",0)){
          //if()
  }
  else{
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  totalData++;
  String totalDataString = String(totalData);
  String path = "data/"+(totalDataString);
  String path1="data";
  
  if(Firebase.setInt(firebaseData,path+"/kecerahan",brightnessReal)){
  
  }
  else{
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if(Firebase.setInt(firebaseData,path+"/interval",intervalReal)){
  
  }else{
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  
  if(Firebase.setInt(firebaseData,path+"/deviasi",totalSelisih)){
  
  }else{
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  
  if(Firebase.setInt(firebaseData,path1+"/total_data",totalData)){
      //totalData++;
  }else{
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  
  
  //reset variabel
  currentTotalBrightness = 0;
  currentBrightnessLED1=0;
  currentBrightnessLED2=0;
  totalDurasi=0;
  jam=-1;
  menit=-1;
  stateAlarm=1;
  analogWrite(led1,0);
  analogWrite(led2,0);


  
  String message = "Alarm dimatikan";
  server.send(200,"text/plain",message);
}

void handleMatikanSimulasi(){
  Serial.println("Matikan Simulasi is called");
  
  //reset semuanya
  currentTotalBrightness = 0;
  currentBrightnessLED1=0;
  currentBrightnessLED2=0;
  totalDurasi=0;
  stateAlarm=1;
  analogWrite(led1,0);
  analogWrite(led2,0);

  String message = "Simulasi dimatikan";
  server.send(200,"text/plain",message);
}

void handleConfigData(){
  Serial.println("handleConfigData is called");
  
  if(server.hasArg("jam") == true){
    String jamString = server.arg("jam");
    if(server.hasArg("menit") == true){
      String menitString=server.arg("menit");
      if(server.hasArg("kecerahan") == true){
        String kecerahan = server.arg("kecerahan");
        if(server.hasArg("interval") == true){
          String interval = server.arg("interval");
          //kirim respon
          String message = "Post request received, data saved";
          server.send(200,"text/plain",message);
          Serial.print("Jam: ");
          Serial.println(jamString);
          Serial.print("Menit: ");
          Serial.println(menitString);
          Serial.print("Kecerahan: ");
          Serial.println(kecerahan);
          Serial.print("Interval: ");
          Serial.println(interval);

          //int kecerahanInt = kecerahan.toInt();
          brightnessReal= kecerahan.toInt()*0.01*totalBrightnessMax;
          //int intervalInt = interval.toInt();
          intervalReal = interval.toInt()*1000;
          jam = jamString.toInt();
          menit = menitString.toInt();
          jedaReal= round(intervalReal/brightnessReal);
        }
      }
      
      
      //startSimulation(kecerahan,interval);
    }
  }
}

void handleSimulasi(){
  Serial.println("Simulation called");  

  if(server.hasArg("kecerahan") == true){
    String brightnessString = server.arg("kecerahan");
    brightnessSimulasi=brightnessString.toInt();//ngubah ke integer dari string
    brightnessSimulasi = brightnessSimulasi*0.01*totalBrightnessMax;//ngubah dari persen ke nilai sebenarnya
    Serial.print("Kecerahan: ");
    Serial.println(brightnessString);
    if(server.hasArg("interval") == true){
      String intervalString = server.arg("interval");
      intervalSimulasi=intervalString.toInt();
      intervalSimulasi = intervalSimulasi*1000;
      Serial.print("Interval: ");
      Serial.println(intervalString);
      //startSimulation(kecerahan,interval);
      jeda=round(intervalSimulasi/brightnessSimulasi);
      stateAlarm=3;//pindah ke state 3
      analogWrite(led1,0);
      analogWrite(led2,0);
      //kirim respon
      String message = "Post request received";
      server.send(200,"text/plain",message);

      //startSimulation(kecerahan,interval);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT); 
  Serial.begin(115200);


  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  
  Wire.begin();
  Wire.status();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//    rtc.adjust(DateTime(2019, 7, 16, 20, 15, 15));
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
     //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  //connect to WiFi net
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid,pass);//connect ke wifi
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }
  Serial.println("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  String path = "data/total_data";
  if (Firebase.getInt(firebaseData, path))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.println("ETag: " + firebaseData.ETag());
      Serial.print("VALUE: ");
      if (firebaseData.dataType() == "int"){
        Serial.println(firebaseData.intData());
        totalData = firebaseData.intData();
      }
      else if (firebaseData.dataType() == "float")
        Serial.println(firebaseData.floatData(), 5);
      else if (firebaseData.dataType() == "double")
        printf("%.9lf\n", firebaseData.doubleData());
      else if (firebaseData.dataType() == "boolean")
        Serial.println(firebaseData.boolData() == 1 ? "true" : "false");
      else if (firebaseData.dataType() == "string")
        Serial.println(firebaseData.stringData());
      else if (firebaseData.dataType() == "json")
        Serial.println(firebaseData.jsonData());
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

  
  //routing function
  server.on("/simulation",handleSimulasi);
  server.on("/save",handleConfigData);
  server.on("/matikan_simulasi",handleMatikanSimulasi);
  server.on("/matikan_alarm",handleMatikanAlarm);
  
  
  //start the server
  server.begin();
  Serial.println("Server started");
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  server.handleClient();//ini untuk mendengarkan komunikasi/data/client yg masuk

  //ada 3 state dalam aplikasi:
  // 1 = alarm dalam keadaan ON, standby menunggu waktu menyala
  // 2 = alarm dalam keadaan ON, waktunya menyala/alarm menyala
  // 3 = alarm dalam keadaan SIMULASI
  if(stateAlarm==0){
    Serial.print("State: ");
    Serial.println(stateAlarm);
    //do nothing
  }else if(stateAlarm==1){
    //menunggu waktu yg tepat
    DateTime now = rtc.now();
//    Serial.print(now.hour(),DEC);
//    Serial.print(":");
//    Serial.print(now.minute(),DEC);
//    Serial.print(":");
//    Serial.print(now.second(),DEC);
//    Serial.println("");
//    //String hour = now.hour();

    //char dateTimeBuffer[20]="";    
    //parsing nilai di rtc ke int
    char hourTimeBuffer[5]="";
    char minuteTimeBuffer[5]="";
    sprintf(hourTimeBuffer,"%02d",now.hour());
    sprintf(minuteTimeBuffer,"%02d",now.minute());
    int hour = atoi(hourTimeBuffer);
    int minute=atoi(minuteTimeBuffer);
    
    
    if(hour == jam){
      if(minute == menit){
        analogWrite(led1,0);
        analogWrite(led2,0);
        stateAlarm=2;
        if (Firebase.setInt(firebaseData, "status_alarm",1)){
          //if()
        }
        else{
          Serial.println("FAILED");
          Serial.println("REASON: " + firebaseData.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
        }
    
//        Firebase.setFloat("status_alarm", 1); 
//        // handle error 
//        if (Firebase.failed()) { 
//          Serial.print("setting /number failed:"); 
//          Serial.println(Firebase.error());   
////      return; 
//        } 
      }
    }
    String dateTimeCompare="01:35";
//    sprintf(dateTimeBuffer,"%02d:%02d",now.hour(),now.minute());
//    Serial.print(dateTimeBuffer);
//    if(strcmp(dateTimeBuffer,dateTimeCompare.c_str())==0){
//      stateAlarm=2;
//    }
//    else{
//      Serial.print("Wrong");
//    }

    
  }else if(stateAlarm==2){
    //alarm nyala
     Serial.println("Alarm menyala!");
     if(currentTotalBrightness >= brightnessReal){
        Serial.print("Durasi total: ");
        Serial.println(totalDurasi);
        Serial.print("Brightness Total: ");
        Serial.println(currentTotalBrightness);
        return;
        //kembalikan ke state awal, dan reset semua variabel
//        stateAlarm=1;
//        currentTotalBrightness = 0;
//        currentBrightnessLED2=0;
//        currentBrightnessLED1=0;
//        totalDurasi=0;
//        return;
    }
    else if(currentTotalBrightness <= brightnessMaxLED1){
      //nyalakan lampu pertama
      analogWrite(led1,currentBrightnessLED1);
      currentTotalBrightness++;
      currentBrightnessLED1++;
   }else{
      //nyalakan lampu kedua
      analogWrite(led2,currentBrightnessLED2);
      currentTotalBrightness++;
      currentBrightnessLED2++;
   } 
   Serial.print("Brightness Total: ");
   Serial.println(currentTotalBrightness);
   totalDurasi+=jedaReal;
   delay(jedaReal);    
 }else if(stateAlarm==3){
    //simulasi
    
    
    if(currentTotalBrightness >= brightnessSimulasi){
        Serial.print("Durasi total: ");
        Serial.println(totalDurasi);
        Serial.print("Brightness Total: ");
        Serial.println(currentTotalBrightness);
        
        //kembalikan ke state awal, dan reset semua variabel
        stateAlarm=1;
        currentTotalBrightness = 0;
        currentBrightnessLED2=0;
        currentBrightnessLED1=0;
        totalDurasi=0;
        return;
    }
    else if(currentTotalBrightness <= brightnessMaxLED1){
      //nyalakan lampu pertama
      analogWrite(led1,currentBrightnessLED1);//ngasih arus ke pin wemos yg nyambung dengan led
      currentTotalBrightness++;
      currentBrightnessLED1++;
    }else{
      //nyalakan lampu kedua
      analogWrite(led2,currentBrightnessLED2);
      currentTotalBrightness++;
      currentBrightnessLED2++;
    }

    Serial.print("Brightness Total: ");
    Serial.println(currentTotalBrightness);
    totalDurasi+=jeda;
    delay(jeda);    
  }
}
