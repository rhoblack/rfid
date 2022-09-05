#ifndef __SPI_H__
#define __SPI_H__
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <iostream>

class SPI
{
public:

    SPI();
    SPI(int chipSelectPinNum, int spiSpeed);
    ~SPI();
    void spi_init();                        // spi.c line 8
    uint8_t spi_transmit(uint8_t data);     // spi.c line 15
    void sendByte(uint8_t data);
    void sendStream(uint8_t* data, size_t size);
    uint8_t receiveByte();
    void receiveStream(uint8_t* rxData, size_t size);
    void chipSelected();
    void chipDeselected();
    void ENABLE_CHIP(void);
    void DISABLE_CHIP(void);

private:
    const int spi_cs = 10;
    const int spi_channel_0 = 0;
    const int spi_speed = 1000000;
    void txData(int channel, uint8_t* data, size_t size);
};


#endif /* __SPI_H__ */
