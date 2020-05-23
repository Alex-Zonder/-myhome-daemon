// Write to serial port
// Get status on device 01 and port 01
/*
char commStr[8] = "";
commStr[0] = 0xFF;
commStr[1] = 0xFF;
commStr[2] = 0x05;	// Command Code (05 - get output status, 0A - set value)
commStr[3] = 0x01;	// Module address
commStr[4] = 0x00;	// Data
commStr[5] = 0x40;	// Data (Port 0x40 - first for get status) (0x00 - first for set value)
commStr[6] = 0x00;	// Data
commStr[7] = commStr[2] + commStr[3] + commStr[4] + commStr[5] + commStr[6];
*/
/*
$01O01S;	// Get state
$01O01Oxxx;	// Set state
*/




// Convert from CyberLight to Uniel //
char modAddrCh[2];
char modPortCh[2];
unsigned char unielCom[10];
unsigned char* cyber2uniel(char* input)
{
	// Mod Addr (input[1-2]) //
	modAddrCh[0] = input[1];
	modAddrCh[1] = input[2];

	// Command (input[6]) //
	char modComm = 0x05;		// Get status
	//char modComm = 0x0A;		// Set status

	// Port (input[4-5]) //
	modPortCh[0] = '0';
	modPortCh[1] = input[5] - 0x30 - 1 + 0x30;

	// Value //
	unsigned int modValue = 0;


	// Get Value //
	if (input[6] == 'S') {
		modComm = 0x05;		// Command
		modPortCh[0] = '4';	// Port
	}
	// Send Value //
	else if (input[6] == 'O') {
		modComm = 0x0A;		// Command
		modPortCh[0] = '0';	// Port
		modValue = (unsigned int)(input[7]-0x30)*100 + (unsigned int)(input[8]-0x30)*10 + (unsigned int)(input[9]-0x30);
	}


	// Gen String //
	//unsigned char* unielCom;
	unielCom[0] = 0xFF;
	unielCom[1] = 0xFF;

	unielCom[2] = modComm;						// Command Code
	unielCom[3] = stringToHexByte(modAddrCh);	// Module address
	unielCom[4] = modValue;						// Data (Value)
	unielCom[5] = stringToHexByte(modPortCh);	// Data (Port)
	unielCom[6] = 0x00;							// Data

	unielCom[7] = unielCom[2] + unielCom[3] + unielCom[4] + unielCom[5] + unielCom[6];
	unielCom[8] = '\0';
	return unielCom;
}






// Convert to CyberLight //
char cyberCommRet[16];
char portStr[4];
char* uniel2cyber(unsigned char* input)
{
	// Mod Addr (always 0) //
	//modAddrCh[0] = input[1];

	// Command //
	char command = input[2];

	// Port num //
	sprintf(portStr, "%s", hexByte2string(input[5]));
	int port = 0;
	if (command == 0x05) {
		port = (portStr[0] - 0x30 - 4)*10 + (portStr[1] - 0x30 + 1);
	}
	else
	{
		port = (portStr[0] - 0x30)*10 + (portStr[1] - 0x30 + 1);
	}
	sprintf(portStr, "%02d", port);
	//printf("Port:%s\n", portStr);	// Garbage

	// Port data //
	unsigned int pVal = input[4];
	//printf("Val:%i\n", pVal);	// Garbage

	sprintf(cyberCommRet, "#01O%s:%d;", portStr, pVal);
	return cyberCommRet;
}






// Check Uniel Checksum //
int unielChecksum(unsigned char* input)
{
	if (input[2] + input[3] + input[4] + input[5] + input[6] == input[7])
		return 1;
	return 0;
}

// Check input data for uniel //
char*  unielCheck(unsigned char* input, int len)
{
	// if (unielChecksum(input)) {
		char* hexStr = hex2string(input, 8);
		char* toRet = uniel2cyber(input);
		printf("Read %i bytes: %s -> %s\n", len, hexStr, toRet);
		return toRet;
	// }
}
