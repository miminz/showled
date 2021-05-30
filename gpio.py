#6输出NO，NC、20输出NO，7输出指示灯LED.按键输入3
import RPi.GPIO as GPIO
import time
key = 0
led = 3
iotest = [6,20,7]
GPIO.setmode(GPIO.BCM)
GPIO.setup(iotest,GPIO.OUT)
def my_callback(led):
    global key
    key = key + 1
    print(f"广下{key}")

GPIO.setup(led , GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.add_event_detect(led, GPIO.RISING, callback=my_callback, bouncetime=500)
#GPIO.add_event_callback(led , my_callback)#中断，去抖动

while True:
    GPIO.output(iotest,GPIO.HIGH)
    time.sleep(1)
    GPIO.output(iotest,GPIO.LOW)
    time.sleep(1)
GPIO.cleanup()