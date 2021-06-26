import sys
import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)  # GPIO Numbers instead of board numbers

if len(sys.argv) == 2:
    RELAIS_1_GPIO = int(sys.argv[1])
    GPIO.setup(RELAIS_1_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)  # GPIO Assign mode
    print(f"GPIO {RELAIS_1_GPIO} = f{GPIO.input(RELAIS_1_GPIO)}")
elif len(sys.argv) == 3:
    RELAIS_1_GPIO = int(sys.argv[1])
    # newValue = int(sys.argv[2])
    timeout = int(sys.argv[2])

    GPIO.setup(RELAIS_1_GPIO, GPIO.OUT)  # GPIO Assign mode
    GPIO.output(RELAIS_1_GPIO, GPIO.HIGH)  # initial OFF
    GPIO.output(RELAIS_1_GPIO, GPIO.LOW)  # out
    time.sleep(timeout)

GPIO.cleanup()