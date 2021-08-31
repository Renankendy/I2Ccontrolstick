//###########################################################################
//
// FILE:   Example_2806xI2C_eeprom.c
//
// TITLE:  I2C EEPROM Example
//
//!  \addtogroup f2806x_example_list
//!  <h1>I2C EEPROM(i2c_eeprom)</h1>
//!
//!  This program requires an external I2C EEPROM connected to
//!  the I2C bus at address 0x50.
//!  This program will write 1-14 words to EEPROM and read them back.
//!  The data written and the EEPROM address written to are contained
//!  in the message structure, \b I2cMsgOut1. The data read back will be
//!  contained in the message structure \b I2cMsgIn1.
//!  
//!  \note This program will only work on kits that have an on-board I2C EEPROM. 
//!
//!  \b Watch \b Variables \n
//!  - I2cMsgIn1
//!  - I2cMsgOut1
//
//###########################################################################

//
// Included Files
//
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

//
// Note: I2C Macros used in this example can be found in the
// F2806x_I2C_defines.h file
//

//
// Function Prototypes
//
void   I2CA_Init(void);
__interrupt void i2c_int1a_isr(void);



#define High  0x55
#define Low   0xFF

//
// Main
//
void main(void)
{
    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2806x_SysCtrl.c file.
    //
    InitSysCtrl();

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the F2806x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    //InitGpio();
    
    //
    // Setup only the GP I/O only for I2C functionality
    //
    InitI2CGpio();

    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    DINT;

    //
    // Initialize PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2806x_PieCtrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2806x_DefaultIsr.c.
    // This function is found in F2806x_PieVect.c.
    //
    InitPieVectTable();

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.I2CINT1A = &i2c_int1a_isr;
    EDIS;   // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. Initialize all the Device Peripherals:
    // This function is found in F2806x_InitPeripherals.c
    //
    //InitPeripherals(); // Not required for this example
    I2CA_Init();

    //
    // Step 5. User specific code
    //
    

    //
    // Enable interrupts required for this example
    //

    //
    // Enable I2C interrupt 1 in the PIE: Group 8 interrupt 1
    //
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;

    //
    // Enable CPU INT8 which is connected to PIE group 8
    //
    IER |= M_INT8;
    EINT;

    //
    // Application loop
    //

    for(;;)
    {

    }
}

//
// I2CA_Init - 
//
void
I2CA_Init(void)
{
    //
    // Initialize I2C
    //
    ////I2caRegs.I2CSAR = 0x0050;       // Slave address - EEPROM control code

    I2caRegs.I2CPSC.all = 6;        // Prescaler - need 7-12 Mhz on module clk
    I2caRegs.I2CCLKL = 10;          // NOTE: must be non zero
    I2caRegs.I2CCLKH = 5;           // NOTE: must be non zero

    I2caRegs.I2COAR = 0x10;         // Own Address Register

    I2caRegs.I2CIER.bit.AAS = 1;    // Addressed as slave enabled
    I2caRegs.I2CIER.bit.SCD = 1;    // Stop Condition Detected enabled
    I2caRegs.I2CIER.bit.XRDY = 1;   // Transmit Ready enabled
    I2caRegs.I2CIER.bit.RRDY = 1;   // Receive Ready enabled
    I2caRegs.I2CIER.bit.NACK = 1;   // No-Acknowledgment enabled

    I2caRegs.I2CMDR.bit.FREE = 1;

    I2caRegs.I2CMDR.bit.MST = 0;    // Slave mode
    I2caRegs.I2CMDR.bit.TRX = 0;    // Receiver mode
    I2caRegs.I2CMDR.bit.XA = 0;     // 7 bit addressing mode
    I2caRegs.I2CMDR.bit.FDF = 0;    // Free data format disabled
    I2caRegs.I2CMDR.bit.BC = 0;     // 8 bits per data byte
    I2caRegs.I2CMDR.bit.IRS = 1;    // I2C module enabled

    I2caRegs.I2CCNT = 2;            // Transfer and receive 16 bits data



    //I2caRegs.I2CFFTX.all = 0x6000;    // Enable FIFO mode and TXFIFO
    //I2caRegs.I2CFFRX.all = 0x2040;    // Enable RXFIFO, clear RXFFINT,

    return;
}

//
// i2c_int1a_isr - I2C-A
//
__interrupt void
i2c_int1a_isr(void)
{
    Uint16 IntSource;

    //
    // Read interrupt source
    //
    IntSource = I2caRegs.I2CISRC.all;

    // Module is addressed as a slave
    if(IntSource == I2C_AAS_ISRC)
    {
        // Slave receiver
        if(I2caRegs.I2CSTR.bit.SDIR == 0)
        {
            while(I2caRegs.I2CSTR.bit.RRDY == 0)
            {

            }
            if(I2caRegs.I2CCNT == 2)
            {
                I2caRegs.I2CDXR = 1*I2caRegs.I2CDRR;
                I2caRegs.I2CCNT = 1;
            }
            if(I2caRegs.I2CCNT == 1)
            {
                I2caRegs.I2CCNT = 0;
            }
            /*
            while(I2caRegs.I2CSTR.bit.RRDY == 0)
            {

            }
            if(I2caRegs.I2CCNT == 2)
            {
                Low == I2caRegs.I2CDRR;
                I2caRegs.I2CCNT = 1;
            }
            else
            {
                High == I2caRegs.I2CDRR;
                I2caRegs.I2CCNT = 2;
            }
            */
        }

        // Slave transmitter
        else if(I2caRegs.I2CSTR.bit.SDIR == 1)
        {
            while(I2caRegs.I2CSTR.bit.XRDY == 0)
            {

            }
            if(I2caRegs.I2CCNT == 0)
            {
                I2caRegs.I2CCNT = 1;
            }
            else if(I2caRegs.I2CCNT == 1)
            {
                I2caRegs.I2CDXR = 1*I2caRegs.I2CDRR;
                I2caRegs.I2CCNT = 2;
            }
            else if(I2caRegs.I2CCNT == 2)
            {
                I2caRegs.I2CDXR = 0xFF;
                I2caRegs.I2CCNT = 3;
            }
            else
            {
                I2caRegs.I2CDXR = 0x55;
                I2caRegs.I2CCNT = 2;
            }
        }
        else
        {
            __asm("   ESTOP0");
        }
    }


    if(I2caRegs.I2CISRC.bit.INTCODE == 111)
    {
        __asm("   ESTOP0");
    }
    //Stop Condition Detected
    if(I2caRegs.I2CISRC.bit.INTCODE == 110)
    {
        I2CA_Init();
    }

    //Transmit Ready
    if(I2caRegs.I2CISRC.bit.INTCODE == 101)
    {
        __asm("   ESTOP0");
    }

    //Receive Ready
    if(I2caRegs.I2CISRC.bit.INTCODE == 100)
    {
        __asm("   ESTOP0");
    }

    //Nack
    if(I2caRegs.I2CISRC.bit.INTCODE == 010)
    {
        __asm("   ESTOP0");
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;
}

//
// End of File
//

