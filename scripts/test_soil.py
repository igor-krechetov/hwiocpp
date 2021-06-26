import time
import Adafruit_ADS1x15


def remap(value, oldMin, oldMax, newMin, newMax):
    isReverse = False
    if oldMin > oldMax:
        temp = oldMin
        oldMin = oldMax
        oldMax = temp
        isReverse = True

    normValue = value - oldMin
    oldRange = oldMax- oldMin
    newRange = newMax- newMin
    valuePercent = normValue / oldRange
    print(valuePercent)

    if isReverse:
        newValue = (newMax + newMin) - (newMin + valuePercent * newRange)
    else:
        newValue = (newMin + valuePercent * newRange)
    return newValue



# Create an ADS1115 ADC (16-bit) instance.
adc = Adafruit_ADS1x15.ADS1115(address=0x48)

# Note you can change the I2C address from its default (0x48), and/or the I2C
# bus by passing in these optional parameters:
#adc = Adafruit_ADS1x15.ADS1015(address=0x49, busnum=1)

# Choose a gain of 1 for reading voltages from 0 to 4.09V.
# Or pick a different gain to change the range of voltages that are read:
#  - 2/3 = +/-6.144V
#  -   1 = +/-4.096V
#  -   2 = +/-2.048V
#  -   4 = +/-1.024V
#  -   8 = +/-0.512V
#  -  16 = +/-0.256V
# See table 3 in the ADS1015/ADS1115 datasheet for more info on gain.
GAIN = 1

ch=3
AirValue = 22000 # humidity 15%, temp 48*
WaterValue = 7963

print('Reading ADS1x15 values, press Ctrl-C to quit...')

print('-' * 37)
# Main loop.
while True:
    soilMoistureValue = adc.read_adc(ch, gain=GAIN)
    soilMoisturePercent = remap(soilMoistureValue, AirValue, WaterValue, 0, 100)
    print(f"moisture={ soilMoisturePercent } %, sensor value={soilMoistureValue}")
    time.sleep(0.5)
