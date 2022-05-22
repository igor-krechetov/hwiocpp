#              .';:cc;.
#            .,',;lol::c.
#            ;';lddddlclo
#            lcloxxoddodxdool:,.
#            cxdddxdodxdkOkkkkkkkd:.
#          .ldxkkOOOOkkOO000Okkxkkkkx:.
#        .lddxkkOkOOO0OOO0000Okxxxxkkkk:
#       'ooddkkkxxkO0000KK00Okxdoodxkkkko
#      .ooodxkkxxxOO000kkkO0KOxolooxkkxxkl
#      lolodxkkxxkOx,.      .lkdolodkkxxxO.
#      doloodxkkkOk           ....   .,cxO;
#      ddoodddxkkkk:         ,oxxxkOdc'..o'
#      :kdddxxxxd,  ,lolccldxxxkkOOOkkkko,
#       lOkxkkk;  :xkkkkkkkkOOO000OOkkOOk.
#        ;00Ok' 'O000OO0000000000OOOO0Od.
#         .l0l.;OOO000000OOOOOO000000x,
#            .'OKKKK00000000000000kc.
#               .:ox0KKKKKKK0kdc,.
#                      ...
#
# Author: peppe8o
# Blog: https://peppe8o.com
# Date: Nov 8th, 2020
# Version: 1.0

import RPi.GPIO as GPIO
import sys
import time

#define PINs according to cabling
dataPIN = 16
latchPIN = 21
clockPIN = 20

#set pins to putput
GPIO.setmode(GPIO.BCM)
GPIO.setup((dataPIN,latchPIN,clockPIN),GPIO.OUT)

#define shift register update function
def shift_update(input,data,clock,latch):
  #put latch down to start data sending
  GPIO.output(clock,0)
  # time.sleep(0.5)
  GPIO.output(latch,0)
  # time.sleep(0.5)
  GPIO.output(clock,1)
  # time.sleep(0.5)

  print(input)
  
  #load data in reverse order
  for i in range(7, -1, -1):
    GPIO.output(clock,0)
    # time.sleep(0.5)
    GPIO.output(data, int(input[i]))
    # time.sleep(0.5)
    GPIO.output(clock,1)
    # time.sleep(0.5)
  
  #put latch up to store data on register
  GPIO.output(clock,0)
  # time.sleep(0.5)
  GPIO.output(latch,1)
  # time.sleep(0.5)
  GPIO.output(clock,1)
  # time.sleep(0.5)


#main program, calling shift register function
#uses "sys.argv" to pass arguments from command line
shift_update(sys.argv[1],dataPIN,clockPIN,latchPIN)
# print("sleep 10...")
# time.sleep(10.0)

#PINs final cleaning
GPIO.cleanup()
