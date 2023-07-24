#include "mbed.h"
#include "hcsr04.h" //超音波センサ
#include "Dht11.h"  //
#include"Air_Quality.h"
#include "MPL3115A2.h"//大気圧


RawSerial pc(USBTX, USBRX); 
HCSR04  usensor(p21,p12);//超音波センサ p21:trig p12:echo

DigitalOut led0(LED1), led25(LED2), led50(LED3), led75(LED4);//光センサ
AnalogIn temt6000(p18);//光センサ
I2C i2c(p28, p27);       // sda, scl

MPL3115A2 psensor(&i2c, &pc);//気圧
 
DigitalOut myled(LED1);     // Sanity check to make sure the program is working.
DigitalOut powerPin(p21);   // <-- I powered the sensor from a pin. You don't have to.
AnalogIn sensorUV(p15);//紫外線
 
//空気清浄度
AirQuality airqualitysensor;
int current_quality = -1;
PinName analogPin = p15;
// temt6000 breakout: VCC(to VOUT=3.3V), GND(to GND), SIG(to p15) 


// Interrupt Handler
void AirQualityInterrupt()
{
    AnalogIn sensor(analogPin);
    airqualitysensor.last_vol = airqualitysensor.first_vol;
    airqualitysensor.first_vol = sensor.read()*1000;
    airqualitysensor.timer_index = 1;
}
Dht11 sensor(p15);//湿度
//a:Uv
//b:超音波
//c:温湿度
//d:光
//e:air
//f:気圧
int main() {
    char c;
    int temp,humid;
    float UVvalue;
    int dist;
    float x = temt6000;
    
    
    while (true) {
        pc.printf("-- enter any key --> ");
        c = pc.getc();
        if (c=='a'){//紫外線
            for (int i=0;i<5;i++){
                UVvalue = sensorUV;
                printf("\rUV Value = %3.2f%%\r\n",UVvalue*100);
                wait(0.5);
            }
        }
        
        else if(c=='b'){//超音波
            for (int i=0;i<5;i++){
                usensor.start();
                dist=usensor.get_dist_cm();
                pc.printf("distance = %d [cm]\r\n",dist);
                wait(0.5);
            }
        }
        
        else if(c=='c'){//温・湿度
            for (int i=0;i<5;i++){
                sensor.read();
                temp = sensor.getFahrenheit();
                humid = sensor.getHumidity();
                temp = (temp - 32.0)/1.80;    // Fahrenheit --> Celsius
                pc.printf("T: %d, H: %d\r\n", temp, humid);
                wait(0.5);
            }
        }

        else if(c=='d'){//光
            for (int i=0;i<5;i++){
                // four LEDs meaning "light level" (1 to 4):
                led0 = 1;
                led25 = led50 = led75 = 0;
                if(x>0.25) led25 = 1;
                if(x>0.50) led50 = 1;
                if(x>0.75) led75 = 1;
                
                printf("%f\r\n", x);
                wait(0.50);
            }
        }
        else if(c=='e'){//空気
            airqualitysensor.init(analogPin, AirQualityInterrupt);
            for (int i=0;i<5;i++){
                current_quality=airqualitysensor.slope();
                if (current_quality >= 0) { // if a valid data returned.
                    if (current_quality == 0)
                        printf("High pollution! Force signal active\n\r");
                    else if (current_quality == 1)
                        printf("High pollution!\n\r");
                    else if (current_quality == 2)
                        printf("Low pollution!\n\r");
                    else if (current_quality == 3)
                        printf("Fresh air\n\r");
                }
            }
            
        }else if(c=='f'){
            
            powerPin = 1;
            wait_ms(300);

            pc.printf("** MPL3115A2 SENSOR **\r\n");
            psensor.init();

            Altitude a;
            Temperature t;
            Pressure p;

            // Offsets for Dacula, GA
            psensor.setOffsetAltitude(83);
            psensor.setOffsetTemperature(20);
            psensor.setOffsetPressure(-32);
                
            for (int i=0;i<5;i++){
                psensor.readAltitude(&a);
                psensor.readTemperature(&t);
                
                psensor.setModeStandby();
                psensor.setModeBarometer();
                psensor.setModeActive();
                psensor.readPressure(&p);
                
                pc.printf("Altitude: %sft, Temp: %sdegF, Pressure: %sPa\r\n", a.print(), t.print(), p.print());
                
                psensor.setModeStandby();
                psensor.setModeAltimeter();
                psensor.setModeActive();
                
                wait(1.0);
            }
        }
        
        
    }
    
}





