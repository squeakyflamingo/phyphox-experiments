/* Link between Arduino Nano 33 BLE Sense and phyphox app v1
 * Allows to choose which experiment the Arduino Nano 33 BLE Sense should execute, using Phyphox.
 * by Gautier Creutzer and Frédéric Bouquet, La Physique Autrement / Physics Reimagined, Laboratoire de Physique des Solides / Laboratory of Solid state Physics, Université Paris-Saclay / Paris-Saclay University
 * Check our other projects in English and French: www.physicsreimagined.com
 * 
 * This work is based on the phyphox Arduino library developed by the phyphox team at the RWTH Aachen University, which is released under the GNU Lesser General Public Licence v3.0 (or newer).
 * This work is released under the GNU Lesser General Public Licence v3.0 (or newer).
 */

#include <phyphoxBle.h> 
#include <Arduino_LSM9DS1.h>
//#include <Adafruit_LSM9DS1.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_HTS221.h>
#include <Arduino_APDS9960.h>

char board_name[] = "nano_white"; // to change the name displayed by the board using BLE: no space and no special character
float choice = 0.0;
float accx, accy, accz, acc;
float gyrx, gyry, gyrz, gyr;
float magx, magy, magz, magn;
float pressure;
float temperature, humidity;
int red, green, blue, ambient;
float red_float, green_float, blue_float, ambient_float;
const int analogInPin1 = A0; // change here to read another analog input
const int analogInPin2 = A1;
const int analogInPin3 = A2;
float voltage1, voltage2, voltage3;
unsigned long initial_time, first_time, fourth_time;
float first_difference_float, fourth_difference_float;
unsigned int period = 50;
const int ledPin = 22;
const int ledPin2 = 23;
const int ledPin3 = 24;

void receivedData(); // see Phyphox Arduino Library example

void setup()
{   
   PhyphoxBLE::minConInterval = 6; //6 = 7.5ms
   PhyphoxBLE::maxConInterval = 6; //6 = 7.5ms
   PhyphoxBLE::slaveLatency = 0; //
   PhyphoxBLE::timeout = 10; //10 = 100ms

   pinMode(22, OUTPUT);
   pinMode(23, OUTPUT);
   pinMode(24, OUTPUT);

   digitalWrite(ledPin, LOW);
   digitalWrite(ledPin2, HIGH);
   digitalWrite(ledPin3, HIGH);
   
   if (!IMU.begin()) { // starting all useful sensors
     while (1);
   }
   if (!BARO.begin()) {
     while (1);
   }
   if (!HTS.begin()) {
     while (1);
   }
   if (!APDS.begin()) {
    while (1);
   }
   PhyphoxBLE::start(board_name); // the name of the board can be changed at the beginning of the program
   PhyphoxBLE::configHandler=&receivedData; // see Phyphox Arduino library example
}

void loop() // depending on the config parameter sent by Phyphox, an experiment is chosen
{
   if (PhyphoxBLE::currentConnections > 0){
      digitalWrite(ledPin, HIGH);
      digitalWrite(ledPin2, LOW);
      digitalWrite(ledPin3, HIGH);
   }else{
      digitalWrite(ledPin, LOW);
      digitalWrite(ledPin2, HIGH);
      digitalWrite(ledPin3, HIGH);
   }
   
   if(choice == 1.0){
      accelerometerChoice();
   }else if(choice == 2.0){
      gyroscopeChoice();
   }else if(choice == 3.0){
      magnetometerChoice();
   }else if(choice == 4.0){
      pressureChoice();
   }else if(choice == 5.0){
      temperatureChoice();
   }else if(choice == 6.0){
      lightChoice();
   }else if(choice == 9.0) {
      analogChoice();
   }else if(choice == 11.0){
      isochoricChoice();
   }else if(choice == 12.0){
      pressureAvg();
   }else{
   }
}

