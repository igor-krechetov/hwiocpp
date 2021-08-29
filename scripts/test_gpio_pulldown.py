import sys
import time
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

pin = int(sys.argv[1])
GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
# GPIO.setup(pin, GPIO.IN)

for i in range(20):
    print ('pin: ' + str(pin), GPIO.input(pin))
    time.sleep(1)

GPIO.cleanup()

print("done")