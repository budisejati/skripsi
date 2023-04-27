   // PENDEKLARASIAN LIBRARY
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <SD.h>

// PENDEKLARASIAN VARIABEL
static const int RXPin2 = 16, TXPin2 = 17;                    // PIN TX DAN RX DARI GPS KE ESP32
HardwareSerial serial2(2);                                    // PIN HARDWARE SERIAL
TinyGPSPlus gps;                                              // MEMBUAT VARIABEL "GPS"
char wtanggal[10];                                  // MEMBUAT VARIABEL JAM & TANGGAL BERTIPE DATA CHAR
const int UTC_offset = 7;                                     // VARIABEL (Universal Time Coordinate) BERTIPE DATA CONSTANTA INTEGER
time_t prevDisplay = 0;                                       // MEMBUAT VARIABEL PREVDISPLAY
const int CS = 2;
File file;
String dataMessage;
float lati;
float longi;
String wktu;
String gpsString;
/*===================================================================================================================================================================*/

void setup(){
  Serial.begin(9600);                                         // KOMUNIKASI SERIAL DENGAN BOUDRATE 9600
  Serial2.begin(9600);
  Serial.println("Waiting for GPS time ... ");                // MENAMPILKAN TEKS KETIKA GPS SEDANG PROSES MENGAMBIL DATA SATELIT 

  // Initialize SD card
  SD.begin(CS);  
  if(!SD.begin(CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("SD Card tidak terpasang");
    return;
  }
  
  Serial.println("Menginisialisasi SD Card...");
  if (!SD.begin(CS)) {
    Serial.println("ERROR - Gagal Menginisialisasi SD Card!");
    return;    // init failed
  }
  file = SD.open("/fix5.txt", FILE_WRITE);
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/fix5.txt", "Latitude, Longitude, Waktu \n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}//setup

/*===================================================================================================================================================================*/

void loop(){
  GPS_Timezone_Adjust(); // MENYESUAIKAN WAKTU YANG DITAMPILKAN GPS KE WAKTU SEBENARNYA
  //dataMessage = "Latitude : " + float(lati) + "Longitude : " + float(longi) + "Waktu = " + String(wktu)"\t\n";
  //checkGPS();
  
   dataMessage = "Waktu = " + String(wktu) + "," + "\tLatitude " + float(lati) + "\tLongitude " + float(longi), "\r\n";
 // dataMessage = "" + String(lati) + "" +String(longi)  + "" + String(wktu), "\r\n";
  //Serial.println("Save data: ");
  //Serial.println(dataMessage);
  appendFile(SD, "/fix5.txt", dataMessage.c_str());
  delay(1000);
  
}//voidloop

/*===================================================================================================================================================================*/

// WAKTU
void GPS_Timezone_Adjust() {                                  // FUNGSI UNTUK MENYESUAIKAN WAKTU YANG DITAMPILKAN GPS KE WAKTU SEBENARNYA
  while (Serial2.available()) {                               // KONDISI YANG BERJALAN 'KETIKA' SERIAL 2 TERSEDIA
    if (gps.encode(Serial2.read())) {                         // KONDISI YANG BERJALAN 'JIKA' SERIAL 2 MEMBACA INPUTAN GPS
      int Year = gps.date.year();                             // MENGISI VARIABEL YEAR YANG BERTIPE DATA INTEGER DENGAN NILAI TAHUN
      byte Month = gps.date.month();                          // MENGISI VARIABEL MONTH YANG BERTIPE DATA BYTE DENGAN NILAI BULAN
      byte Day = gps.date.day();                              // MENGISI VARIABEL DAY YANG BERTIPE DATA BYTE DENGAN NILAI HARI
      byte Hour = gps.time.hour();                            // MENGISI VARIABEL HOUR YANG BERTIPE DATA BYTE DENGAN NILAI JAM
      byte Minute = gps.time.minute();                        // MENGISI VARIABEL MINUTE YANG BERTIPE DATA BYTE DENGAN NILAI MENIT
      byte Second = gps.time.second();                        // MENGISI VARIABEL SECOND YANG BERTIPE DATA BYTE DENGAN NILAI DETIK

      setTime(Hour, Minute, Second, Day, Month, Year);        // MENGATUR STRING DATA GPS
      adjustTime(UTC_offset * SECS_PER_HOUR);                 // MENGHITUNG WAKTU DARI ZONA WAKTU SAAT INI DENGAN NILAI OFFSET
    }//if
  }//while

  if (timeStatus() != timeNotSet) {                           // KONDISI YANG BERJALAN JIKA STATUS WAKTU
    if (now() != prevDisplay) {
      prevDisplay = now();
      tampilkan();                                            // UNTUK MEMBUAT FUNGSI 'TAMPILKAN'
    }//if now
  }//if time
}//void GPS Timezone

/*===================================================================================================================================================================*/

// LATITUDE LONGITUDE
 static void smartDelay(unsigned long ms){                          // FUNGSI UNTUK MENGATUR DELAY 
  unsigned long start = millis();                                   // MENGISI VARIABEL START DENGAN TIPE DATA UNSIGNED LONG DENGAN NILAI MILLIS
  do {                                                              // KONDISI YANG BERJALAN KETIKA 
    while (serial2.available())
      gps.encode(serial2.read());
  }// do
  while (millis() - start < ms);
}//static void smart delay
static void printFloat(float val, bool valid, int len, int prec){   // FUNGSI UNTUK MENGATUR NILAI BUJUR LINTANG DENGAN TIPE DATA FLOAT
  if (!valid){                                                      // KONDISI YANG BERJALAN JIKA GPS TIDAK DAPAT MENAMPILKAN NILAI DARI SATELIT
    while (len-- > 1)
    Serial.print('*');
    Serial.print(' ');
  }
  else{                                                             // KONDISI YANG BERJALAN KETIKA GPS DAPAT MENAMPILKAN NILAI DARI SATELIT
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
    Serial.print(' ');
  }
  smartDelay(0);
}
static void printInt(unsigned long val, bool valid, int len){       // FUNGSI UNTUK MENGATUR NILAI BUJUR LINTANG DENGAN TIPE DATA INTEGER
  char sz[32] = "*";
  if (valid)
  sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
  sz[i] = ' ';
  if (len > 0) 
  sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}
static void printStr(const char *str, int len){                     // FUNGSI UNTUK MENGATUR NILAI BUJUR LINTANG DENGAN TIPE DATA STRING
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
  Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

/*===================================================================================================================================================================*/
void tampilkan() {                                                                                                    // FUNGSI UNTUK MENAMPILKAN BEBERAPA KARAKTER
  // BUJUR LINTANG
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;                                                 //
  //Serial.println("");                                                                                                 //
 //Serial.print("Latitude : ");                                                                                        // MENAMPILKAN KARAKTER LATITUDE
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);                                                      // MENAMPILKAN NILAI DARI KARAKTER LATITUDE
  lati = gps.location.lat(), gps.location.isValid();
 // printf("%.6f",lati); 
 //Serial.print("Longitude : ");                                                                                     // MENAMPILKAN KARAKTER LONGITUDE
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);                                                      // MENAMPILKAN NILAI DARI KARAKTER LONGITUDE
  longi = gps.location.lng(), gps.location.isValid();
  //printf("%.6f",longi); 
  // WAKTU 
  if (gps.date.isValid() && gps.time.isValid()){
   sprintf (wtanggal, "%02d/%02d/%02d  %02d:%02d:%02d ", year(), month(), day(), hour(), minute(), second());        // MENGATUR NILAI DARI VARIABEL TANGGAL
  Serial.print("| Waktu = ");                                                                                     // MENAMPILKAN KARAKTER WAKTU
  Serial.print(wtanggal);                                                                                           // MENAMPILKAN NILAI DARI VARIABEL TANGGAL
  //Serial.print("|");
    wktu = wtanggal;
  }//if
  else {                                                                                                              // KONDISI YANG BERJALAN JIKA WAKTU BELUM TERBACA
    Serial.println("\tWaktu Belum Terbaca");
  }//else
  //delay(1000);                                                                                                        // DELAY 1 DETIK
} //void tampilkan

void checkGPS() {
     //Assuming we already have the coordinates
    float lati, longi;

    //I want to do something like this
    gpsString ="";  //make sure the string is empty if its not
    gpsString += lati;
    gpsString += " ";
    gpsString += longi;
    Serial.println(gpsString);
}
/*===============================================================*/

// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message))
   {
    Serial.println("Data appended");
    Serial.println(); 
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
