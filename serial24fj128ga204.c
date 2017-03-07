/*PIC16F Serial Coms Routine (8bits, 1 Stop bit, No Parity) Bidirectional. TX and RX,

 Rx on RB2 pin 23
 Tx on RB3 pin 24
 */

void serial_setup(unsigned int Baud_Rate);
void putch(unsigned char c);
unsigned char getch(void);
void putst(register const char *str);
void putchhex(unsigned char c);

void serial_setup(unsigned int Baud_Rate) {

    unsigned long FP=FCY/2;
    unsigned int BRGVAL =0;
    
    BRGVAL =((FP/Baud_Rate)/16)-1; //calculate Baud Rate, FP is Half of the System Clock, must be set in main code

    RPINR18bits.U1RXR = 2; //UART1 receive set to RB2
    RPOR1bits.RP3R = 3; //UART1 transmit set to RB3

    U1MODEbits.STSEL = 0; // 1-Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 0; // Standard-Speed mode
    U1BRG = BRGVAL; // Baud Rate
    U1STAbits.UTXISEL0 = 0; // Interrupt after one TX character is transmitted
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1TXIE = 0; // Disable UART TX interrupt
    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX
    U1STAbits.URXEN = 1; // Enable UART RX
    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag

}

//writes a character to the serial port

void put_serial_ch(unsigned char c) {
    while (U1STAbits.UTXBF == 1);
    U1TXREG = c; // Transmit one character
    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
}

//gets a character from the serial port without timeout

unsigned char get_serial_ch(void) {
    while (!U1STAbits.URXDA) { //wait for Buffer to be full;
    }
    IFS0bits.U1RXIF = 0;
    return U1RXREG;

}

void put_serial_st(char *str) {
    while ((*str) != 0) {
        put_serial_ch(*str);
        if (*str == 13) put_serial_ch(10);
        if (*str == 10) put_serial_ch(13);
        str++;
    }
}

void putchhex(unsigned char c)
{
	unsigned char temp;

	// transmits in hex

	temp=c;

	c=(c >> 4);
	if (c<10) c+=48; else c+=55;
	put_serial_ch(c);

	c=temp;

	c=(c & 0x0F);
	if (c<10) c+=48; else c+=55;
	put_serial_ch(c);
}
