#include	"nrf24l01p.h"

/*
SPI����--
	���е�ƽ����
	������Ч��������
*/
unsigned char payload_rx[NRF_RX_PLOAD_WIDTH];//nrf24l01���յ�������
unsigned char payload_tx[NRF_RX_PLOAD_WIDTH];//nrf24l01��Ҫ���͵�����
unsigned char address_rx[NRF_RX_ADR_WIDTH]={NRF_ADDR_RX};//���յ�ַ
unsigned char address_tx[NRF_TX_ADR_WIDTH]={NRF_ADDR_TX};//���ص�ַ


/**********************************************************************************************************/
/*						����������							*/
/*****************
	COMMANDS
*****************/
#define R_REGISTER			0x00	// ���Ĵ���
#define W_REGISTER			0x20	// д�Ĵ���
#define RD_RX_PLOAD			0x61	// ��ȡ��������
#define WR_TX_PLOAD			0xA0	// д��������
#define FLUSH_TX			0xE1	// ���TX����(����TXģʽ)
#define FLUSH_RX			0xE2	// ���RX����(����RXģʽ)
#define REUSE_TX_PL			0xE3	// �����ظ�װ������ָ��	(TX)
#define NOP					0xFF	// �޲���

#define R_RX_PL_WID			0x60	// ��ȡ�������ݳ���
#define W_ACK_PAYLOAD		0xA8	// ACK��������������
#define W_TX_PAYLOAD_NOACK	0xB0	// ָ�������ݰ����Զ�ACK
//����������nRF24L01+����ָ�������FEATURE�Ĵ���
/*****************
	����ָ��
*****************/
static unsigned char command(unsigned char cmd)
{
	unsigned char status;
	NRF_CSN_L();
	status=spi_RW(cmd);
	NRF_CSN_H();
	return status;
}

/*****************
	д�Ĵ���
*****************/
static unsigned char wReg(unsigned char reg,unsigned char value)
{
	unsigned char status;
	NRF_CSN_L();
	status=spi_RW(W_REGISTER+reg);//�ڴ˼���W_REGISTER
	spi_RW(value);
	NRF_CSN_H();
	return 	status;
}

/*****************
	���Ĵ���
*****************/
static	unsigned char rReg(unsigned char reg)
{
	unsigned char reg_val;
	NRF_CSN_L();
	spi_RW(R_REGISTER+reg);//�ڴ˼���R_REGISTER
	reg_val=spi_RW(NOP);
	NRF_CSN_H();
	return 	reg_val;
}

/***************************
	д����1�ֽڵļĴ���
***************************/
static	unsigned char wReg2(unsigned char reg,unsigned char *pReg,unsigned char len)
{
	unsigned char i;
	unsigned char status;
	NRF_CSN_L();
	status=spi_RW(W_REGISTER+reg);
	for(i=0;i<len;i++){
		spi_RW(pReg[i]);
	}
	NRF_CSN_H();
	return 	status;
}

/**************************
	������1�ֽڵļĴ���
**************************/
static	unsigned char rReg2(unsigned char reg,unsigned char *pReg,unsigned char len)
{
	unsigned char i;
	unsigned char status;
	NRF_CSN_L();
	status=spi_RW(R_REGISTER+reg);
	for(i=0;i<len;i++){
		pReg[i]=spi_RW(NOP);
	}
	NRF_CSN_H();
	return status;
}

/*******************
	�����ֽڲ���
*******************/
static	void wBuf(unsigned char start,unsigned char *pBuf,unsigned char len)
{
	unsigned char i;
	NRF_CSN_L();
	spi_RW(start);
	for(i=0;i<len;i++){
		spi_RW(pBuf[i]);
	}
	NRF_CSN_H();
}
static	void rBuf(unsigned char start,unsigned char *pBuf,unsigned char len)
{
	unsigned char i;
	NRF_CSN_L();
	spi_RW(start);
	for(i=0;i<len;i++){
		pBuf[i]=spi_RW(NOP);
	}
	NRF_CSN_H();
}
/**********************************************************************************************************/


