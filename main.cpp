// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to scroll text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

// For a utility with a few more features see
// ../utils/text-scroller.cc

#include "led-matrix.h"
#include "graphics.h"
#include "i2c.h"
// #include "ShowLed.h"
#include <string>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

#include <wiringSerial.h>
#include <iostream>
#include <iconv.h>

#define ALARM 28
#define TLIGHTS 22
#define RUNLED 11
#define BUTTON 9
struct frame
{
  const char startOne = 0xff;
  const char startTwo = 0xDC;
  const char startOne1 = 0xfe;
  const char startTwo1 = 0xDB;
  const char dataLenJ = 5;
  int dataLen = 0;
  char cmdType = -1;
  char data[1024];
  char crc = 0;
};
struct ledFrame
{
  char lineZ = 255;     //行号，0为全屏显示
  char mode = 0;        //显示模式
  char colour = 0;      //颜色
  char brightness = 50; //亮度
  char ledShowdata[70]; //显示数据
  std::string oneLine;
  std::string twoLine;
  std::string threeLine;
  std::string fourLine;
  std::string allLine = "系统启动中请稍后";
};
using namespace std;
using namespace rgb_matrix;
char flag = 0;
void myInterrupt()
{
  flag++;
}
int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
  iconv_t cd;
  char **pin = &str_str;
  char **pout = &dst_str;

  cd = iconv_open("utf8", "gbk");
  if (cd == 0)
    return -1;
  memset(dst_str, 0, dst_len);
  if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
    return -1;
  iconv_close(cd);
  *pout = "\0";

  return 0;
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

