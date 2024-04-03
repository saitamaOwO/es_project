#include <LPC17xx.h>

void scan(void);
void lcd_init(void);
void write(int, int);
void delay_lcd(unsigned int);
void lcd_comdata(int, int);
void clear_ports(void);
void lcd_puts(unsigned char *buf1);
void ReadPassword(unsigned char *inputPassword);
unsigned int ComparePasswords(unsigned char *inputPassword, unsigned char *predefinedPassword);

unsigned char Msg1[13] = "Pass: ";
unsigned char row, var, flag, key;
unsigned long int i, var1, temp, temp1, temp2, temp3;
unsigned char SCAN_CODE[16] =
{0x11,0x21,0x41,0x81,
0x12,0x22,0x42,0x82,
0x14,0x24,0x44,0x84,
0x18,0x28,0x48,0x88};
unsigned char ASCII_CODE[16] =
{'0','1','2','3',
'4','5','6','7',
'8','9','A','B',
'C','D','E','F'};

unsigned int PASSWORD_LENGTH = 4; // Define the length of the password
unsigned char inputPassword[4]; // Array to store the input password
unsigned char predefinedPassword[4] = {'1', '2', '3', '4'}; // Define the pre-defined password

int main(void)
{
    LPC_GPIO2->FIODIR |= 0x00003C00; //made output P2.10 to P2.13 (rows)
    LPC_GPIO1->FIODIR &= 0xF87FFFFF; //made input P1.23 to P1.26(cols)
    /*Setting the directions as output */
    LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28;
    clear_ports();
    delay_lcd(3200);
    lcd_init();
    lcd_comdata(0x80, 0); //point to first line of LCD
    delay_lcd(800);
    lcd_puts(Msg1); //display the message
    while (1)
    {
        ReadPassword(inputPassword);
        
        //Compare entered password with predefined password
        unsigned int match = ComparePasswords(inputPassword, predefinedPassword);
        
        // Display the appropriate message based on the comparison result
        lcd_comdata(0x80, 0); // Move cursor to the first line
        delay_lcd(800);
        if (match) {
            // Display "Access Granted"
            lcd_puts("Access Granted");
        } else {
            // Display "Access Denied"
            lcd_puts("Access Denied");
        }
        
        // Delay before resetting for next input
        for (volatile int i = 0; i < 1000000; i++);
         // Delay
    }
}

void ReadPassword(unsigned char *inputPassword) {
    unsigned int passwordIndex = 0;
    while (passwordIndex < PASSWORD_LENGTH)
    {
        while (1)
        {
            for (row = 1; row < 5; row++)
            {
                if (row == 1)
                    var1 = 0x00000400;
                else if (row == 2)
                    var1 = 0x00000800;
                else if (row == 3)
                    var1 = 0x00001000;
                else if (row == 4)
                    var1 = 0x00002000;
                temp = var1;
                LPC_GPIO2->FIOCLR = 0x00003C00; //first clear the
                // port and send appropriate value for
                LPC_GPIO2->FIOSET = var1;        //enabling the row
                flag = 0;
                scan(); //scan if any key pressed in the enabled row
                if (flag == 1)
                    break;
            }
            
            for (i = 0; i < 16; i++)
            {
                if (key == SCAN_CODE[i])
                {
                    key = ASCII_CODE[i];
                    break;
                } //end if(key == SCAN_CODE[i])
            }     //end for(i=0;i<16;i++)

            if (key >= '0' && key <= '9' && passwordIndex < PASSWORD_LENGTH) {
                // Store the entered value into the password array
                inputPassword[passwordIndex++] = key; // Store character directly
                // Display asterisk for each entered digit
                lcd_comdata(0xc0 + passwordIndex - 1, 0); // Move cursor to the next position
                lcd_puts("*");
            } else if (key == 'A') { // Display the password inputted
    unsigned char passwordString[PASSWORD_LENGTH + 1]; // Add 1 for null terminator
    for (int j = 0; j < PASSWORD_LENGTH; j++) {
        passwordString[j] = inputPassword[j] + '0'; // Convert int to char
    }
    passwordString[PASSWORD_LENGTH] = '\0'; // Null terminator
    lcd_puts(passwordString); // Display the password string
} else if (key == 'B') { // Delete the previous value
    if (passwordIndex > 0) {
        passwordIndex--;
        lcd_comdata(0xc0 + passwordIndex, 0); // Move cursor to the previous position
        lcd_puts(" "); // Clear the digit
    }
} else if (key == 'C') { // Clear all input values
                passwordIndex = 0;
                for (int j = 0; j < PASSWORD_LENGTH; j++) {
                    lcd_comdata(0xc0 + j, 0); // Move cursor to each position
                    lcd_puts(" "); // Clear the asterisk
                }
            }
        }
    }
}

