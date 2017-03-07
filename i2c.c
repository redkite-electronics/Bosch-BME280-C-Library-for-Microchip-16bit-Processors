/*
 IC2 Master Base Routines for PIC24FJ128GA204 Processor
 Taken from Endscope
 I. Stewart
 October 2016
 IC2 Port pins SCL1 44, SDA1 1
 This Code Works with a DS1307 RTC chip at 100khz with 2k2 pull up resistors 5/11/2016
 This Code Works with a BME270 05/01/17
 */

#define Fsck	400000 //set to I2C Bus Speed, 400kHz in this case
#define I2C_BRG	((FCY/2/Fsck)-1)


typedef struct doublechar
{
char x, y;
}dchar;

void reset_i2c_bus(void);
void i2c_init(int BRG);
void i2c_start(void);
void i2c_restart(void);
unsigned char send_i2c_byte(unsigned char data);
unsigned char send_i2c_byte_ack(unsigned char data);
unsigned char i2c_read(void);
unsigned char i2c_read_ack(void);
void I2Cwrite(unsigned char addr, unsigned char subaddr, unsigned char value);
unsigned char I2Cread(unsigned char addr, unsigned char subaddr);
void I2Cwritedouble(unsigned char addr, unsigned char subaddr, unsigned char valuelow, unsigned char valuehigh);
struct doublechar I2Creaddouble(unsigned char addr, unsigned char subaddr);

//Resets the I2C bus to Idle

void reset_i2c_bus(void) {
    unsigned int x = 0;
    //initiate stop bit
    I2C1CONLbits.PEN = 1;
    //wait for hardware clear of stop bit
    while (I2C1CONLbits.PEN) {
        //DelayuSec(1);
         __delay_us(1);
        x++;
        if (x > 20) break;
    }
    I2C1CONLbits.RCEN = 0;
    IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2C1STATbits.IWCOL = 0;
    I2C1STATbits.BCL = 0;
    //DelayuSec(10);
    __delay_us(10);
}

//function initiates I2C1 module to baud rate BRG

void i2c_init(int BRG) {
    unsigned int temp;
    // I2CBRG = 194 for 10Mhz OSCI with PPL with 100kHz I2C clock
    //unsigned long I2C_BRG = 0;
    //I2C_BRG = (FCY/2/(BRG*1000))-1;
    I2C1BRG = I2C_BRG;
    I2C1CONLbits.I2CEN = 0; // Disable I2C Mode
    I2C1CONLbits.DISSLW = 1; // Disable slew rate control
    I2C1CONHbits.SDAHT =1; //300ms hold time
    IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2C1CONLbits.I2CEN = 1; // Enable I2C Mode
    temp = I2C1RCV; // read buffer to clear buffer full
    reset_i2c_bus(); // set bus to idle
}

//function iniates a start condition on bus

void i2c_start(void) {
    unsigned int x = 0;
    I2C1CONLbits.ACKDT = 0; //Reset any previous Ack
    //DelayuSec(10);
    __delay_us(10);
    I2C1CONLbits.SEN = 1; //Initiate Start condition
    Nop();
    //the hardware will automatically clear Start Bit
    //wait for automatic clear before proceding
    while (I2C1CONLbits.SEN) {
        //DelayuSec(1);
        __delay_us(1);
        x++;
        if (x > 20)
            break;
    }
    //DelayuSec(2);
    __delay_us(2);
    
}

void i2c_restart(void) {
    unsigned int x = 0;
    I2C1CONLbits.RSEN = 1; //Initiate restart condition
    Nop();
    //the hardware will automatically clear restart bit
    //wait for automatic clear before proceding
    while (I2C1CONLbits.RSEN) {
        //DelayuSec(1);
         __delay_us(1);
        x++;
        if (x > 20) break;
    }
    //DelayuSec(2);
    __delay_us(2);
}

//basic I2C byte send

unsigned char send_i2c_byte(unsigned char data) {
    int i;
    while (I2C1STATbits.TBF) {
    }
    IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2C1TRN = data; // load the outgoing data byte
    // wait for transmission
    for (i = 0; i < 500; i++) {
        if (!I2C1STATbits.TRSTAT) break;
        //DelayuSec(1);
         __delay_us(1);
    }
    if (i == 500) {
        return (1);
    }
    // Check for NO_ACK from slave, abort if not found
    //while (I2C1STATbits.ACKSTAT);

    if (I2C1STATbits.ACKSTAT == 1) {
        reset_i2c_bus();
        return (1);
    }
    //DelayuSec(2);
    __delay_us(2);
    return (0);
}

//function reads data, returns the read data, no ack

unsigned char i2c_read(void) {
    unsigned int i = 0;
    unsigned char data = 0;
    //set I2C module to receive
    I2C1CONLbits.RCEN = 1;
    //if no response, break
    while (!I2C1STATbits.RBF) {
        i++;
        if (i > 2000) break;
    }
    //get data from I2CRCV register
    data = I2C1RCV;
    //return data
    return (data);
}

//function reads data, returns the read data, with ack

unsigned char i2c_read_ack(void) //does not reset bus!!!
{
    int i = 0;
    unsigned char data = 0;
    //set I2C module to receive
    I2C1CONLbits.RCEN = 1;
    //if no response, break
    while (!I2C1STATbits.RBF) {
        i++;
        if (i > 2000) break;
    }
    //get data from I2CRCV register
    data = I2C1RCV;
    //set ACK to high
    I2C1CONLbits.ACKEN = 1;
    //wait before exiting
    //DelayuSec(10);
    __delay_us(10);
   
    return (data);
}

void I2Cwrite(unsigned char addr, unsigned char subaddr, unsigned char value) {
    i2c_start();
    send_i2c_byte(addr);
    send_i2c_byte(subaddr);
    send_i2c_byte(value);
    reset_i2c_bus();
}

unsigned char I2Cread(unsigned char addr, unsigned char subaddr) {
    unsigned char data;
    i2c_start();
    send_i2c_byte(addr);
    send_i2c_byte(subaddr);
    //DelayuSec(10);
     //__delay_us(100);
    i2c_restart();
    send_i2c_byte(addr | 0x01);
    data = i2c_read();
    reset_i2c_bus();
    return (data);
}

unsigned char I2Cread_Ack(unsigned char addr, unsigned char subaddr) {
    unsigned char data;
    i2c_start();
    send_i2c_byte(addr);
    send_i2c_byte(subaddr);
    //DelayuSec(10);
     //__delay_us(10);
    i2c_restart();
    send_i2c_byte(addr | 0x01);
    data = i2c_read_ack();
    reset_i2c_bus();
    return (data);
}


void I2Cwritedouble(unsigned char addr,unsigned char subaddr,unsigned char valuelow, unsigned char valuehigh)
{
i2c_start();
send_i2c_byte(addr);
send_i2c_byte(subaddr);
send_i2c_byte(valuelow);
send_i2c_byte(valuehigh);
reset_i2c_bus();
}

struct doublechar I2Creaddouble(unsigned char addr, unsigned char subaddr)
{
dchar temp;
i2c_start();
send_i2c_byte(addr);
send_i2c_byte(subaddr);
//DelayuSec(10);
__delay_us(10);
i2c_restart();
send_i2c_byte(addr | 0x01);
temp.x = i2c_read_ack();
temp.y = i2c_read();
reset_i2c_bus();
return (temp);
}