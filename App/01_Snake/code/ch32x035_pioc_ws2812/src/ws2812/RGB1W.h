// 1-wire example: 1W-RGB, 1W-DS1820

#include "ch32x035.h"

#include	"PIOC_SFR.h"

#define  RGB_Total_Number 1024              //RGB��������
#define  RGB_State        9               //�ƹ�״̬��
#define  Mode_Select      1               //1:ʹ�÷�ʽ1�������������ʹ󲿷�RGB  0��ʹ�÷�ʽ2���������޸�ʱ�����

#define  RGB1W_CYC       RGB1W_CYC_48M    //������������
#define  RGB1W_CYC_48M   56               // bit cycle for Fsys = 48MHz, 1.167uS/cycle
#define  RGB1W_CYC_24M   28               // bit cycle for Fsys = 24MHz, 1.167uS/cycle
#define  RGB1W_CYC_16M   19               // bit cycle for Fsys = 16MHz, 1.167uS/cycle
#define  RGB1W_CYC_12M   14               // bit cycle for Fsys = 12MHz, 1.167uS/cycle
#define  RGB1W_CYC_8M    9                // bit cycle for Fsys = 8MHz, 1.167uS/cycle

#define  RGB1W_CMD_RAM   0x80    // notice PIOC to send data from RAM

// RGB����ʱ���������
#define RGB_DI_T0H     300            //��λ:ns
#define RGB_DI_T0L     900
#define RGB_DI_T1H     900
#define RGB_DI_T1L     300            //
#define RGB_DI_Trst    300

#define PIOC_CYC        PIOC_Timer_48M       //ָ������
#define PIOC_Timer_48M   (1000 / 48)         //48MHZ��ָ������  ��20.83  ns
#define PIOC_Timer_24M   (1000 / 24)         //24MHZ��ָ������  ��41.67  ns
#define PIOC_Timer_16M   (1000 / 16)         //16MHZ��ָ������  ��62.5  ns
#define PIOC_Timer_12M   (1000 / 12)         //12MHZ��ָ������  ��83.3  ns


#define		RGB1W_SFR_ADDR	((uint8_t *)&(PIOC->D8_DATA_REG0))	// RGB1W data buffer start address for SFR mode
#define		RGB1W_SFR_SIZE	32						// RGB1W data buffer size for SFR mode
#define		RGB1W_RAM_ADDR	((uint8_t *)(PIOC_SRAM_BASE+0x400))	// RGB1W data buffer start address for RAM mode
#define     RGB1W_RAM_ADDR1  ((uint8_t *)(PIOC_SRAM_BASE+0x200))
#define		RGB1W_RAM_SIZE	((uint8_t *)(PIOC_SRAM_BASE+0x1000)-RGB1W_RAM_ADDR)	// RGB1W data buffer size for RAM mode

#define		RGB1W_FREQ_CFG	(0x000C*2)	// system frequency config



#define		RGB1W_COMMAND	(PIOC->D8_CTRL_WR)	// command input
// 1~RGB1W_SFR_SIZE: RGB SFR mode, 0x01-0x20: RGB with short data in SFR, low 7bits is total byte
// > RGB1W_CMD_RAM:  RGB RAM mode, 0x88-0xFC: RGB with long data in code RAM, low 7bits is RGB1W_CYC_*, bit7 is RGB1W_CMD_RAM
// 0x41: start temperature convert
// 0x42: get temperature data
// 0x43: start temperature convert then get data

#define		CMD_T_START		0x41	// start temperature convert
#define		CMD_T_GET		0x42	// get temperature data
#define		CMD_T_STA_GET	0x43	// start temperature convert then get data

#define		RGB1W_ERR_OK	0		// error code for success
#define		RGB1W_ERR_CMD	1		// error code for command error
#define		RGB1W_ERR_PARA	2		// error code for parameter error
#define		RGB1W_ERR_OUTH	4		// error code for pin high at the end
#define		RGB1W_ERR_PINH	6		// error code for pin high at the start




extern  uint8_t RGBpbuf1[ RGB_Total_Number ][ 3 ] ;        //����144��RGB�ĵƹ�����
extern  uint8_t RGBpbuf[ RGB_State ][ 3 ] ;






extern	__IO	uint8_t		stat;

extern	const unsigned char PIOC_1W_CODE[] __attribute__((aligned (4)));

void PIOC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void RGB1W_Init ( void );

void RGB1W_SendSFR( uint16_t total_bytes, uint8_t *p_source_addr );  //SFR mode for 1~32 bytes data
void RGB_Single_Set( uint16_t RGB_Number, uint8_t Green, uint8_t Red, uint8_t Blue );
void RGB_Whole_Set( uint8_t Green,uint8_t Red ,uint8_t Blue );
void RGB1W_SendRAM( uint16_t total_bytes, uint8_t *p_source_addr );  //RAM mode for 1~3072 bytes data

uint8_t RGB1W_SendSFR_Wait( uint16_t total_bytes, uint8_t *p_source_addr );  //SFR mode for 1~32 bytes data

uint8_t RGB1W_SendRAM_Wait( uint16_t total_bytes, uint8_t *p_source_addr );  //RAM mode for 1~3072 bytes data

void RGB1W_Halt( void );  //halt/sleep PIOC
