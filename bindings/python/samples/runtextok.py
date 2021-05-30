#!/usr/bin/env python3
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
from rgbmatrix import RGBMatrix, RGBMatrixOptions

import time


class MSerialPort:
    message = []
    Sport_in = 0
    Sport_out = 0

    def __init__(self, port, buand):
        self.port = serial.Serial(port, buand)
        if not self.port.isOpen():
            self.port.open()

    def port_open(self):
        if not self.port.isOpen():
            self.port.open()

    def port_close(self):
        self.port.close()

    def send_data(self, data):
        number = self.port.write(data)
        return number

    def read_data(self):
        while True:
            time.sleep(0.001)
            data = self.port.read()
            if data != '':
                self.message.append(data)


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="Hello kdasworld!fsdfsasdfsadfsadsadf")



    def run(self):

        options = RGBMatrixOptions()

        options.chain_length = 2
        options.inverse_colors = "BGR"
        options.brightness = 30
        options.rows = 32 #单模组高分辨率
        options.cols = 64 #单模组宽分辨率
        options.chain_length = 1 #单物理接口模组数量
        options.parallel = 3 #物理接口数量
        options.pwm_bits = 11 #灰度级别
        options.multiplexing = 0 #模组扫描方式

        offscreen_canvas = self.matrix.CreateFrameCanvas(options = options)

        font = graphics.Font()
        #fonty = graphics.Font()
        font.LoadFont("/home/matrix-master/fonts/2424.bdf")
        #font.LoadFont("./fonts/32x32h.BDF")
        textColor = graphics.Color(255, 0, 0)
        pos = offscreen_canvas.width
        print(pos)
        my_text = self.args.text
        color = 0
        brightness = 0
        text = "欢迎行驶高速公路   "
        while True:
            offscreen_canvas.Clear()
            # l = lenmSerial.message
            # if (l > 0 ):
            #     text = mSerial.message
            #     del mSerial.message[0:-1]

            len = graphics.DrawText(offscreen_canvas, font,0, 20, textColor, text)
            graphics.DrawText(offscreen_canvas, font,32*3, 20, textColor, text)

            graphics.DrawText(offscreen_canvas, font, 32 * 6, 20, textColor, text)


    # k=graphics.DrawText(offscreen_canvas, fonty, pos+len, 30, textColor, str.join(text))
            pos -= 1
            if (pos + len < 0):
                pos = offscreen_canvas.width

            color += 1
            textColor=graphics.Color(255,color,0)
            if(color>254):
                color=0
            brightness += 1
            if (brightness > 100):
                brightness = 0
            self.matrix.brightness = brightness

            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)
            #tow_canvas = self.matrix.SwapOnVSync(tow_canvas)

import _thread
import serial




# Main function
if __name__ == "__main__":
    mSerial = MSerialPort("/dev/ttyAMA0", 115200)  #
    _thread.start_new_thread(mSerial.read_data, ())
    run_text = RunText()
    if (not run_text.process()):
        run_text.print_help()
