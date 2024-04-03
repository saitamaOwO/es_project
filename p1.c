#include <LPC17xx.h>

unsigned char row, var, flag, key, disp[9]; // Increase disp size to display longer messages
int operation = -1, equalsTyped = 0, result = 0;
unsigned int password = 1234; // Define a 4-digit password
unsigned int enteredPassword = 0;
unsigned long int i, temp, temp1, temp2, temp3;
unsigned char SCAN_CODE[16] = {0x11, 0x21, 0x41, 0x81,
                                0x12, 0x22, 0x42, 0x82,
                                0x14, 0x24, 0x44, 0x84,
                                0x18, 0x28, 0x48, 0x88};
unsigned char ASCII_CODE[16] = {'0', '1', '2', '3',
                                '4', '5', '6', '7',
                                '8', '9', '+', '-',
                                '=', 'D', 'E', 'F'};

void delay_lcd(unsigned int r1) {
    unsigned int r;
    for(r = 0; r < r1; r++);
    return;
}

void clear_ports(void) {
    /* Clearing the lines at power on */
    LPC_GPIO0->FIOCLR = 0x0F<<23; //Clearing data lines
    LPC_GPIO0->FIOCLR = 1<<27; //Clearing RS line
    LPC_GPIO0->FIOCLR = 1<<28; //Clearing Enable line
    
    return;
}

void write(int temp2, int type) { 
    clear_ports();
    LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines 
    if(type == 0)
        LPC_GPIO0->FIOCLR = 1<<27; // clear bit RS for Command
    else
        LPC_GPIO0->FIOSET = 1<<27; // set bit RS for Data
    LPC_GPIO0->FIOSET = 1<<28; // EN=1
    delay_lcd(25);
    LPC_GPIO0->FIOCLR = 1<<28; // EN =0
    return;
}

void lcd_comdata(int temp1, int type) {
    int temp2 = temp1 & 0xf0; //move data (26-8+1) times : 26 - HN place, 4 - Bits
    temp2 = temp2 << 19; //data lines from 23 to 26
    write(temp2, type);
    temp2 = temp1 & 0x0f; //26-4+1
    temp2 = temp2 << 23; 
    write(temp2, type);
    delay_lcd(1000);
    return;
}

void lcd_puts(unsigned char *buf1) {
    unsigned int i=0;
    unsigned int temp3;
    while(buf1[i]!='\0') {
        temp3 = buf1[i];
        lcd_comdata(temp3, 1);
        i++;
        if(i == 16) {
            lcd_comdata(0xc0, 0);
        }
    }
    return;
}

void lcd_init() {
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
    return;
}

void scan(void) {
    temp3 = LPC_GPIO1->FIOPIN;
    temp3 &= 0x07800000; //check if any key pressed in the enabled row .. checks bits 26 to 23
    if(temp3 != 0) {
        flag = 1;
        temp3 >>= 19;//Shifted to come at HN of byte (4 to 7 bits)
        temp >>= 10; //shifted to come at LN of byte (indicates row 0 to 3 bits)
        key = temp3 | temp; //get SCAN_CODE (0 to 7 bits)
    }
}//end scan

void checkPassword() {
    if (enteredPassword == password) {
        lcd_comdata(0x01, 0); // Clear display
        delay_lcd(800);
        lcd_puts("Correct Password");
    } else {
        lcd_comdata(0x01, 0); // Clear display
        delay_lcd(800);
        lcd_puts("Wrong Password");
    }
}

int main(void) {
    unsigned int var1; // Define var1 here
		int digit;
    LPC_GPIO2->FIODIR |= 0x3C00; // made output P2.10 to P2.13 (rows)
    LPC_GPIO1->FIODIR &= 0xF87FFFFF; // made input P1.23 to P1.26(cols)
    LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28;
    clear_ports();
    delay_lcd(3200);
    lcd_init();
    lcd_comdata(0x80, 0); // point to first line of LCD
    delay_lcd(800);

    while (1) {
        // Take input for password
        lcd_comdata(0x01, 0); // Clear display
        delay_lcd(800);
        lcd_puts("Enter Password:");
        enteredPassword = 0;

        for (digit = 0; digit < 4; digit++) {
            flag = 0;
            while (!flag) {
                for (row = 1; row < 5; row++) {
                    if (row == 1)
                        var1 = 1 << 10;
                    else if (row == 2)
                        var1 = 1 << 11;
                    else if (row == 3)
                        var1 = 1 << 12;
                    else if (row == 4)
                        var1 = 1 << 13;
                    temp = var1;
                    LPC_GPIO2->FIOCLR = 0x3C00; // first clear the port and send appropriate value for
                    LPC_GPIO2->FIOSET = var1; // enabling the row
                    flag = 0;
                    scan(); // scan if any key pressed in the enabled row
                    if (flag == 1) {
                        break;
                    }
                } // end for

                if (flag == 1) {
                    for (i = 0; i < 16; i++) {
                        if (key == SCAN_CODE[i]) {
                            key = ASCII_CODE[i];
                            if (key >= '0' && key <= '9') {
                                enteredPassword = enteredPassword * 10 + (key - '0');
                                break;
                            }
                        }
                    }
                }
            } // end while (!flag)
        } // end for (int digit = 0; digit < 4; digit++)

        // Check if the entered password matches the predefined password
        checkPassword();
    } // end while(1)
} // end main
