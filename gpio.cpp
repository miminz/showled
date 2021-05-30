#include "led-matrix.h"
#include "graphics.h"

#include <string>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <unistd.h>
#include <wiringSerial.h>
#include <iostream>
#include <iconv.h>

#define ALARM 28
#define TLIGHTS 22
#define RUNLED 11
#define BUTTON 9
unsigned int buttonTime = 0;
unsigned int runLedTime = 0;
unsigned char buttonType = 0;
char ledtype = 0;
volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

char flag = 0;
void myInterrupt()
{
    flag++;
}
int main(int argc, char *argv[])
{
    wiringPiSetup();
    pinMode(ALARM, OUTPUT);
    pinMode(TLIGHTS, OUTPUT);
    pinMode(RUNLED, OUTPUT);
    pinMode(BUTTON, INPUT);
    // pullUpDnControl(BUTTON, PUD_UP);
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    printf("CTRL-C for exit.\n");
    if (wiringPiISR(BUTTON, INT_EDGE_RISING, &myInterrupt) < 0)
    {
        printf("Unable to setup ISR \n");
    }
    while (!interrupt_received)
    {

        if (millis() - runLedTime > 500)
        {
            runLedTime = millis();
            if (digitalRead(RUNLED) == 0)
                digitalWrite(RUNLED, HIGH);
            else
                digitalWrite(RUNLED, LOW);
        }
        if (flag)
        {
            while (digitalRead(BUTTON) == 0)
                ;

                    if (ledtype++ > 3)
                        ledtype = 0;
                buttonType=0;
            printf("按下次数%d \n", ledtype);            
            flag = 0;
        }
        usleep(10);
    }
}