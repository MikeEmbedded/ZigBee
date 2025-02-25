/**************************************************************************************************
  Filename:       hal_lcd.c
  Revised:        $Date: 2012-09-30 16:36:36 -0700 (Sun, 30 Sep 2012) $
  Revision:       $Revision: 31658 $

  Description:    This file contains the interface to the HAL LCD Service.


  Copyright 2007-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/**************************************************************************************************
 *                                           INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "hal_lcd.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "hal_assert.h"

#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
  #include "DebugTrace.h"
#endif

u8g2_t u8g2;

/**************************************************************************************************
 *                                          CONSTANTS
 **************************************************************************************************/
/*
  LCD pins

  //control
  P0.0 - LCD_MODE
  P1.1 - LCD_FLASH_RESET
  P1.2 - LCD_CS

  //spi
  P1.5 - CLK
  P1.6 - MOSI
  P1.7 - MISO
*/

/* SPI settings */
#define HAL_SPI_CLOCK_POL_LO       0x00
#define HAL_SPI_CLOCK_PHA_0        0x00
#define HAL_SPI_CLOCK_POL_HIGH     0x80
#define HAL_SPI_CLOCK_PHA_1        0x40
#define HAL_SPI_TRANSFER_MSB_FIRST 0x20

///* LCD lines */
//#define LCD_MAX_LINE_COUNT              3
//
///* Defines for HW LCD */
//
///* Set power save mode */
//#define OSC_OFF                         0x00
//#define OSC_ON                          0x01
//#define POWER_SAVE_OFF                  0x00
//#define POWER_SAVE_ON                   0x02
//#define SET_POWER_SAVE_MODE(options)    HalLcd_HW_Control(0x0C | (options))
//
///* Function Set */
//#define CGROM                           0x00
//#define CGRAM                           0x01
//#define COM_FORWARD                     0x00
//#define COM_BACKWARD                    0x02
//#define TWO_LINE                        0x00
//#define THREE_LINE                      0x04
//#define FUNCTION_SET(options)           HalLcd_HW_Control(0x10 | (options))
//
///* Set Display Start Line */
//#define LINE1                           0x00
//#define LINE2                           0x01
//#define LINE3                           0x02
//#define LINE4                           0x03
//#define SET_DISPLAY_START_LINE(line)    HalLcd_HW_Control(0x18 | (line))
//
///* Bias control */
//#define BIAS_1_5                        0x00
//#define BIAS_1_4                        0x01
//#define SET_BIAS_CTRL(bias)             HalLcd_HW_Control(0x1C | (bias))
//
///* Power control */
//#define VOLTAGE_DIVIDER_OFF             0x00
//#define VOLTAGE_DIVIDER_ON              0x01
//#define CONVERTER_AND_REG_OFF           0x00
//#define CONVERTER_AND_REG_ON            0x04
//#define SET_POWER_CTRL(options)         HalLcd_HW_Control(0x20 | (options))
//
//// Set display control
//#define DISPLAY_CTRL_ON                 0x01
//#define DISPLAY_CTRL_OFF                0x00
//#define DISPLAY_CTRL_BLINK_ON           0x02
//#define DISPLAY_CTRL_BLINK_OFF          0x00
//#define DISPLAY_CTRL_CURSOR_ON          0x04
//#define DISPLAY_CTRL_CURSOR_OFF         0x00
//#define SET_DISPLAY_CTRL(options)       HalLcd_HW_Control(0x28 | (options))
//
///* Set DD/ CGRAM address */
//#define SET_DDRAM_ADDR(charIndex)       HalLcd_HW_Control(0x80 | (charIndex))
//#define SET_GCRAM_CHAR(specIndex)       HalLcd_HW_Control(0xC0 | (specIndex))
//
///* Set ICONRAM address */
//#define CONTRAST_CTRL_REGISTER          0x10
//#define SET_ICONRAM_ADDR(addr)          HalLcd_HW_Control(0x40 | (addr))
//
///* Set double height */
//#define LINE_1_AND_2                    0x01
//#define LINE_2_AND_3                    0x02
//#define NORMAL_DISPLAY                  0x00
//#define SET_DOUBLE_HEIGHT(options)      HalLcd_HW_Control(0x08 | (options))
//
///**************************************************************************************************
// *                                           MACROS
// **************************************************************************************************/
//
///* SPI interface control */
//#define LCD_SPI_BEGIN()     HAL_IO_SET(HAL_LCD_CS_PORT,  HAL_LCD_CS_PIN,  0); /* chip select */
//#define LCD_SPI_END()                                                         \
//{                                                                             \
//  asm("NOP");                                                                 \
//  asm("NOP");                                                                 \
//  asm("NOP");                                                                 \
//  asm("NOP");                                                                 \
//  HAL_IO_SET(HAL_LCD_CS_PORT,  HAL_LCD_CS_PIN,  1); /* chip select */         \
//}
///* clear the received and transmit byte status, write tx data to buffer, wait till transmit done */
//#define LCD_SPI_TX(x)                   { U1CSR &= ~(BV(2) | BV(1)); U1DBUF = x; while( !(U1CSR & BV(1)) ); }
//#define LCD_SPI_WAIT_RXRDY()            { while(!(U1CSR & BV(1))); }