int main(int argc, char *argv[])
{
  i2c i2c;
  i2c.init_1750();
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  matrix_options.hardware_mapping = "regular"; //物理iO
  matrix_options.rows = 32;                    //单模组行
  matrix_options.cols = 64;
  matrix_options.chain_length = 3; //单链模组个数
  matrix_options.parallel = 3;     //物理链个数最大3
  matrix_options.pwm_bits = 3;
  matrix_options.pwm_lsb_nanoseconds = 130;
  matrix_options.pwm_dither_bits = 0;
  matrix_options.brightness = 100; //亮度
  matrix_options.scan_mode = 0;
  matrix_options.row_address_type = 0; //模组扫描行，0为8分之一扫
  matrix_options.multiplexing = 0;     //模组点数据排列方式样品0，英沙32Cf24为1
  matrix_options.disable_hardware_pulsing = false;
  matrix_options.show_refresh_rate = false; //false; //显示刷新率
  matrix_options.inverse_colors = false;    //模组显示反向0亮1不亮
  matrix_options.led_rgb_sequence = "RGB";
  matrix_options.pixel_mapper_config = "";  //排线排列映射
  matrix_options.panel_type = "";           //面板驱动芯片类型，"FM6126A"
  matrix_options.limit_refresh_rate_hz = 0; //限制面板刷新率，0表示不限制
  runtime_opt.daemon = 0;
  runtime_opt.do_gpio_init = true;
  runtime_opt.drop_privileges = 1;
  runtime_opt.gpio_slowdown = 1;
  //matrix_options.
  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);

  const char *bdf_font_file = "/home/2424.bdf"; //字库位置

  wiringPiSetup();
  pinMode(ALARM, OUTPUT);
  pinMode(TLIGHTS, OUTPUT);
  pinMode(RUNLED, OUTPUT);

  pinMode(BUTTON, INPUT);
  pullUpDnControl(BUTTON, PUD_UP);
  if (wiringPiISR(BUTTON, INT_EDGE_RISING, &myInterrupt) < 0)
  {
    printf("Unable to setup ISR \n");
  }
  int fd;
  frame rx;
  ledFrame led;
  unsigned int runLedTime = 0;
  char ledtype = 0;
  if ((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }
  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file))
  {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  RGBMatrix *canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (canvas == NULL)
    return 1;

  // const bool all_extreme_colors = (matrix_options.brightness == 100) && FullSaturation(color) && FullSaturation(&bg_color);
  // if (all_extreme_colors)
  //   canvas->SetPWMBits(1);

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("CTRL-C for exit.\n");

  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();
  rgb_matrix::DrawText(offscreen_canvas, font,
                       0, 24 - 3,
                       color, &bg_color,
                       "系统正在启动中请稍后", 0);
  offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);

  while (!interrupt_received)
  {

    if (serialDataAvail(fd) >= 3)
    {
      unsigned char MsgStart = 0;
      MsgStart = serialGetchar(fd);
      if (rx.startOne == MsgStart || rx.startOne1 == MsgStart) //帧头
      {
        MsgStart = serialGetchar(fd);
        if (rx.startTwo == MsgStart)
        {
          rx.dataLen = serialGetchar(fd) - rx.dataLenJ;
          while (serialDataAvail(fd) < rx.dataLen)
            ; //等待完整的帧
          rx.cmdType = serialGetchar(fd);
          read(fd, &rx.data, rx.dataLen);
          rx.crc = serialGetchar(fd);
          unsigned char crc = rx.startOne ^ rx.startTwo ^ (rx.dataLen + rx.dataLenJ) ^ rx.cmdType;
          for (unsigned char k = 0; k < rx.dataLen;)
          {
            crc = crc ^ rx.data[k];
            k++;
          }
          if (crc != rx.crc) //BCC校验
          {
            write(fd, "crcerr", 6);
            continue;
          }
          switch (rx.cmdType)
          {
          case 'D': //显示命令
            led.lineZ = rx.data[0];
            led.colour = rx.data[1] & 0x0f;
            led.mode = rx.data[1] & 0xf0;
            led.brightness = rx.data[2]; //更新亮度
            memcpy(&led.ledShowdata, &rx.data[3], rx.dataLen - 3);
            led.ledShowdata[rx.dataLen - 3] = 0;
            //led.ledShowdata[rx.dataLen-3+1] = 0;
            break;
          case 'T':
            if (rx.data[0] == 0)
              digitalWrite(TLIGHTS, LOW); //红灯亮
            else
              digitalWrite(TLIGHTS, HIGH); //红灯亮;//绿灯亮
            break;
          case 'A':
            if (rx.data[0] == 0)
              digitalWrite(ALARM, LOW); //黄闪关
            else
              digitalWrite(ALARM, HIGH); //黄闪开
            break;
          case 'C':                                                //清除显示行或全屏
            led.lineZ = rx.data[0];                                //清除显示内容
            memset(&led.ledShowdata, 32, sizeof(led.ledShowdata)); //内容填充空格
            led.ledShowdata[64] = 0;
            //led.ledShowdata[65] = 0;
            break;
          default:
            break;
          }
          // write(fd, &rx.data, rx.dataLen);
        }
        else if (rx.startTwo1 == MsgStart)
        {
          //图片帧
        }
      }
    }

    if (led.lineZ != 255)
    {
      int z = 0;
      if (led.brightness)
        canvas->SetBrightness(led.brightness / 3); //亮度设置要在前，才能当次生效
      else
        canvas->SetBrightness(100); //自动调光

      offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

      while (led.ledShowdata[z] != 0)
        z++;
      char dst_utf8[1024] = {0};
      GbkToUtf8(led.ledShowdata, z, dst_utf8, sizeof(dst_utf8));
      std::string line = reinterpret_cast<char *>(dst_utf8);
      std::cout << line << std::endl;
      int zz = 0;
      char l[17] = {0};
      char a = 1, b = 0;
      switch (led.lineZ)
      {
      case 0://全屏显示

        while (z >= zz)
        {
          for (unsigned char k = 0; k < 16;)
          {

            if (k == 14 && led.ledShowdata[zz] > 0x80)
            {
              l[k] = led.ledShowdata[zz];
              l[k + 1] = led.ledShowdata[zz + 1];

              b = 16;
              zz = zz + 2;
              break;
            }
            if (k == 15 && led.ledShowdata[zz] > 0x80)
            {
              b = 15;
              zz++;
              break;
            }
            l[k] = led.ledShowdata[zz];
            zz++;
            k++;
            b = 16;
          }
          GbkToUtf8(l, b, dst_utf8, sizeof(dst_utf8));
          b = 0;
          std::string linek = reinterpret_cast<char *>(dst_utf8);
          if (a == 1)
            led.oneLine = linek;
          else if (a == 2)
            led.twoLine = linek;
          else if (a == 3)
            led.threeLine = linek;
          else if (a == 4)
            led.fourLine = linek;
          a++;
        }
        break;
      case 1:
        led.oneLine = line;
        break;
      case 2:
        led.twoLine = line;
        break;
      case 3:
        led.threeLine = line;
        break;
      case 4:
        led.fourLine = line;
        break;
      default:
        break;
      }
      std::cout << led.oneLine << std::endl;
      std::cout << led.twoLine << std::endl;
      std::cout << led.threeLine << std::endl;
      std::cout << led.fourLine << std::endl;

      char o = 3;
      rgb_matrix::DrawText(offscreen_canvas, font,
                           0, 24 - o,
                           color, &bg_color,
                           led.oneLine.c_str(), 0);
      rgb_matrix::DrawText(offscreen_canvas, font,
                           0, 48 - o,
                           color, &bg_color,
                           led.twoLine.c_str(), 0);
      rgb_matrix::DrawText(offscreen_canvas, font,
                           0, 72 - o,
                           color, &bg_color,
                           led.threeLine.c_str(), 0);
      rgb_matrix::DrawText(offscreen_canvas, font,
                           0, 96 - o,
                           color, &bg_color,
                           led.fourLine.c_str(), 0);
      write(fd, &led.ledShowdata, z);
      led.lineZ = 255;
      // Swap the offscreen_canvas with canvas on vsync, avoids flickering
      offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
    }

    if (flag)
    {
      while (digitalRead(BUTTON) == 0)
        ;
      if (++ledtype > 5)
        ledtype = 1;
      canvas->SetBrightness(100);
      const int sub_blocks = 16;
      const int width = offscreen_canvas->width();
      const int height = offscreen_canvas->height();
      const int x_step = max(1, width / sub_blocks);
      const int y_step = max(1, height / sub_blocks);
      switch (ledtype)
      {
      case 1:
        offscreen_canvas->Fill(color.r, bg_color.g, bg_color.b);
        break;
      case 2:
        offscreen_canvas->Fill(bg_color.r, color.g, bg_color.b);
        break;
      case 3:
        offscreen_canvas->Fill(color.r, color.g, bg_color.b);
        break;
      case 4:
        offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
        break;

      case 5:

        for (int y = 0; y < height; ++y)
        {
          for (int x = 0; x < width; ++x)
          {
            int c = sub_blocks * (y / y_step) + x / x_step;
            offscreen_canvas->SetPixel(x, y, c, c, c);
          }
        }

        break;
      default:

        break;
      }
      offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
      printf("按下次数%d \n", ledtype);
      flag = 0;
    }

    if (millis() - runLedTime > 1000)
    {
      runLedTime = millis();
      i2c.get_1750();
      // bright = bright + 5;
      // if(bright >= 100)bright = 0;
      // printf("bright= %d\r\n",bright);
      // canvas->SetBrightness(bright); //自动调光
      if (canvas->brightness() < 1)
      {
        canvas->SetBrightness(100);
      }
      else
      {
        canvas->SetBrightness(canvas->brightness() - 1);
      }
      // offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
      printf("亮度：%d\r\n",canvas->brightness());
      offscreen_canvas->Fill(color.r, color.g, color.b);
      offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);

      if (digitalRead(RUNLED) == 0)
        digitalWrite(RUNLED, HIGH);
      else
        digitalWrite(RUNLED, LOW);
    }
    usleep(100);
  }

  // Finished. Shut down the RGB matrix.
  delete canvas;

  return 0;
}
