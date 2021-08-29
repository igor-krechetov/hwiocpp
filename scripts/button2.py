from signal import pause
from time import sleep
from gpiozero import Button

blink_on = False
interval = 0.5

button = Button(19)
# led1 = LED(26)
# led2 = LED(19)

# def go_blink():
    # global blink_on

    # if blink_on:
    #     led1.off()
    #     led2.off()
    # else:
    #     led1.blink(interval, interval)
    #     sleep(interval)
    #     led2.blink(interval, interval)

    # blink_on = not blink_on

def on_pressed():
    print("pressed")


try:
    button.when_pressed = on_pressed
    pause()

except KeyboardInterrupt:
    pass

finally:
    # led1.close()
    # led2.close()
    print("done")