#if (HAL_LCD == TRUE)
/**************************************************************************************************
 *                                       LOCAL VARIABLES
 **************************************************************************************************/

//static uint8 *Lcd_Line1;

/**************************************************************************************************
 *                                       FUNCTIONS - API
 **************************************************************************************************/

void HalLcd_HW_Init(void);
void HalLcd_HW_WaitUs(uint32 i);
//void HalLcd_HW_ClearAllSpecChars(void);
//void HalLcd_HW_Control(uint8 cmd);
//void HalLcd_HW_Write(uint8 data);
//void HalLcd_HW_SetContrast(uint8 value);
//void HalLcd_HW_WriteChar(uint8 line, uint8 col, char text);
//void HalLcd_HW_WriteLine(uint8 line, const char *pText);
#endif //LCD

/**************************************************************************************************
 * @fn      HalLcdInit
 *
 * @brief   Initilize LCD Service
 *
 * @param   init - pointer to void that contains the initialized value
 *
 * @return  None
 **************************************************************************************************/
void HalLcdInit(void)
{
#if (HAL_LCD == TRUE)
  //Lcd_Line1 = NULL;
  HalLcd_HW_Init();
#endif
}

/*************************************************************************************************
 *                    LCD EMULATION FUNCTIONS
 *
 * Some evaluation boards are equipped with Liquid Crystal Displays
 * (LCD) which may be used to display diagnostic information. These
 * functions provide LCD emulation, sending the diagnostic strings
 * to Z-Tool via the RS232 serial port. These functions are enabled
 * when the "LCD_SUPPORTED" compiler flag is placed in the makefile.
 *
 * Most applications update both lines (1 and 2) of the LCD whenever
 * text is posted to the device. This emulator assumes that line 1 is
 * updated first (saved locally) and the formatting and send operation
 * is triggered by receipt of line 2. Nothing will be transmitted if
 * only line 1 is updated.
 *
 *************************************************************************************************/


/**************************************************************************************************
 * @fn      HalLcdWriteString
 *
 * @brief   Write a string to the LCD
 *
 * @param   str    - pointer to the string that will be displayed
 *          option - display options
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdWriteString ( char *str, uint8 option)
//{
//#if (HAL_LCD == TRUE)
//
//  uint8 strLen = 0;
//  uint8 totalLen = 0;
//  uint8 *buf;
//  uint8 tmpLen;
//
//  if ( Lcd_Line1 == NULL )
//  {
//    Lcd_Line1 = osal_mem_alloc( HAL_LCD_MAX_CHARS+1 );
//    HalLcdWriteString( "TexasInstruments", 1 );
//  }
//
//  strLen = (uint8)osal_strlen( (char*)str );
//
//  /* Check boundries */
//  if ( strLen > HAL_LCD_MAX_CHARS )
//    strLen = HAL_LCD_MAX_CHARS;
//
//  if ( option == HAL_LCD_LINE_1 )
//  {
//    /* Line 1 gets saved for later */
//    osal_memcpy( Lcd_Line1, str, strLen );
//    Lcd_Line1[strLen] = '\0';
//  }
//  else
//  {
//    /* Line 2 triggers action */
//    tmpLen = (uint8)osal_strlen( (char*)Lcd_Line1 );
//    totalLen =  tmpLen + 1 + strLen + 1;
//    buf = osal_mem_alloc( totalLen );
//    if ( buf != NULL )
//    {
//      /* Concatenate strings */
//      osal_memcpy( buf, Lcd_Line1, tmpLen );
//      buf[tmpLen++] = ' ';
//      osal_memcpy( &buf[tmpLen], str, strLen );
//      buf[tmpLen+strLen] = '\0';
//
//      /* Send it out */
//#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
//
//#if defined(SERIAL_DEBUG_SUPPORTED)
//      debug_str( (uint8*)buf );
//#endif //LCD_SUPPORTED
//
//#endif //ZTOOL_P1
//
//      /* Free mem */
//      osal_mem_free( buf );
//    }
//  }
//
//  if (option != HAL_LCD_SERIAL_DEBUG_ONLY)
//  {
//    /* Display the string */
//    HalLcd_HW_WriteLine (option, str);
//  }
//
//#endif //HAL_LCD
//
//}

