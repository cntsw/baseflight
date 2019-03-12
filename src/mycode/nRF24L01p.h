#ifndef _NRF24L01P_H_
#define _NRF24L01P_H_
#include "stm32f10x.h"
//************************************************�����趨********************************************************
/*˵��
1����ӦCE CSN����
2����Ҫspi�е�spi_RW(u8 dat)����
3��ע���ʼ��
4������Ҫ�ж�
5�������������м����������ж��¼�

void nRF_checkEvent(void)
{
	u8 sta=nRF_writeCmd(NOP);
	if( sta & MAX_RT ) 	//�ﵽ����ط�����
	{
		nRF_clearFlag(STATUS,MAX_RT);
	}
	if( sta & TX_DS )		//�������
	{
		nRF_clearFlag(STATUS,TX_DS);
	}
	if( sta & RX_DR )		//���յ�����
	{
		nRF_clearFlag(STATUS,RX_DR);
	}
}
*/
#include "spi1.h"
u8 spi_RW(u8 dat);
#define NRF_CE_H()		GPIO_SetBits(GPIOC,GPIO_Pin_7);
#define NRF_CE_L()		GPIO_ResetBits(GPIOC,GPIO_Pin_7);
#define NRF_CSN_H()		GPIO_SetBits(GPIOC,GPIO_Pin_6);
#define NRF_CSN_L()		GPIO_ResetBits(GPIOC,GPIO_Pin_6);

#define NRF_RX_PLOAD_WIDTH	32
#define NRF_TX_PLOAD_WIDTH	32
#define NRF_TX_ADR_WIDTH	5
#define NRF_RX_ADR_WIDTH	5
#define NRF_ADDR_RX			0xA0,0xB1,0x2C,0x3D,0x00
#define NRF_ADDR_TX			0xA0,0xB1,0x2C,0x3D,0x00


/*****************
	REGISTERS
*****************/
#define CONFIG		0x00	// �����շ�״̬��CRCУ��ģʽ�Լ��շ�״̬��Ӧ��ʽ
#define EN_AA		0x01	// �Զ�Ӧ��������
#define EN_RXADDR	0x02	// �����ŵ�����
#define SETUP_AW	0x03	// �շ���ַ�������
#define SETUP_RETR	0x04	// �Զ��ط���������
#define RF_CH		0x05	// ����Ƶ������
#define RF_SETUP	0x06	// �������ʡ����Ĺ�������
#define STATUS		0x07	// ״̬�Ĵ���
#define OBSERVE_TX	0x08	// ���ͼ�⹦��
#define RPD			0x09	// ��������̽��	
#define RX_ADDR_P0	0x0A	// Ƶ��0�������ݵ�ַ	
#define RX_ADDR_P1	0x0B	// Ƶ��1�������ݵ�ַ
#define RX_ADDR_P2	0x0C	// Ƶ��2�������ݵ�ַ
#define RX_ADDR_P3	0x0D	// Ƶ��3�������ݵ�ַ
#define RX_ADDR_P4	0x0E	// Ƶ��4�������ݵ�ַ
#define RX_ADDR_P5	0x0F	// Ƶ��5�������ݵ�ַ
#define TX_ADDR		0x10	// ���͵�ַ�Ĵ���
#define RX_PW_P0	0x11	// ����Ƶ��0�������ݳ���
#define RX_PW_P1	0x12	// ����Ƶ��1�������ݳ���
#define RX_PW_P2	0x13	// ����Ƶ��2�������ݳ���
#define RX_PW_P3	0x14	// ����Ƶ��3�������ݳ���
#define RX_PW_P4	0x15	// ����Ƶ��4�������ݳ���
#define RX_PW_P5	0x16	// ����Ƶ��5�������ݳ���
#define FIFO_STATUS	0x17	// FIFO״̬�Ĵ���
#define DYNPD		0x1C	// ����̬��Ч���ݳ���
#define FEATURE		0x1D	// �����Ĵ���

/*****************
	REG MASK
*****************/
#define STA_RX_DR		0x40	// RX Data Ready;�������ݽ���RXFIFOʱ��1 
#define STA_TX_DS		0x20	// TX Data Sent;�����ѷ��ͱ�־λ�����������AUTO_ACKֻ�е����յ�ACKʱ����1
#define STA_MAX_RT		0x10	// MAX number of TX retransmits interrupt

#define FIFO_TX_FULL 	0x20
#define FIFO_RX_EMPTY	0x01
//************************************************ģʽѡ��********************************************************
#define MODEL_RX				1			//��ͨ����
#define MODEL_TX				2			//��ͨ����
#define MODEL_RX2				3			//����ģʽ2,����˫����
#define MODEL_TX2				4			//����ģʽ2,����˫����

//***********************************************ȫ�ֱ�������*****************************************************
extern 	u8 	NRF24L01_RXDATA[NRF_RX_PLOAD_WIDTH];		//nrf24l01���յ�������
extern 	u8 	NRF24L01_TXDATA[NRF_RX_PLOAD_WIDTH];		//nrf24l01�����͵�����



typedef enum {
	RX=0,
	TX=1
}mRF_mode_t;
//*************************************************��������*******************************************************
void nRF_transmit(unsigned char *pBuf, unsigned char len);
void nRF_transmit_ackPayload(unsigned char * pBuf, unsigned char len);
void nRF_transmit_noAck(unsigned char * pBuf, unsigned char len);

unsigned char nRF_receive_getlen(void);
void nRF_receive(unsigned char *pBuf, unsigned char len);

void nRF_clearFlag(unsigned char reg,unsigned char flagBit);
u8 nRF_getFlag(unsigned char reg,unsigned char flagBit);

void nRF_FLUSE_TX(void);
void nRF_FLUSE_RX(void);

u8 nRF_init(mRF_mode_t mode, unsigned char ch);



u8 nRF_readReg(u8 reg);
u8 nRF_getStatus(void);
unsigned char nRF_canTransmit(void);
#endif
