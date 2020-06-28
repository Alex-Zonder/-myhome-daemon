/*
Modbusrtu command
00 - addres
01 - command
02 - Hi Адрес регистра
03 - Lo Адрес регистра
04 - Hi Количество регистров
05 - Lo Количество регистров
06 - Hi CRC
07 - Lo CRC

Command byte (01)
01 (0x01) - Чтение дискретного вывода
02 (0x02) - Чтение дискретного ввода
03 (0x03) - Чтение аналогового вывода (01 03 00 00 00 01 840A)
04 (0x03) - Чтение аналогового ввода
05 (0x05) - Запись дискретного вывода
06 (0x06) - Запись аналогового вывода (01 06 00 01 01 00 D99A)
15 (0x0F) - Запись нескольких дискретных выводов
16 (0x10) - Запись нескольких аналоговых выходов
 */

/*
Запись аналогового вывода (01 06 00 01 01 00 D99A)
02 - Hi адрес порта
03 - Lo адрес порта
04 - Change port 01 or 02
Смена адреса устройства (02 06 00 FF 00 04 B80A)
02 - Hi адрес
03 - Lo адрес порта (0xFF - slave adress)
04 - Hi адрес
05 - Lo адрес - New address
 */



//   ModBusRtu CRC   //
int crc_chk ( unsigned char* data, unsigned char length )
{
    register unsigned int reg_crc = 0XFFFF;
    register int j;
    while (length--)
    {
        reg_crc ^= *data++;
        for (j=0; j<8; j++)
        {
            reg_crc = reg_crc & 0x01 ? (reg_crc >> 1) ^ 0xA001 : reg_crc >> 1;
        }
    }
    return reg_crc;
}



unsigned char mobusrtuData[32] = "";
unsigned char* cyber2modbusrtu(char* input) {
    //   Address   //
    mobusrtuData[0] = (input[1] - 0x30) * 10 + (input[2] - 0x30); // 00 - address

    // Get state //
    if (input[6] == 'S') {
        //   Command   //
        mobusrtuData[1] = 3; // 01 - command

        //   Data   //
        mobusrtuData[2] = 0; // 02 - Hi Адрес регистра
        mobusrtuData[3] = ((input[4] - 0x30) * 10) + (input[5] - 0x30); // 03 - Lo Адрес регистра

        mobusrtuData[4] = 0; // 04 - Hi Количество регистров
        mobusrtuData[5] = 0x01; // 05 - Lo Количество регистров
    }
    // Change state //
    if (input[6] == 'N' || input[6] == 'F') {
        //   Command   //
        mobusrtuData[1] = 6; // 01 - command

        //   Data   //
        mobusrtuData[2] = 0; // 02 - Hi Адрес регистра
        mobusrtuData[3] = ((input[4] - 0x30) * 10) + (input[5] - 0x30); // 03 - Lo Адрес регистра

        mobusrtuData[4] = input[6] == 'N' ? 1 : 2; // 04 - Change port 01 or 02
        mobusrtuData[5] = 0; // 05 - Lo Количество регистров
    }

    //   CRC   //
    int crc = crc_chk(mobusrtuData, 6);
    // printf("%02X\n", crc);
    unsigned char *rep = (unsigned char *)&crc;
    // printf("%s\n", hex2string(rep,2));
    mobusrtuData[6] = rep[0]; // 06 - Hi CRC
    mobusrtuData[7] = rep[1]; // 07 - Lo CRC
    //   Return   //
    return mobusrtuData;
}
