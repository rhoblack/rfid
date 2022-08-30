#include "Spi.h"

Spi::Spi(int chipSelectPinNum, int spiSpeed)
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

Spi::~Spi()
{
    
}

void Spi::chipSelected()
{
    digitalWrite(spi_cs, LOW);
}

void Spi::chipDeselected()
{
    digitalWrite(spi_cs, HIGH);
}

void Spi::txData(int channel, uint8_t* data, size_t size)
{
    wiringPiSPIDataRW(channel, data, size);
}

uint8_t Spi::spi_transmit(uint8_t data)
{
    txData(spi_channel_0, &data, 1);

    return data;
}

void Spi::sendByte(uint8_t data)
{
    chipSelected();
    txData(spi_channel_0, &data, 1);
    chipDeselected();
}

void Spi::sendStream(uint8_t* data, size_t size)
{
    chipSelected();
    txData(spi_channel_0, data, size);
    chipDeselected();
}

uint8_t Spi::receiveByte()
{
    uint8_t tempData;
    chipSelected();
    txData(spi_channel_0, &tempData, 1);
    chipDeselected();

    return tempData;
}

void Spi::receiveStream(uint8_t* rxData, size_t size)
{
    chipSelected();
    txData(spi_channel_0, rxData, size);
    chipDeselected();
}

void Spi::ENABLE_CHIP(void)
{
    chipSelected();
}

void Spi::DISABLE_CHIP(void)
{
    chipDeselected();
}
