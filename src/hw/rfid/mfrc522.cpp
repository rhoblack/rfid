/*
RFID-RC522  
SDA  ----- SS, chip select
SCK  ----- SCK
MOSI ----- MOSI
MISO ----- MISO
IRQ  ----- N.C
GND  ----- GND
RST  ----- +3.3V
VCC  ----- +3.3v
*/



#include "mfrc522.h"
#include <iostream>

mfrc522::mfrc522()
{
}

mfrc522::mfrc522(SPI *spi)
    : mfrcSpi(spi)
{
    uint8_t byte;
    mfrc522_reset();
    mfrc522_write(TModeReg, 0x8D);
    mfrc522_write(TPrescalerReg, 0x3E);
    mfrc522_write(TReloadReg_1, 30);
    mfrc522_write(TReloadReg_2, 0);
    mfrc522_write(TxASKReg, 0x40);
    mfrc522_write(ModeReg, 0x3D);

    byte = mfrc522_read(TxControlReg);
    if (!(byte & 0x03))
    {
        mfrc522_write(TxControlReg, byte | 0x03);
    }
}

mfrc522::~mfrc522()
{
}

void mfrc522::mfrc522_write(uint8_t reg, uint8_t data)
{
    uint8_t buff[2];
    buff[0] = (reg << 1) & 0x7E;
    buff[1] = data;
    mfrcSpi->sendStream(buff,2);
}

uint8_t mfrc522::mfrc522_read(uint8_t reg)
{
    uint8_t buff[2];

    buff[0] = ((reg << 1) & 0x7E) | 0x80;
    mfrcSpi->receiveStream(buff, 2);

    return buff[1];
}

void mfrc522::mfrc522_reset()
{
    mfrc522_write(CommandReg, SoftReset_CMD);
}

uint8_t mfrc522::mfrc522_request(uint8_t req_mode, uint8_t *tag_type)
{
    uint8_t status;
    uint32_t backBits; // The received data bits

    mfrc522_write(BitFramingReg, 0x07); // TxLastBists = BitFramingReg[2..0]	???

    tag_type[0] = req_mode;
    status = mfrc522_to_card(Transceive_CMD, tag_type, 1, tag_type, &backBits);
    //printf("status : 0x%02x, backBits : 0x%02x\n", status, backBits);
    if ((status != CARD_FOUND) || (backBits != 0x10))
    {
        status = ERROR;
    }

    return status;
}

uint8_t mfrc522::mfrc522_to_card(uint8_t cmd, uint8_t *send_data, uint8_t send_data_len, uint8_t *back_data, uint32_t *back_data_len)
{
    uint8_t status = ERROR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint8_t tmp;
    uint32_t i;

    switch (cmd)
    {
    case MFAuthent_CMD: // Certification cards close
    {
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
    }
    case Transceive_CMD: // Transmit FIFO data
    {
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    }
    default:
        break;
    }

    // mfrc522_write(ComIEnReg, irqEn|0x80);	//Interrupt request
    n = mfrc522_read(ComIrqReg);
    mfrc522_write(ComIrqReg, n & (~0x80)); // clear all interrupt bits
    n = mfrc522_read(FIFOLevelReg);
    mfrc522_write(FIFOLevelReg, n | 0x80); // flush FIFO data

    mfrc522_write(CommandReg, Idle_CMD); // NO action; Cancel the current cmd???

    // Writing data to the FIFO
    for (i = 0; i < send_data_len; i++)
    {
        mfrc522_write(FIFODataReg, send_data[i]);
    }

    // Execute the cmd
    mfrc522_write(CommandReg, cmd);
    if (cmd == Transceive_CMD)
    {
        n = mfrc522_read(BitFramingReg);
        mfrc522_write(BitFramingReg, n | 0x80);
    }

    // Waiting to receive data to complete
    i = 2000; // i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
    do
    {
        // CommIrqReg[7..0]
        // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = mfrc522_read(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    tmp = mfrc522_read(BitFramingReg);
    mfrc522_write(BitFramingReg, tmp & (~0x80));

    if (i != 0)
    {
        if (!(mfrc522_read(ErrorReg) & 0x1B)) // BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = CARD_FOUND;
            if (n & irqEn & 0x01)
            {
                status = CARD_NOT_FOUND; //??
            }

            if (cmd == Transceive_CMD)
            {
                n = mfrc522_read(FIFOLevelReg);
                lastBits = mfrc522_read(ControlReg) & 0x07;
                if (lastBits)
                {
                    *back_data_len = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *back_data_len = n * 8;
                }

                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAX_LEN)
                {
                    n = MAX_LEN;
                }

                // Reading the received data in FIFO
                for (i = 0; i < n; i++)
                {
                    back_data[i] = mfrc522_read(FIFODataReg);
                }
            }
        }
        else
        {
            status = ERROR;
        }
    }

    // SetBitMask(ControlReg,0x80);           //timer stops
    // mfrc522_write(cmdReg, PCD_IDLE);

    return status;
}
uint8_t mfrc522::mfrc522_get_card_serial(uint8_t *serial_out)
{
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint32_t unLen;

    mfrc522_write(BitFramingReg, 0x00); // TxLastBists = BitFramingReg[2..0]

    serial_out[0] = PICC_ANTICOLL;
    serial_out[1] = 0x20;
    status = mfrc522_to_card(Transceive_CMD, serial_out, 2, serial_out, &unLen);

    if (status == CARD_FOUND)
    {
        // Check card serial number
        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serial_out[i];
        }
        if (serNumCheck != serial_out[i])
        {
            status = ERROR;
        }
    }
    return status;
}
