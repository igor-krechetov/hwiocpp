import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

pins = (3,5,7,8,10,11,12,13,15,16,18,19,21,22,23,24,26,29,31,32,33,35,36,37,38,40)

for pin in range (0,26):
    GPIO.setup(pins[pin], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    # print ('pin: '+ str(pins[pin]), GPIO.input(pins[pin]))
print("done")