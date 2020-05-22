/*
 * Convert from hex
 */

// Hex byte to 2 ascii byte //
char hexBytes[4];
char* hexByte2string(unsigned char hex)
{
	//char* hexBytes;
	sprintf((char*)(hexBytes), "%02X", hex);
	return hexBytes;
}



// Convert hex to string //
char hexOutput[20];
char* hex2string(unsigned char* input, int len)
{
	//char* hexOutput;
	int loop = 0;
	int i = 0;
	while(loop < len)
	{
		sprintf((char*)(hexOutput+i),"%02X", input[loop]);
		loop+=1;
		i+=2;
	}
	hexOutput[i] = '\0';
	return hexOutput;
}






/*
 * Convert to hex
 */

// Convert to hex //
unsigned char charToHexDigit(char c)
{
//	if (c >= 'A')
//		return c - 'A' + 10;
//	else
//		return c - '0';
	return (c >= 'A') ? c - 'A' + 10 : c - '0';
}
char stringToHexByte(char c[2])
{
	return charToHexDigit(c[0]) * 16 + charToHexDigit(c[1]);
}