/**********************************************************************************************************/
/*						Ӧ�ú�����							*/
/*****************
	�������ݰ�
*****************/
void nRF_transmit(unsigned char *pBuf,unsigned char len)
{
	NRF_CE_L();// StandBy	Iģʽ
	wBuf(WR_TX_PLOAD,pBuf,len);// װ������
	NRF_CE_H();// �ø�CE���������ݷ���
}
void nRF_transmit_ackPayload(unsigned char *pBuf,unsigned char len)
{
	NRF_CE_L();
	wBuf(W_ACK_PAYLOAD,pBuf,len);
	NRF_CE_H();
}
void nRF_transmit_noAck(unsigned char *	pBuf,unsigned char len)
{
	NRF_CE_L();
	wBuf(W_TX_PAYLOAD_NOACK,pBuf,len);
	NRF_CE_H();
}

/*****************
	�������ݰ�
*****************/
unsigned char nRF_receive_getlen()
{
	return rReg(R_RX_PL_WID);
}
void nRF_receive(unsigned char *pBuf,unsigned char len)
{
	rBuf(RD_RX_PLOAD,pBuf,len);
}

/*********************
		��־λ���
*********************/
void nRF_clearFlag(unsigned char reg,unsigned char flagBit)
{
	wReg(reg,flagBit);//д1�����־λ
}
unsigned char nRF_getFlag(unsigned char reg,unsigned char flagBit)
{
	unsigned char val=rReg(reg);
	return (val & flagBit) && 1;
}

/****************
	nRFָ��
****************/
void nRF_FLUSE_TX(void)
{
	command(FLUSH_TX);
}
void nRF_FLUSE_RX(void)
{
	command(FLUSH_RX);
}


/********************
	���nRF�豸
********************/
unsigned char nRF_check(void)
{
	unsigned char sendDat[NRF_TX_ADR_WIDTH];
	unsigned char i;
	wReg2(TX_ADDR,address_tx,NRF_TX_ADR_WIDTH);//д��5���ֽڵĵ�ַ
	rReg2(TX_ADDR,sendDat,NRF_TX_ADR_WIDTH);//����д��ĵ�ַ
	for(i=0;i<NRF_TX_ADR_WIDTH;i++){
		if(sendDat[i]!=address_tx[i])
			break;
	}
	if(i==5)
		return 0	;//MCU��NRF�ɹ�����
	else
		return 1	;//MCU��NRF����������
}

/*****************
	��ʼ��
*****************/

unsigned char nRF_init(mRF_mode_t	mode,unsigned char ch)
{
	NRF_CE_L();
	if(nRF_check()!=0) return 1;

	//д��ַ
	wReg2(RX_ADDR_P0,address_rx,NRF_RX_ADR_WIDTH);
	wReg2(TX_ADDR,address_tx,NRF_TX_ADR_WIDTH);

	wReg(EN_AA,0x01);// ʹ��ͨ��0���Զ�Ӧ��
	wReg(EN_RXADDR,0x01);// ʹ��ͨ��0�Ľ��յ�ַ
	wReg(SETUP_RETR,0x1a);// �����Զ��ط����ʱ��:500us;����Զ��ط�����:10��
	wReg(RF_CH,ch);// ����RFͨ��CHANAL
	wReg(RF_SETUP,0x0e);// 0db,2Mbps

	wReg(DYNPD,0x01);// ͨ��0��̬���ݳ���
	wReg(FEATURE,0x06);// ����Я������ACK�Ͷ�̬���ݳ���

	if(mode==TX){
		wReg(CONFIG,0x0e);
	}	else	{
		wReg(CONFIG,0x0f);
	}

	NRF_CE_H();
	return 0;
}


unsigned char nRF_readReg(unsigned char reg)
{
	return rReg(reg);
}

unsigned char nRF_getStatus(void)
{
	return command(NOP);
}