void receivedData(){ // see Phyphox Arduino Library example
   if (PhyphoxBLE::currentConnections == 1){
      PhyphoxBLE::read(choice); // the "choice" variable is written by our Phyphox experiments
      initial_time = millis();
   }
}

void accelerometerChoice() { // if the sensor is available, the acceleration and a timestamp are written to the BLE server, and then to the Phyphox app
    if (IMU.accelerationAvailable()) {
      first_time = millis();
      IMU.readAcceleration(accx, accy, accz);
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      acc = sqrt(pow(accx, 2) + pow(accy, 2) + pow(accz, 2));
      PhyphoxBLE::write(first_difference_float, accx, accy, accz, acc);
    }
}

void gyroscopeChoice() {
    if (IMU.gyroscopeAvailable()) {
      first_time = millis();
      IMU.readGyroscope(gyrx, gyry, gyrz);
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      gyr = sqrt(pow(gyrx, 2) + pow (gyry, 2) + pow(gyrz, 2));
      PhyphoxBLE::write(first_difference_float, gyrx, gyry, gyrz, gyr);
    }
}

void magnetometerChoice() {
    if (IMU.magneticFieldAvailable()) {
      first_time = millis();
      IMU.readMagneticField(magx, magy, magz);
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      magn = sqrt(pow(magx, 2) + pow (magy, 2) + pow(magz, 2));
      PhyphoxBLE::write(first_difference_float, magx, magy, magz, magn);
    }
}

void pressureChoice() {
    IMU.end();
    if ((unsigned long)(millis() - first_time) > period) { // to make sure that the board does not send too many times the same value
      first_time = millis();
      pressure = BARO.readPressure();
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      PhyphoxBLE::write(first_difference_float, pressure);
    }
    IMU.begin();
}

void temperatureChoice() {
    if ((unsigned long)(millis() - first_time) > period) { // to make sure that the board does not send too many times the same value  
      first_time = millis();
      temperature = HTS.readTemperature();
      humidity = HTS.readHumidity();
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      PhyphoxBLE::write(first_difference_float, temperature, humidity);
    }
}

void lightChoice() {
    if (APDS.colorAvailable()) {
      first_time = millis();
      APDS.readColor(red, green, blue, ambient);
      red_float = (float)red;
      green_float = (float)green;
      blue_float = (float)blue;
      ambient_float = (float)ambient;
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      PhyphoxBLE::write(first_difference_float, ambient_float, red_float, green_float, blue_float);
    }
}

void analogChoice() { // reads the analog input selected at the beginning
    first_time = millis();
    voltage1 = analogRead(analogInPin1);
    voltage2 = analogRead(analogInPin2);
    voltage3 = analogRead(analogInPin3);
    first_difference_float = ((float)first_time-(float)initial_time)/1000;
    PhyphoxBLE::write(first_difference_float, voltage1, voltage2, voltage3);
    delay(2);
}

void isochoricChoice() { // custom experiment :^)
    IMU.end();
    if ((unsigned long)(millis() - first_time) > period) { // to make sure that the board does not send too many times the same value
      first_time = millis();
      temperature = HTS.readTemperature();
      pressure = BARO.readPressure();
      first_difference_float = ((float)first_time-(float)initial_time)/1000;
      PhyphoxBLE::write(first_difference_float, pressure, temperature);
    }
    IMU.begin();
}

void pressureAvg() { // custom experiment :^)
    float pressureVals[5], pressureAverage;
    int i;
    
    IMU.end();
    first_time = millis();
    
    for(i=0;i<5;i++) {
        pressureVals[i] = BARO.readPressure();
        delay(period/2);
    }

    first_difference_float = ((float)first_time-(float)initial_time)/1000;
    pressureAverage = (pressureVals[0] + pressureVals[1] + pressureVals[2] + pressureVals[3] + pressureVals[4])/5.0;
    
    PhyphoxBLE::write(first_difference_float, pressureAverage);
    IMU.begin();
}
