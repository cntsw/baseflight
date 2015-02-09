#include "spi1.h"


void spi1_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);
	
	/*���� SPI_NRF_SPI�� SCK,MISO,MOSI���ţ�GPIOA^5,GPIOA^6,GPIOA^7 */ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //���ù��� 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 		//CE
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 		//CSN
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	
	
	/*
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;		//irq
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	*/
}

void spi1_modeInit(void)
{
	SPI_InitTypeDef SPI_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 	//˫��ȫ˫�� 
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 											//��ģʽ 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 									//���ݴ�С8λ 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 													//ʱ�Ӽ��ԣ�����ʱΪ�� 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 												//��1��������Ч��������Ϊ����ʱ�� 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 													//NSS�ź���������� 
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 	//8��Ƶ��9MHz 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 									//��λ��ǰ 
	SPI_InitStructure.SPI_CRCPolynomial = 7; 
	SPI_Init(SPI1, &SPI_InitStructure); 
}

void spi1_init(void)
{
	spi1_GPIOInit();
	spi1_modeInit();
	SPI_Cmd(SPI1, ENABLE);
}

u8 spi_RW(u8 dat) 
{

	while( (SPI1->SR & SPI_I2S_FLAG_TXE )	== 0);
	SPI1->DR = dat; 
	while( (SPI1->SR & SPI_I2S_FLAG_RXNE) == 0);
	return SPI1->DR;

	/*
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 	//�� SPI���ͻ������ǿ�ʱ�ȴ�
	SPI_I2S_SendData(SPI1, dat); 																			//ͨ�� SPI2����һ�ֽ�����
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); 	//��SPI���ջ�����Ϊ��ʱ�ȴ�
	return SPI_I2S_ReceiveData(SPI1);																	//Return the byte read from the SPI bus
	*/
}