unsigned int ComparePasswords(unsigned char *inputPassword, unsigned char *predefinedPassword) {
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (inputPassword[i] != predefinedPassword[i]) {
            return 0; // Return 0 if passwords don't match
        }
    }
    return 1; // Return 1 if passwords match
}

void lcd_init()
{
    /*Ports initialized as GPIO */
    LPC_PINCON->PINSEL1 &= 0xFC003FFF; //P0.23 to P0.28
    /*Setting the directions as output */
    LPC_GPIO0->FIODIR |= 0x0F<<23 | 1<<27 | 1<<28;
    clear_ports();
    delay_lcd(3200);
    lcd_comdata(0x33, 0);
    delay_lcd(30000);
    lcd_comdata(0x32, 0);
    delay_lcd(30000);
    lcd_comdata(0x28, 0); //function set
    delay_lcd(30000);
    lcd_comdata(0x0c, 0);//display on cursor off
    delay_lcd(800);
    lcd_comdata(0x06, 0); //entry mode set increment cursor right
    delay_lcd(800);
    lcd_comdata(0x01, 0); //display clear
    delay_lcd(10000);
}

void scan(void){
    temp3 = LPC_GPIO1->FIOPIN;
    temp3 &= 0x07800000; //check if any key pressed in the enabled row
    if(temp3 != 0x00000000)
    {
        flag = 1;
        temp3 >>= 19; //Shifted to come at HN of byte
        temp >>= 10; //shifted to come at LN of byte
        key = temp3|temp; //get SCAN_CODE
    }//if(temp3 != 0x00000000)
}//end scan

void lcd_comdata(int temp1, int type)
{
    int temp2 = temp1 & 0xf0; //move data (26-8+1) times : 26 - HN place, 4 - Bits
    temp2 = temp2 << 19; //data lines from 23 to 26
    write(temp2, type);
    temp2 = temp1 & 0x0f; //26-4+1
    temp2 = temp2 << 23;
    write(temp2, type);
    delay_lcd(1000);
    return;
}

void write(int temp2, int type) //write to command/data reg
{
    clear_ports();
    LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines
    if(type==0)
        LPC_GPIO0->FIOCLR = 1<<27; // clear bit RS for Command
    else
        LPC_GPIO0->FIOSET = 1<<27; // set bit RS for Data
    LPC_GPIO0->FIOSET = 1<<28; // EN=1
    delay_lcd(25);
    LPC_GPIO0->FIOCLR = 1<<28; // EN =0
    return;
}

void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for(r=0;r<r1;r++);
    return;
}

void clear_ports(void)
{
    /* Clearing the lines at power on */
    LPC_GPIO0->FIOCLR = 0x0F<<23; //Clearing data lines
    LPC_GPIO0->FIOCLR = 1<<27; //Clearing RS line
    LPC_GPIO0->FIOCLR = 1<<28; //Clearing Enable line
    return;
}

void lcd_puts(unsigned char *buf1)
{
    unsigned int i=0;
    unsigned int temp3;
    while(buf1[i]!='\0')
    {
        temp3 = buf1[i];
        lcd_comdata(temp3, 1);
        i++;
        if(i==16)
        {
            lcd_comdata(0xc0, 0);
        }
    }
    return;
}