/**************************************************************************************************
 * @fn      HalLcdWriteValue
 *
 * @brief   Write a value to the LCD
 *
 * @param   value  - value that will be displayed
 *          radix  - 8, 10, 16
 *          option - display options
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option)
//{
//#if (HAL_LCD == TRUE)
//  uint8 buf[HAL_LCD_MAX_BUFF];
//
//  _ltoa( value, &buf[0], radix );
//  HalLcdWriteString( (char*)buf, option );
//#endif
//}

/**************************************************************************************************
 * @fn      HalLcdWriteScreen
 *
 * @brief   Write a value to the LCD
 *
 * @param   line1  - string that will be displayed on line 1
 *          line2  - string that will be displayed on line 2
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdWriteScreen( char *line1, char *line2 )
//{
//#if (HAL_LCD == TRUE)
//  HalLcdWriteString( line1, HAL_LCD_DEBUG_LINE_1 );
//  HalLcdWriteString( line2, HAL_LCD_DEBUG_LINE_2 );
//#endif
//}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title  - Title that will be displayed before the value
 *          value  - value
 *          format - redix
 *          line   - line number
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line )
//{
//#if (HAL_LCD == TRUE)
//  uint8 tmpLen;
//  uint8 buf[HAL_LCD_MAX_BUFF];
//  uint32 err;
//
//  tmpLen = (uint8)osal_strlen( (char*)title );
//  osal_memcpy( buf, title, tmpLen );
//  buf[tmpLen] = ' ';
//  err = (uint32)(value);
//  _ltoa( err, &buf[tmpLen+1], format );
//  HalLcdWriteString( (char*)buf, line );		
//#endif
//}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title   - Title that will be displayed before the value
 *          value1  - value #1
 *          format1 - redix of value #1
 *          value2  - value #2
 *          format2 - redix of value #2
 *          line    - line number
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1,
//                                  uint16 value2, uint8 format2, uint8 line )
//{
//
//#if (HAL_LCD == TRUE)
//
//  uint8 tmpLen;
//  uint8 buf[HAL_LCD_MAX_BUFF];
//  uint32 err;
//
//  tmpLen = (uint8)osal_strlen( (char*)title );
//  if ( tmpLen )
//  {
//    osal_memcpy( buf, title, tmpLen );
//    buf[tmpLen++] = ' ';
//  }
//
//  err = (uint32)(value1);
//  _ltoa( err, &buf[tmpLen], format1 );
//  tmpLen = (uint8)osal_strlen( (char*)buf );
//
//  buf[tmpLen++] = ',';
//  buf[tmpLen++] = ' ';
//  err = (uint32)(value2);
//  _ltoa( err, &buf[tmpLen], format2 );
//
//  HalLcdWriteString( (char *)buf, line );		
//
//#endif
//}

/**************************************************************************************************
 * @fn      HalLcdDisplayPercentBar
 *
 * @brief   Display percentage bar on the LCD
 *
 * @param   title   -
 *          value   -
 *
 * @return  None
 **************************************************************************************************/
