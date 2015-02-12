#include "nRFapp.h"

static u16 rc_data[8]= {1500,1500,1500,1000,1000,1000,1000,1000};

extern u16 RC_timeout;


static void rxdata_deal(u8 *pBuf,u8 len)
{
	u8 sum = 0;
	u8 i;
	if(*(pBuf)!=0xAA) return;
	if(*(pBuf+1)!=0x55) return;
	for(i=0; i<(len-1); i++)
		sum += *(pBuf+i);
	if(!(sum==*(pBuf+len-1)))	return;

	if(*(pBuf+2)==0X03) {
		rc_data[0]	= (int16_t)(*(pBuf+4)<<8)	| *(pBuf+5);
		rc_data[1] = (int16_t)(*(pBuf+6)<<8)	| *(pBuf+7);
		rc_data[2] = (int16_t)(*(pBuf+8)<<8)	| *(pBuf+9);
		rc_data[3] = (int16_t)(*(pBuf+10)<<8)   | *(pBuf+11);
		rc_data[4]	= (int16_t)(*(pBuf+12)<<8)	| *(pBuf+13);
		rc_data[5] = (int16_t)(*(pBuf+14)<<8)	| *(pBuf+15);
		rc_data[6] = (int16_t)(*(pBuf+16)<<8)	| *(pBuf+17);
		rc_data[7] = (int16_t)(*(pBuf+18)<<8)	| *(pBuf+19);
		RC_timeout=0;
	}
}


uint16_t nRFReadRawRC(uint8_t chan)
{
	return rc_data[chan];
}


/**********************************************************************************/

void nrfWrite(serialPort_t *instance, uint8_t ch)
{
	nrfPort_t *s = (nrfPort_t *)instance;
	s->port.txBuffer[s->port.txBufferHead] = ch;
	s->port.txBufferHead = (s->port.txBufferHead + 1) % s->port.txBufferSize;
}

uint8_t nrfTotalBytesWaiting(serialPort_t *instance)
{
	nrfPort_t *s = (nrfPort_t*)instance;
	// FIXME always returns 1 or 0, not the amount of bytes waiting
	return s->port.rxBufferTail != s->port.rxBufferHead;
}

uint8_t nrfRead(serialPort_t *instance)
{
	uint8_t ch;
	nrfPort_t *s = (nrfPort_t *)instance;
	ch = s->port.rxBuffer[s->port.rxBufferTail];//从尾往头读
	s->port.rxBufferTail = (s->port.rxBufferTail + 1) % s->port.rxBufferSize;
	return ch;
}
// Specified baud rate may not be allowed by an implementation, use nrfGetBaudRate to determine actual baud rate in use.
void nrfSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
}
bool isnrfTransmitBufferEmpty(serialPort_t *instance)
{
	nrfPort_t *s = (nrfPort_t *)instance;
	return s->port.txBufferTail == s->port.txBufferHead;
}
void nrfSetMode(serialPort_t *instance, portMode_t mode)
{
}

const struct serialPortVTable nrfVTable[] = {
	{
		nrfWrite,
		nrfTotalBytesWaiting,
		nrfRead,
		nrfSetBaudRate,
		isnrfTransmitBufferEmpty,
		nrfSetMode,
	}
};


nrfPort_t nrfPort;

serialPort_t *nRF_serial(void)
{
	nrfPort_t *s = NULL;

	static volatile uint8_t rx1Buffer[512];
	static volatile uint8_t tx1Buffer[512];
	s = &nrfPort;
	s->port.vTable = nrfVTable;
	s->port.baudRate = 0;
	s->port.rxBuffer = rx1Buffer;
	s->port.txBuffer = tx1Buffer;
	s->port.rxBufferSize = 512;
	s->port.txBufferSize = 512;

	return (serialPort_t *)s;
}

//////////////////////////////////////
serialPort_t *RFPort=(serialPort_t*)&nrfPort;
void nRF_checkEvent(void)
{
	u8 sta=nRF_getStatus();
	if( sta & STA_MAX_RT ) {//达到最大重发次数
		nRF_FLUSE_TX();
		nRF_clearFlag(STATUS,STA_MAX_RT);
	}
	if( sta & STA_TX_DS ) {//发送完成
		nRF_clearFlag(STATUS,STA_TX_DS);
	}
	if( sta & STA_RX_DR ) {//接收到数据
		u8 _cnt=0;
		do {
			u8 rx_buf[32];
			u8 rx_len = nRF_receive_getlen();
			if(rx_len<33 && rx_len>0) {
				nRF_receive(rx_buf,rx_len);// read receive payload from RX_FIFO buffer
				if(*rx_buf==0xda) {	//电脑端传来的数据
					u8 cnt=0;
					cnt++,rx_len--;//忽略第一个字节
					for(; rx_len; cnt++,rx_len--) { //将其他所有的数据依次送入nRF的rxBuffer  注意，实例只能读rx，此处为操作函数所以可以写rx
						nrfPort.port.rxBuffer[nrfPort.port.rxBufferHead] = rx_buf[cnt];
						nrfPort.port.rxBufferHead = (nrfPort.port.rxBufferHead + 1) % nrfPort.port.rxBufferSize;
					}
				} else {	//用户遥控数据
					rxdata_deal(rx_buf,rx_len);
				}
			} else {
				nRF_FLUSE_RX();
			}
			_cnt++;
			if(_cnt>5) break;	//防守程序，飞机不能循环太长时间
		} while(!(nRF_readReg(FIFO_STATUS) & FIFO_RX_EMPTY));
		nRF_clearFlag(STATUS,STA_RX_DR);
	}

	if(!isSerialTransmitBufferEmpty(RFPort)) { //如果nRF写入缓冲区非空  //此处当实例来操作不太恰当
		if(nRF_canTransmit()) { //TX FIFO enable
			u8 dat_buf[32];
			u8 cnt=0;
			dat_buf[cnt++]=0xda;
			while(!isSerialTransmitBufferEmpty(RFPort)) { //FIXME 此函数体是实例抽象后的执行函数，这样写不太妥当
				dat_buf[cnt]=nrfPort.port.txBuffer[nrfPort.port.txBufferTail];
				nrfPort.port.txBufferTail = (nrfPort.port.txBufferTail + 1) % nrfPort.port.txBufferSize; //从尾往头发送
				cnt++;
				if(cnt==32) break;
			}
			nRF_transmit_ackPayload(dat_buf,cnt); //发送一包数据 以0xda开头
		}
		//else {
		//	nRF_FLUSE_TX();
		//}
	}

//	sta=nRF_readReg(FIFO_STATUS);
//	if(sta & 0x02) {
//		nRF_FLUSE_RX();
//	}
}
