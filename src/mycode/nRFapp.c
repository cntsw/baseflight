#include "nRFapp.h"

static u16 rc_data[8]={1500,1500,1500,1000,1000,1000,1000,1000};

extern u16 RC_timeout;
static u8 rx_buf[32];

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
		u8 rx_len = nRF_receive_getlen();
		if(rx_len<=32) {
			nRF_receive(rx_buf,rx_len);// read receive payload from RX_FIFO buffer
			rxdata_deal(rx_buf,rx_len);
		} else {
			nRF_FLUSE_RX();
		}
        nRF_clearFlag(STATUS,STA_RX_DR);
	}
}