//void HalLcdDisplayPercentBar( char *title, uint8 value )
//{
//#if (HAL_LCD == TRUE)
//
//  uint8 percent;
//  uint8 leftOver;
//  uint8 buf[17];
//  uint32 err;
//  uint8 x;
//
//  /* Write the title: */
//  HalLcdWriteString( title, HAL_LCD_LINE_1 );
//
//  if ( value > 100 )
//    value = 100;
//
//  /* convert to blocks */
//  percent = (uint8)(value / 10);
//  leftOver = (uint8)(value % 10);
//
//  /* Make window */
//  osal_memcpy( buf, "[          ]  ", 15 );
//
//  for ( x = 0; x < percent; x ++ )
//  {
//    buf[1+x] = '>';
//  }
//
//  if ( leftOver >= 5 )
//    buf[1+x] = '+';
//
//  err = (uint32)value;
//  _ltoa( err, (uint8*)&buf[13], 10 );
//
//  HalLcdWriteString( (char*)buf, HAL_LCD_LINE_2 );
//
//#endif
//
//}

#if (HAL_LCD == TRUE)
/**************************************************************************************************
 *                                    HARDWARE LCD
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      halLcd_ConfigIO
 *
 * @brief   Configure IO lines needed for LCD control.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
//static void halLcd_ConfigIO(void)
//{
//  /* GPIO configuration */
//  HAL_CONFIG_IO_OUTPUT(HAL_LCD_MODE_PORT,  HAL_LCD_MODE_PIN,  1);
//  HAL_CONFIG_IO_OUTPUT(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 1);
//  HAL_CONFIG_IO_OUTPUT(HAL_LCD_CS_PORT,    HAL_LCD_CS_PIN,    1);
//  HAL_CONFIG_IO_INPUT(HAL_LCD_BUSY_PORT,   HAL_LCD_BUSY_PIN);
//}

/**************************************************************************************************
 * @fn      halLcd_ConfigSPI
 *
 * @brief   Configure SPI lines needed for talking to LCD.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
//static void halLcd_ConfigSPI(void)
//{
//  /* UART/SPI Peripheral configuration */
//
//   uint8 baud_exponent;
//   uint8 baud_mantissa;
//
//  /* Set SPI on UART 1 alternative 2 */
//  PERCFG |= 0x02;
//
//  /* Configure clk, master out and master in lines */
//  HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_CLK_PORT,  HAL_LCD_CLK_PIN);
//  HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_MOSI_PORT, HAL_LCD_MOSI_PIN);
//  //HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_MISO_PORT, HAL_LCD_MISO_PIN);
//
//
//  /* Set SPI speed to 1 MHz (the values assume system clk of 32MHz)
//   * Confirm on board that this results in 1MHz spi clk.
//   */
//  baud_exponent = 15;
//  baud_mantissa =  0;
//
//  /* Configure SPI */
//  U1UCR  = 0x80;      /* Flush and goto IDLE state. 8-N-1. */
//  U1CSR  = 0x00;      /* SPI mode, master. */
//  U1GCR  = HAL_SPI_TRANSFER_MSB_FIRST | HAL_SPI_CLOCK_PHA_0 | HAL_SPI_CLOCK_POL_LO | baud_exponent;
//  //U1GCR  = HAL_SPI_TRANSFER_MSB_FIRST | HAL_SPI_CLOCK_PHA_1 | HAL_SPI_CLOCK_POL_HIGH | baud_exponent;
//  U1BAUD = baud_mantissa;
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_Init
 *
 * @brief   Initilize HW LCD Driver.
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
uint8_t u8x8_gpio_and_delay (u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch (msg)
  {
    // called once during init phase of u8g2/u8x8
    // can be used to setup pins
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      HAL_CONFIG_IO_OUTPUT(HAL_LCD_MODE_PORT,  HAL_LCD_MODE_PIN,  1);
      HAL_CONFIG_IO_OUTPUT(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 1);
      HAL_CONFIG_IO_OUTPUT(HAL_LCD_CS_PORT,    HAL_LCD_CS_PIN,    1);
      HAL_CONFIG_IO_OUTPUT(HAL_LCD_CLK_PORT,   HAL_LCD_CLK_PIN,   1);
      HAL_CONFIG_IO_OUTPUT(HAL_LCD_MOSI_PORT,  HAL_LCD_MOSI_PIN,  1);
      HAL_CONFIG_IO_INPUT(HAL_LCD_BUSY_PORT,   HAL_LCD_BUSY_PIN);
      break;
    // CS (chip select) pin: Output level in arg_int
    case U8X8_MSG_GPIO_CS:
//      if (arg_int)
//        HAL_IO_SET(HAL_LCD_CS_PORT, HAL_LCD_CS_PIN, 1);
//      else
//        HAL_IO_SET(HAL_LCD_CS_PORT, HAL_LCD_CS_PIN, 0);
      break;
    // DC (data/cmd, A0, register select) pin: Output level in arg_int
    case U8X8_MSG_GPIO_DC:
      if (arg_int)
        HAL_IO_SET(HAL_LCD_DC_PORT, HAL_LCD_DC_PIN, 1);
      else
        HAL_IO_SET(HAL_LCD_DC_PORT, HAL_LCD_DC_PIN, 0);
      break;
    // Reset pin: Output level in arg_int
    case U8X8_MSG_GPIO_RESET:
      if (arg_int)
        HAL_IO_SET(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 1);
      else
        HAL_IO_SET(HAL_LCD_RESET_PORT, HAL_LCD_RESET_PIN, 0);
      break;
    case U8X8_MSG_DELAY_NANO:  // delay arg_int * 1 nano second
      break;
    case U8X8_MSG_DELAY_100NANO:       // delay arg_int * 100 nano seconds
      break;
    case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
      break;
    case U8X8_MSG_DELAY_MILLI:  // delay arg_int * 1 milli second
      break;
    case U8X8_MSG_WAIT_FOR_PIN:
      HalLcd_HW_WaitUs(1);
      while(P1_2 == 1)      //LOW: idle, HIGH: busy
          HalLcd_HW_WaitUs(1);
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);
      break;
  }
  return 1;
}

