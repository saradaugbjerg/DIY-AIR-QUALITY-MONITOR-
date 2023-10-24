# DIY-AIR-QUALITY-MONITOR-

A DIY Air Quality Monitor designed and developed by Sara Daugbjerg 

This unit measures air quality with a BME680 sensor. The sensor measures temperature, humidity, and VOC (Volatile Organic Compounds) gases. Indoor temperature should be between 20-22°C for a comfortable working environment. Low humidity can lead to dry mucous membranes, causing fatigue and discomfort, while high humidity can promote mold growth.

Volatile organic compounds (VOC) are gases that evaporate at room temperature. VOC gases can be both human-made and naturally occurring. Human-made VOCs, such as solvents and fuels, can have adverse health effects with prolonged exposure.

In the air quality calculation in this unit, temperature and humidity are given a weight of 10%, while VOC gases are weighted at 80%.

Within the microcontroller, ESP8266, there is a code that receives measurements from the air sensor and assesses its quality. The air quality is then visualized through the color of the Neopixel ring.
 
####WIRING DIAGRAMS 
![](/content/images/2023/10/diagram_bb_bme680.jpg)
![](/content/images/2023/10/diagram_schem_bme680.jpg)
