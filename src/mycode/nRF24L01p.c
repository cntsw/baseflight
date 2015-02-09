#include	"nrf24l01p.h"

/*
SPI描述--
	空闲电平：低
	数据有效：上升沿
*/
unsigned char payload_rx[NRF_RX_PLOAD_WIDTH];//nrf24l01接收到的数据
unsigned char payload_tx[NRF_RX_PLOAD_WIDTH];//nrf24l01需要发送的数据
unsigned char address_rx[NRF_RX_ADR_WIDTH]={NRF_ADDR_RX};//接收地址
unsigned char address_tx[NRF_TX_ADR_WIDTH]={NRF_ADDR_TX};//本地地址


/**********************************************************************************************************/
/*						基本函数区							*/
/*****************
	COMMANDS
*****************/
#define R_REGISTER			0x00	// 读寄存器
#define W_REGISTER			0x20	// 写寄存器
#define RD_RX_PLOAD			0x61	// 读取接收数据
#define WR_TX_PLOAD			0xA0	// 写待发数据
#define FLUSH_TX			0xE1	// 清除TX缓存(用于TX模式)
#define FLUSH_RX			0xE2	// 清除RX缓存(用于RX模式)
#define REUSE_TX_PL			0xE3	// 定义重复装载数据指令	(TX)
#define NOP					0xFF	// 无操作

#define R_RX_PL_WID			0x60	// 读取接收数据长度
#define W_ACK_PAYLOAD		0xA8	// ACK包附带发送数据
#define W_TX_PAYLOAD_NOACK	0xB0	// 指定的数据包不自动ACK
//以上三个是nRF24L01+扩充指令，需设置FEATURE寄存器
/*****************
	发送指令
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
	写寄存器
*****************/
static unsigned char wReg(unsigned char reg,unsigned char value)
{
	unsigned char status;
	NRF_CSN_L();
	status=spi_RW(W_REGISTER+reg);//在此加上W_REGISTER
	spi_RW(value);
	NRF_CSN_H();
	return 	status;
}

/*****************
	读寄存器
*****************/
static	unsigned char rReg(unsigned char reg)
{
	unsigned char reg_val;
	NRF_CSN_L();
	spi_RW(R_REGISTER+reg);//在此加上R_REGISTER
	reg_val=spi_RW(NOP);
	NRF_CSN_H();
	return 	reg_val;
}

/***************************
	写大于1字节的寄存器
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
	读大于1字节的寄存器
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
	连续字节操作
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
/*						应用函数区							*/
/*****************
	发送数据包
*****************/
void nRF_transmit(unsigned char *pBuf,unsigned char len)
{
	NRF_CE_L();// StandBy	I模式
	wBuf(WR_TX_PLOAD,pBuf,len);// 装载数据
	NRF_CE_H();// 置高CE，激发数据发送
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
	接收数据包
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
		标志位相关
*********************/
void nRF_clearFlag(unsigned char reg,unsigned char flagBit)
{
	wReg(reg,flagBit);//写1清除标志位
}
unsigned char nRF_getFlag(unsigned char reg,unsigned char flagBit)
{
	unsigned char val=rReg(reg);
	return (val & flagBit) && 1;
}

/****************
	nRF指令
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
	检测nRF设备
********************/
unsigned char nRF_check(void)
{
	unsigned char sendDat[NRF_TX_ADR_WIDTH];
	unsigned char i;
	wReg2(TX_ADDR,address_tx,NRF_TX_ADR_WIDTH);//写入5个字节的地址
	rReg2(TX_ADDR,sendDat,NRF_TX_ADR_WIDTH);//读出写入的地址
	for(i=0;i<NRF_TX_ADR_WIDTH;i++){
		if(sendDat[i]!=address_tx[i])
			break;
	}
	if(i==5)
		return 0	;//MCU与NRF成功连接
	else
		return 1	;//MCU与NRF不正常连接
}

/*****************
	初始化
*****************/

unsigned char nRF_init(mRF_mode_t	mode,unsigned char ch)
{
	NRF_CE_L();
	if(nRF_check()!=0) return 1;

	//写地址
	wReg2(RX_ADDR_P0,address_rx,NRF_RX_ADR_WIDTH);
	wReg2(TX_ADDR,address_tx,NRF_TX_ADR_WIDTH);

	wReg(EN_AA,0x01);// 使能通道0的自动应答
	wReg(EN_RXADDR,0x01);// 使能通道0的接收地址
	wReg(SETUP_RETR,0x1a);// 设置自动重发间隔时间:500us;最大自动重发次数:10次
	wReg(RF_CH,ch);// 设置RF通道CHANAL
	wReg(RF_SETUP,0x0e);// 0db,2Mbps

	wReg(DYNPD,0x01);// 通道0动态数据长度
	wReg(FEATURE,0x06);// 开启携带数据ACK和动态数据长度

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
