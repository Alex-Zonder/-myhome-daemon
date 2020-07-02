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
int mrtGenCrc (unsigned char* data, unsigned char length)
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
int mrtChkCrc (unsigned char* input, unsigned char length)
{
    int crc = mrtGenCrc((unsigned char*)input, length - 2);
    unsigned char *crcBytes = (unsigned char *)&crc;
    // CRC OK //
    if (input[length - 2] == crcBytes[0] && input[length - 1] == crcBytes[1]) {
        return 1;
    }
    else return 0;
}


// Convert from CyberLight to Modbus RTU //
unsigned char mobusrtuData[32] = "";
int cyber2modbusrtu(char* input) {
    int commLen = 0;

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
        commLen = 6;
    }
    // Get all state //
    else if (input[3] == 'Z') {
        //   Command   //
        mobusrtuData[1] = 3; // 01 - command

        //   Data   //
        mobusrtuData[2] = 0; // 02 - Hi Адрес регистра
        mobusrtuData[3] = 1; // 03 - Lo Адрес регистра

        mobusrtuData[4] = 0; // 04 - Hi Количество регистров
        mobusrtuData[5] = 8; // 05 - Lo Количество регистров
        commLen = 6;
    }
    // Change state //
    else if (input[6] == 'N' || input[6] == 'F') {
        //   Command   //
        mobusrtuData[1] = 6; // 01 - command

        //   Data   //
        mobusrtuData[2] = 0; // 02 - Hi Адрес регистра
        mobusrtuData[3] = ((input[4] - 0x30) * 10) + (input[5] - 0x30); // 03 - Lo Адрес регистра

        mobusrtuData[4] = input[6] == 'N' ? 1 : 2; // 04 - Change port 01 or 02
        mobusrtuData[5] = 0; // 05 - Lo Количество регистров
        commLen = 6;
    }

    //   CRC   //
    int crc = mrtGenCrc(mobusrtuData, commLen);
    // printf("%02X\n", crc);
    unsigned char *rep = (unsigned char *)&crc;
    // printf("%s\n", hex2string(rep,2));
    mobusrtuData[commLen++] = rep[0]; // 06 - Hi CRC
    mobusrtuData[commLen++] = rep[1]; // 07 - Lo CRC
    mobusrtuData[commLen] = '\0';
    //   Return   //
    return commLen;
}


// Convert from Modbus RTU to CyberLight //
char cyberCommRet[16];
char* modbusrtu2cyber(unsigned char* input, int len) {
    static char addr[4] = "";
    sprintf(addr, "");
    char command = '\0';
    static char data[32] = "";
    sprintf(data, "");

    // CRC OK //
    if (mrtChkCrc((unsigned char*)input, len)) {
        // printf("CRC-OK\n");
        // Address //
        addr[0] = (char)(input[0]/10%10)+'0';
        addr[1] = (char)(input[0]%10)+'0';

        // Command //
        command = input[1] == 6 ? 'R' : 'Z';
        if (command == 'Z') {
            sprintf(data, ":R");
            int dataLen = input[2] / 2;
            for (int c=0; c<dataLen; c++)
            {
                char bt[2] = "";
                bt[0] = input[4+c*2]+'0';
                strcat(data, bt);
            }
        }
        else
        {
            sprintf(data, "%c%c%c",
                (char)(input[3]/10%10)+'0', // 3 - port
                (char)(input[3]%10)+'0',
                input[4] == 2 ? 'F' : 'N' // 4 - state (2-off 1-on)
            );
        }
    }
    // CRC ERROR //
    else {
        printf("CRC-Error\n");
        sprintf(data, "CRC-Error");
    }

    // Return //
    sprintf(cyberCommRet, "#%s%c%s;", addr, command, data);
    return cyberCommRet;
}
