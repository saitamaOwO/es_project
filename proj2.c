#include <LPC17xx.h>
#include <string.h>

void scan(void);
void lcd_init(void);
void write(unsigned int, int);
void delay_lcd(unsigned int);
void lcd_comdata(unsigned int, int);
void clear_ports(void);
void lcd_puts(unsigned char *buf1);
unsigned char Msg1[13] = "Pass: ";
unsigned char row, var, flag, key;
unsigned long int var1, temp, temp3;
unsigned char SCAN_CODE[16] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'};
unsigned char inputPassword[5] = {0}; // Array to store the input password
unsigned char predefinedPassword[5] = {'1', '2', '3', '4', '\0'}; // Define the pre-defined password

int main(void) {
    LPC_GPIO2->FIODIR |= 0x00003C00; // made output P2.10 to P2.13 (rows)
    LPC_GPIO1->FIODIR &= 0xF87FFFFF; // made input P1.23 to P1.26 (cols)
    LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28;
    clear_ports();
    delay_lcd(3200);
    lcd_init();
    lcd_comdata(0x80, 0); // point to first line of LCD
    delay_lcd(800);
    lcd_puts(&Msg1[0]); // display the message
    while (1) {
        while (1) {
            for (row = 1; row < 5; row++) {
                if (row == 1)
                    var1 = 0x00000400;
                else if (row == 2)
                    var1 = 0x00000800;
                else if (row == 3)
                    var1 = 0x00001000;
                else if (row == 4)
                    var1 = 0x00002000;
                temp = var1;
                LPC_GPIO2->FIOCLR = 0x00003C00; // first clear the port and send appropriate value for
                LPC_GPIO2->FIOSET = var1;        // enabling the row
                flag = 0;
                scan(); // scan if any key pressed in the enabled row
                if (flag == 1)
                    break;
            }
            if (flag == 1)
                break;
        }
        unsigned int passwordIndex = 0;
        while (1) {
            temp3 = LPC_GPIO1->FIOPIN;
            temp3 &= 0x07800000; // check if any key pressed in the enabled row
            if (temp3 != 0x00000000) {
                flag = 1;
                temp3 >>= 19; // Shifted to come at HN of byte
                temp >>= 10;  // shifted to come at LN of byte
                key = temp3 | temp; // get SCAN_CODE
            }
            for (unsigned int i = 0; i < 16; i++) {
                if (key == SCAN_CODE[i]) {
                    key = i + '0';
                    break;
                }
            }
            if (key != '#') {
                if (key != 'D') {
                    if (key == '*') {
                        passwordIndex = 0;
                        clear_ports();
                        lcd_comdata(0x80, 0); // point to first line of LCD
                        delay_lcd(800);
                        lcd_puts(&Msg1[0]); // display the message
                        break;
                    } else if (passwordIndex < 4) {
                        inputPassword[passwordIndex++] = key;
                        clear_ports();
                        lcd_comdata(0xc0 + passwordIndex - 1, 0); // Move cursor to the next position
                        lcd_puts("*");
                    }
                }
            } else {
                break;
            }
        }
        inputPassword[passwordIndex] = '\0';
        if (strcmp((char *)inputPassword, (char *)predefinedPassword) == 0) {
            lcd_comdata(0x80, 0); // point to first line of LCD
            delay_lcd(800);
            lcd_puts("Access Granted");
        } else {
            lcd_comdata(0x80, 0); // point to first line of LCD
            delay_lcd(800);
            lcd_puts("Access Denied ");
        }
        delay_lcd(1000000);
    }
}

void scan(void) {
    temp = LPC_GPIO1->FIOPIN & 0x07800000; // check if any key pressed in the enabled row
    if (temp != 0x00000000) {
        flag = 1;
        temp3 = temp >> 19; // Shifted to come at HN of byte
        temp >>= 10;        // shifted to come at LN of byte
        key = temp3 | temp; // get SCAN_CODE
    }
}

void lcd_init() {
    LPC_PINCON->PINSEL1 &= 0xFC003FFF; // P0.23 to P0.28
    LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28; // Setting the directions as output
    clear_ports();
    delay_lcd(3200);
    lcd_comdata(0x33, 0);
    delay_lcd(30000);
    lcd_comdata(0x32, 0);
    delay_lcd(30000);
    lcd_comdata(0x28, 0); // function set
    delay_lcd(30000);
    lcd_comdata(0x0c, 0); // display on cursor off
    delay_lcd(800);
    lcd_comdata(0x06, 0); // entry mode set increment cursor right
    delay_lcd(800);
    lcd_comdata(0x01, 0); // display clear
    delay_lcd(10000);
}

void lcd_comdata(unsigned int temp1, int type) {
    unsigned int temp2 = temp1 & 0xf0;     // move data (26-8+1) times : 26 - HN place, 4 - Bits
    temp2 = temp2 << 19;                    // data lines from 23 to 26
    write(temp2, type);
    temp2 = temp1 & 0x0f;                    // 26-4+1
    temp2 = temp2 << 23;
    write(temp2, type);
    delay_lcd(1000);
}

void write(unsigned int temp2, int type) // write to command/data reg
{
    clear_ports();
    LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines
    if (type == 0)
        LPC_GPIO0->FIOCLR = 1 << 27; // clear bit RS for Command
    else
        LPC_GPIO0->FIOSET = 1 << 27; // set bit RS for Data
    LPC_GPIO0->FIOSET = 1 << 28;     // EN=1
    delay_lcd(25);
    LPC_GPIO0->FIOCLR = 1 << 28;     // EN =0
}

void delay_lcd(unsigned int r1) {
    for (unsigned int r = 0; r < r1; r++);
}

void clear_ports(void) {
    LPC_GPIO0->FIOCLR = 0x0F << 23; // Clearing data lines
    LPC_GPIO0->FIOCLR = 1 << 27;    // Clearing RS line
    LPC_GPIO0->FIOCLR = 1 << 28;    // Clearing Enable line
}

void lcd_puts(unsigned char *buf1) {
    unsigned int i = 0;
    unsigned int temp3;
    while (buf1[i] != '\0') {
        temp3 = buf1[i];
        lcd_comdata(temp3, 1);
        i++;
        if (i == 16) {
            lcd_comdata(0xc0, 0);
        }
    }
}
