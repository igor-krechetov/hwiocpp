#!/usr/bin/python3

#https://github.com/Thinary/AHT10/blob/master/src/Thinary_AHT10.cpp
#https://myhydropi.com/raspberry-pi-i2c-temperature-sensor
#i2cdetect -y 0

import smbus
import time

# Get I2C bus
bus = smbus.SMBus(1)
config = [0x08, 0x00]
bus.write_i2c_block_data(0x38, 0xE1, config)
time.sleep(0.5)
byt = bus.read_byte(0x38)

print(f"Config -> {byt}")
print(f"Config -> {byt&0x68}")

while True:
  #print(byt&0x68)
  MeasureCmd = [0x33, 0x00]
  bus.write_i2c_block_data(0x38, 0xAC, MeasureCmd)
  time.sleep(0.5)
  data = bus.read_i2c_block_data(0x38,0x00)

  print(f"DUMP ({len(data)}): {data}")
  temp = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5]
  ctemp = ((temp*200) / 1048576) - 50
  print(u'Temperature: {0:.1f}C'.format(ctemp))
  tmp = ((data[1] << 16) | (data[2] << 8) | data[3]) >> 4

  #print(tmp)
  ctmp = int(tmp * 100 / 1048576)
  print(u'Humidity: {0}%'.format(ctmp))

  time.sleep(1) 