uint8_t u8x8_byte_hw_spi (u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t *data;
  
  switch (msg)
  {
    case U8X8_MSG_BYTE_INIT:
      {
        /* UART/SPI Peripheral configuration */
         uint8 baud_exponent;
         uint8 baud_mantissa;

        /* Set SPI on UART 1 alternative 2 */
        PERCFG |= 0x02;

        /* Configure clk, master out and master in lines */
        HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_CLK_PORT,  HAL_LCD_CLK_PIN);
        HAL_CONFIG_IO_PERIPHERAL(HAL_LCD_MOSI_PORT, HAL_LCD_MOSI_PIN);

        /* Set SPI speed to 20 MHz (the values assume system clk of 32MHz)*/
        baud_exponent = 19;
        baud_mantissa = 64;

        /* Configure SPI */
        U1UCR  = 0x80;      /* Flush and goto IDLE state. 8-N-1. */
        U1CSR  = 0x00;      /* SPI mode, master. */
        U1GCR  = HAL_SPI_TRANSFER_MSB_FIRST | HAL_SPI_CLOCK_PHA_0 | HAL_SPI_CLOCK_POL_LO | baud_exponent;
        U1BAUD = baud_mantissa;

        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
      }
      break;
    case U8X8_MSG_BYTE_SET_DC:
      u8x8_gpio_SetDC(u8x8, arg_int);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      break;
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *) arg_ptr;

      while (arg_int > 0)
      {
        HAL_IO_SET(HAL_LCD_CS_PORT, HAL_LCD_CS_PIN, 0);
        U1CSR &= ~(BV(2) | BV(1));
        U1DBUF = (unsigned char) *data;
//        while( !(U1CSR & BV(1)) );
        while( (U1CSR & BV(0)) );
        HAL_IO_SET(HAL_LCD_CS_PORT, HAL_LCD_CS_PIN, 1);

        data++;
        arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      break;
    default:
      return 0;
  }
  
  return 1;
}

void HalLcd_HW_Init(void)
{
  u8g2_Setup_ssd1607_200x200_5(&u8g2, U8G2_R0, u8x8_byte_hw_spi, u8x8_gpio_and_delay);
  
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_ClearBuffer(&u8g2);
}

