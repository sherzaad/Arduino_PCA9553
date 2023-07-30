#include <Wire.h>

#define PCA9553_01_I2C_Addr 0x62
#define PCA9553_02_I2C_Addr 0x63

/* ls_val
  00 = output is set LOW (LED on)
  01 = output is set high-impedance (LED off; default)
  10 = output blinks at PWM0 rate
  11 = output blinks at PWM1 rate
*/
char *led_status[4] = {"ON", "OFF(INPUT)", "PWM0", "PWM1"};
enum ls0_mode {ON, OFF_INPUT, PWM0, PWM1};
enum LEDx {LED0, LED1, LED2, LED3};

struct PCA_regs
{
  uint8_t IO;
  uint8_t PSC0;
  uint8_t PWM0;
  uint8_t PSC1;
  uint8_t PWM1;
  uint8_t LS0;
} pca_regs;

uint8_t IO_Read(int Addr, enum LEDx led_num) {
  uint8_t ip;  
  
  Wire.beginTransmission(Addr);
  Wire.write(0x00);        //don't set auto-increment for reading input port
  Wire.endTransmission();    //end transmission

  Wire.requestFrom(Addr, 1); // request 1 byte

  ip = Wire.read(); // receive a byte  

  return ((ip >> ((uint8_t)led_num + 4)) & 0x01);
}

uint8_t Get_LED_Mode(uint8_t ls0, enum LEDx led_num) {
  return ((ls0 >> ((uint8_t)led_num * 2)) & 0x03);
}

void Set_LED_Mode(uint8_t *ls0, enum LEDx led_num, enum ls0_mode m) {

  uint8_t temp = (((uint8_t)m) << ((uint8_t)led_num * 2));
  uint8_t mask = ~(0x03 << ((uint8_t)led_num * 2));

  *ls0 = (*ls0 & mask) | temp;
}

void PCA9553_read_registers(int Addr, struct PCA_regs *regs) {
  uint8_t buf[6];
  Wire.beginTransmission(Addr); // initialise transmit to device
  Wire.write(0x10);        //set auto-increment for reading all registers
  Wire.endTransmission();    //end transmission

  Wire.requestFrom(Addr, 6); // request 6 bytes from device

  for (uint8_t i = 0; i < 6; ++i) {
    buf[i] = Wire.read(); // receive a byte
  }

  memcpy(regs, buf, 6);

  Wire.beginTransmission(Addr);
  Wire.write(0x00);        //don't set auto-increment for reading input port
  Wire.endTransmission();    //end transmission

  Wire.requestFrom(Addr, 1); // request 1 byte

  regs->IO = Wire.read(); // receive a byte
}

void print_regs() {
  Serial.print("INPUT(read-only): ");
  Serial.println(pca_regs.IO, BIN);
  Serial.print("PSC0: ");
  Serial.println(pca_regs.PSC0, BIN);
  Serial.print("PWM0: ");
  Serial.println(pca_regs.PWM0, BIN);
  Serial.print("PSC1: ");
  Serial.println(pca_regs.PSC0, BIN);
  Serial.print("PWM1: ");
  Serial.print("LS0: ");
  Serial.println(pca_regs.LS0, BIN);
  Serial.print("LED0 Mode: ");
  Serial.println(led_status[Get_LED_Mode(pca_regs.LS0, LED0)]);
  Serial.print("LED1 Mode: ");
  Serial.println(led_status[Get_LED_Mode(pca_regs.LS0, LED1)]);
  Serial.print("LED2 Mode: ");
  Serial.println(led_status[Get_LED_Mode(pca_regs.LS0, LED2)]);
  Serial.print("LED3 Mode: ");
  Serial.println(led_status[Get_LED_Mode(pca_regs.LS0, LED3)]);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(); // join i2c bus (address optional for master)

  Serial.println("----PCA9553 Example Code----");
  Serial.println("Current Register Status:");
  PCA9553_read_registers(PCA9553_01_I2C_Addr, &pca_regs); //assuming chip part number is PCA9553/01
  print_regs();

  Wire.beginTransmission(PCA9553_01_I2C_Addr); //assuming chip part number is PCA9553/01
  Wire.write(0x11); //PSC0 subaddress + Auto-Increment
  Wire.write(0x2B); //Set prescaler PSC0 to achieve a period of 1 second:
  Wire.write(0x80); //Set PWM0 duty cycle to 50 %
  Wire.write(0x0A); //Set prescaler PSC1 to achieve a period of 0.25 seconds:
  Wire.write(0xC0); //Set PWM1 output duty cycle to 25 %:

  //Set LED0 on, LED1 off, LED2 set to blink at PSC0, PWM0, LED3 set to blink at PSC1, PWM1
  uint8_t ls0;
  Set_LED_Mode(&ls0, LED0, ON);
  Set_LED_Mode(&ls0, LED1, OFF_INPUT);
  Set_LED_Mode(&ls0, LED2, PWM0);
  Set_LED_Mode(&ls0, LED3, PWM1);
  Wire.write(ls0);
  //

  Wire.endTransmission();    //end transmission

  Serial.println("Register Status After Update:");
  PCA9553_read_registers(PCA9553_01_I2C_Addr, &pca_regs); //assuming chip part number is PCA9553/01
  print_regs();
}

void loop() {
//nothing here
}
