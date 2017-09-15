#define CHAR_CONVERT_CONST 48

int X_REGISTER = 0x10;
int Y_REGISTER = 0x11;

unsigned int t = 0;
unsigned int s = 0;

int i2c_err = 0;


// X-coordinate
int pointer_Buffer[2] =  {0x10, 0x00};
signed short X = 1;
signed short Y = 0;

// LCD module connections
sbit LCD_RS at GPIOB_ODR.B9;
sbit LCD_EN at GPIOB_ODR.B8;
sbit LCD_D4 at GPIOE_ODR.B5;
sbit LCD_D5 at GPIOB_ODR.B0;
sbit LCD_D6 at GPIOA_ODR.B5;
sbit LCD_D7 at GPIOA_ODR.B6;

// LCD output
char text[16]    = {'*','*','*','*','W','E','L','C', 'O','M','E','*','*','*','*', '*'};
char output_text[16]    = {'X',':',' ','*','*','*',' ','Y', ':',' ','*','*','*',' ',' ', ' '};
char error_text[16]    = {'*','*','*','*','E','R','R','O', 'R','*','*','*','*','*','*', '*'};

unsigned char uart_text[16] = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'};
unsigned char ok[2] = {0x13, 0x10};
unsigned char _at[2] = {'A', 'T'};
unsigned mask = 4096;
// User interface

void welcomeMessage()
{
  Lcd_Out(1,1,text);
  Lcd_Out(2,1,text);
}

void upaliDiodu() {
GPIOE_ODR |= mask;
}
void ugasiDiodu() {
GPIOE_ODR &= ~mask;
}

void error(){
     Lcd_Cmd(_LCD_CLEAR);
     Lcd_Out(1,1,error_text);
     Delay_ms(100);
     Lcd_Cmd(_LCD_CLEAR);
}
void LCD_setup()
{
  GPIO_Digital_Output(&GPIOB_BASE, _GPIO_PINMASK_8 | _GPIO_PINMASK_9);  // RS & EN
  GPIO_Digital_Output(&GPIOE_BASE, _GPIO_PINMASK_5 | _GPIO_PINMASK_12);                    // D4
  GPIO_Digital_Output(&GPIOB_BASE, _GPIO_PINMASK_0);                    // D5
  GPIO_Digital_Output(&GPIOA_BASE, _GPIO_PINMASK_5 | _GPIO_PINMASK_6);  // D6 & D7

  GPIO_Digital_Output(&GPIOE_BASE, _GPIO_PINMASK_8 | _GPIO_PINMASK_12);
  GPIO_Digital_Input(&GPIOE_BASE, _GPIO_PINMASK_7);

  Lcd_Init();
  Delay_ms(100);
  Lcd_Cmd(_LCD_TURN_ON);
  Lcd_Cmd(_LCD_CURSOR_OFF);
  Lcd_Cmd(_LCD_CLEAR);
}

//Timer2 Prescaler :1; Preload = 59999; Actual Interrupt Time = 1 ms

//Place/Copy this part in declaration section
void InitTimer2(){
  RCC_APB1ENR.TIM2EN = 1;
  TIM2_CR1.CEN = 0;
  TIM2_PSC = 1;
  TIM2_ARR = 59999;
  NVIC_IntEnable(IVT_INT_TIM2);
  TIM2_DIER.UIE = 1;
  TIM2_CR1.CEN = 1;
}





void Joystick_read()
{
    // X-coordinate
    pointer_Buffer[0] = 0x10;
    pointer_Buffer[1] = 0x00;
    X = 0;
    Y = 0;


    i2c_err = 0;

    I2C2_Start();
    upaliDiodu();
    i2c_err = I2C2_Write(0x40, &X_REGISTER, 1, END_MODE_RESTART);  // read X_axis
    Delay_ms(10);
    I2C2_Read(0x40, &pointer_Buffer, 1,END_MODE_STOP);  // read X_axis
    ugasiDiodu();
    X = (signed short)(pointer_Buffer[0]);

    if (X < 0) {
    output_text[2] = 0x2D; //minus sign
    X = fabs(X);
    } else {
    output_text[2] = 0x2B; //minus sign
    }
    output_text[3] = CHAR_CONVERT_CONST+((int)(X/10));
    output_text[4] = '.';
    output_text[5] = CHAR_CONVERT_CONST+(X%10);
    // Y-coordinate
    pointer_Buffer[0] = 0x11;
    pointer_Buffer[1] = 0x00;
    X = 0;

    i2c_err = 0;

    I2C2_Start();
    upaliDiodu();
    i2c_err = I2C2_Write(0x40, &Y_REGISTER, 1,END_MODE_RESTART);  // read Y_axis
    Delay_ms(10);
    I2C2_Read(0x40, &pointer_Buffer, 1,END_MODE_STOP);  // read Y_axis
    ugasiDiodu();
    Y = (signed short)(pointer_Buffer[0]);

    if (Y < 0) {
    output_text[9] = 0x2D; //minus sign
    Y = fabs(Y);
    } else {
    output_text[9] = 0x2B; //minus sign
    }
    output_text[10] = CHAR_CONVERT_CONST+((int)(Y/10));
    output_text[11] = '.';
    output_text[12] = CHAR_CONVERT_CONST+(Y%10);

}

void display()
{
    Lcd_Out(1,1,output_text);
}


void setup()
{
  //InitTimer2();
  I2C2_Init();
  LCD_setup();
}


void main()
{
  setup();
  welcomeMessage();
  Delay_ms(100);
  Lcd_Cmd(_LCD_CLEAR);


//  UART2_Init(115200);//, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART2_PD56);
//  Delay_ms(100);
//  UART2_Write_Text(_at);
//  UART2_Write(0x13);
//  UART2_Write(0x10);
//  upaliDiodu();
//  while(!UART2_Data_Ready());
//  ugasiDiodu();
//  UART2_Read_Text(uart_text, ok, 10);
//  Lcd_Out(1,1, uart_text);
//
//  while(1);


  while (1)
  {
      Joystick_read();
      display();
      Delay_ms(100);
      Lcd_Cmd(_LCD_CLEAR);
  }
}

void Timer2_interrupt() iv IVT_INT_TIM2 {
  TIM2_SR.UIF = 0;
  t+=10;

  if (t == 2000 && s==0) {
     t=0;
//     s=1;
     //welcomeMessage();
//     Joystick_read();
//     display();
  }
//  if (t == 1000 && s==1) {
//     t=0;
//     s=0;
//     Lcd_Cmd(_LCD_CLEAR);
//
//  }
  //Enter your code here
}