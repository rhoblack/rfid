#include "SPI.h"

SPI::SPI(int chipSelectPinNum, int spiSpeed)
    : spi_cs(chipSelectPinNum), spi_speed(spiSpeed)
{
    if (wiringPiSetup() < 0) {
        printf("Unable to start wiringPi: \n");
        exit(1);
    }

    pinMode(spi_cs, OUTPUT);

    if (wiringPiSPISetup(spi_channel_0, spi_speed) < 0) {
        printf("wiringPiSPISetup Failed:\n");
        exit(1);
    }
}

SPI::~SPI()
{
    
}

void SPI::chipSelected()
{
    digitalWrite(spi_cs, LOW);
}

void SPI::chipDeselected()
{
    digitalWrite(spi_cs, HIGH);
}

void SPI::txData(int channel, uint8_t* data, size_t size)
{
    wiringPiSPIDataRW(channel, data, size);
}

uint8_t SPI::spi_transmit(uint8_t data)
{
    txData(spi_channel_0, &data, 1);

    return data;
}

void SPI::sendByte(uint8_t data)
{
    chipSelected();
    txData(spi_channel_0, &data, 1);
    chipDeselected();
}

void SPI::sendStream(uint8_t* data, size_t size)
{
    chipSelected();
    txData(spi_channel_0, data, size);
    chipDeselected();
}

uint8_t SPI::receiveByte()
{
    uint8_t tempData;
    chipSelected();
    txData(spi_channel_0, &tempData, 1);
    chipDeselected();

    return tempData;
}

void SPI::receiveStream(uint8_t* rxData, size_t size)
{
    chipSelected();
    txData(spi_channel_0, rxData, size);
    chipDeselected();
}

void SPI::ENABLE_CHIP(void)
{
    chipSelected();
}

void SPI::DISABLE_CHIP(void)
{
    chipDeselected();
}