/**************************************************************************************************
 * @fn      HalLcd_HW_Control
 *
 * @brief   Write 1 command to the LCD
 *
 * @param   uint8 cmd - command to be written to the LCD
 *
 * @return  None
 **************************************************************************************************/
//void HalLcd_HW_Control(uint8 cmd)
//{
//  LCD_SPI_BEGIN();
//  LCD_DO_CONTROL();
//  LCD_SPI_TX(cmd);
//  LCD_SPI_WAIT_RXRDY();
//  LCD_SPI_END();
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_Write
 *
 * @brief   Write 1 byte to the LCD
 *
 * @param   uint8 data - data to be written to the LCD
 *
 * @return  None
 **************************************************************************************************/
//void HalLcd_HW_Write(uint8 data)
//{
//  LCD_SPI_BEGIN();
//  LCD_DO_WRITE();
//  LCD_SPI_TX(data);
//  LCD_SPI_WAIT_RXRDY();
//  LCD_SPI_END();
//}

/**************************************************************************************************
 * @fn          HalLcd_HW_SetContrast
 *
 * @brief       Set display contrast
 *
 * @param       uint8 value - contrast value
 *
 * @return      none
 **************************************************************************************************/
//void HalLcd_HW_SetContrast(uint8 value)
//{
//  SET_ICONRAM_ADDR(CONTRAST_CTRL_REGISTER);
//  HalLcd_HW_Write(value);
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_Clear
 *
 * @brief   Clear the HW LCD
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
//void HalLcd_HW_Clear(void)
//{
//  uint8 n;
//
//  SET_DDRAM_ADDR(0x00);
//  for (n = 0; n < (LCD_MAX_LINE_COUNT * HAL_LCD_MAX_CHARS); n++)
//  {
//    HalLcd_HW_Write(' ');
//  }
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_ClearAllSpecChars
 *
 * @brief   Clear all special chars
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
//void HalLcd_HW_ClearAllSpecChars(void)
//{
//  uint8 n = 0;
//
//  SET_GCRAM_CHAR(0);
//  for (n = 0; n < (8 * 8); n++)
//  {
//    HalLcd_HW_Write(0x00);
//  }
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_WriteChar
 *
 * @brief   Write one char to the display
 *
 * @param   uint8 line - line number that the char will be displayed
 *          uint8 col - colum where the char will be displayed
 *
 * @return  None
 **************************************************************************************************/
//void HalLcd_HW_WriteChar(uint8 line, uint8 col, char text)
//{
//  if (col < HAL_LCD_MAX_CHARS)
//  {
//    SET_DDRAM_ADDR((line - 1) * HAL_LCD_MAX_CHARS + col);
//    HalLcd_HW_Write(text);
//  }
//  else
//  {
//    return;
//  }
//}

/**************************************************************************************************
 * @fn          halLcdWriteLine
 *
 * @brief       Write one line on display
 *
 * @param       uint8 line - display line
 *              char *pText - text buffer to write
 *
 * @return      none
 **************************************************************************************************/
//void HalLcd_HW_WriteLine(uint8 line, const char *pText)
//{
//  uint8 count;
//  uint8 totalLength = (uint8)osal_strlen( (char *)pText );
//
//  /* Write the content first */
//  for (count=0; count<totalLength; count++)
//  {
//    HalLcd_HW_WriteChar(line, count, (*(pText++)));
//  }
//
//  /* Write blank spaces to rest of the line */
//  for(count=totalLength; count<HAL_LCD_MAX_CHARS;count++)
//  {
//    HalLcd_HW_WriteChar(line, count, ' ');
//  }
//}

/**************************************************************************************************
 * @fn      HalLcd_HW_WaitUs
 *
 * @brief   wait for x us. @ 32MHz MCU clock it takes 32 "nop"s for 1 us delay.
 *
 * @param   x us. range[0-65536]
 *
 * @return  None
 **************************************************************************************************/
void HalLcd_HW_WaitUs(uint32 microSecs)
{
  while(microSecs--)
  {
    /* 32 NOPs == 1 usecs */
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}
#endif


/**************************************************************************************************
**************************************************************************************************/



