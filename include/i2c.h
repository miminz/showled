#ifndef I2C_H
#define I2C_H

#define I2C_ADDR 0x23

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"{
#endif

int ioctl(int d,int request, ...);

#ifdef __cplusplus
}
#endif

class i2c
{
private:
    char buf[3];
    char val, value;
    float flight;
    int fdi;
    /* data */
public:
    int init_1750(void);
    float get_1750(void);
    i2c(/* args */);
    ~i2c();
};

float i2c::get_1750(void)
{
    if (read(fdi, &buf, 3))
    {
        flight = (buf[0] * 256 + buf[1]) * 0.5 / 1.2;
        printf("光照度: %6.2flx\r\n", flight);
        return flight;
    }
    else
    {
        printf("读取错误\r\n");
        return 10000;
    }
}

int i2c::init_1750(void)
{
    fdi = open("/dev/i2c-0", O_RDWR);
    if (fdi < 0)
    {
        printf("打开文件错误:\r\n");
        return 1;
    }
    if (ioctl(fdi, I2C_SLAVE, I2C_ADDR) < 0)
    {
        printf("ioctl 错误 : \r\n");
        return 1;
    }
    val = 0x01;
    if (write(fdi, &val, 1) < 0)
    {
        printf("上电失败\r\n");
    }
    val = 0x11;
    if (write(fdi, &val, 1) < 0)
    {
        printf("开启高分辨率模式2\r\n");
    }
    usleep(200000);
}

i2c::i2c(/* args */)
{
}

i2c::~i2c()
{
}
#endif
