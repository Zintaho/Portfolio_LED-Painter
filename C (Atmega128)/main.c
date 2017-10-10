/*
* 8x8 LED Matrix Printer - main.c (유진석)
* ATMEGA128 기판에서 실제로 동작할 코드입니다.
* (최종 수정 2017-10-10)
*/

/*main 함수는 최 하단에 있다*/

//CPU Frequency 설정
#define F_CPU 16000000UL

//AVR 기판 관련 헤더
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

//사용자 정의 헤더
#include "pictures.h"

//매크로
#define ClearOutBit    PORTB &= ~(1<<PB1) 			 //기판과 LED Matrix간의 연결을 닫는다.
#define SetOutBit      PORTB |= (1<<PB1)  			 //기판과 LED Matrix간의 연결을 연다.

//블루투스 모듈과 안드로이드와의 통신을 위한 BAUD 값
#define BAUD_H			0
#define BAUD			103

//전역변수
RGB matrix[MAX] = {{0,0,0},};						 //8x8 LED Matrix 전체의 색상 정보.
unsigned char rx[4]; 								 //안드로이드에서 블루투스를 통해 받아들인 개별 데이터 (R, G, B, index)
													 //8-bit 기판이기 때문에 char를 사용.
unsigned char counterColor = 0; 					 //점멸된 총 개별 소자의 수



/* 
	아래의 두 함수에서 asm("nop")는 어셈블리어 No Operation이다. us 단위의 대기시간을 만들기 위한 방법. 
	반복문을 사용하지 않은 이유는, 반복문의 호출과정에서 시간이 흐르기 때문이다.
*/

//[연결단자를 0.4us 동안 개방 : LED Matrix가 0이라고 인식 (데이터 시트 참조)]
void Set0( void ) 
{
	SetOutBit;
	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); 
	ClearOutBit;
}
//[연결단자를 0.85us 동안 개방 : LED Matrix가 1이라고 인식 (데이터 시트 참조)]
void Set1( void ) 
{
	SetOutBit;
	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	ClearOutBit;
}
//[연결단자를 60us 동안 개방 : LED Matrix가 리셋으로 인식 (데이터 시트 참조)]
void Reset(void)
{
	ClearOutBit;
	_delay_us(60);
}

//[LED Matrix의 지정한 위치의 소자의 색상 정보를 변경 (Print 하기전엔 적용되지 않음)]
void SetCell(unsigned char R, unsigned char G, unsigned char B, unsigned char index)
{
	matrix[index].R = R;
	matrix[index].G = G;
	matrix[index].B = B;
}
//[LED Matrix를 인자로 지정한 프리셋의 색상대로 변경 (Print 하기전엔 적용되지 않음)]
void SetMatrix(RGB temp[MAX])
{
	int i=0;
	for(i = 0; i < MAX; i ++)
	{
		SetCell(temp[i].R,temp[i].G,temp[i].B,i);
	}
	
}

//[LED Matrix의 특정 소자(index)에, bit stream을 보내 색상을 적용시킴.]
//데이터 시트상 G, R, B순으로 전달한다.
void PrintCell(unsigned char index)
{
	unsigned char dummy = 0b10000000;
	int i = 0;
	
	for(i = 0; i < 8 ; i++)
	{
		(matrix[index].G & dummy) == dummy ? Set1() : Set0();
		dummy = dummy >> 1;
	}
	dummy = 0b10000000;
	
	for(i = 0; i < 8 ; i++)
	{
		(matrix[index].R & dummy) == dummy ? Set1() : Set0();
		dummy = dummy >> 1;
	}
	dummy = 0b10000000;
	for(i = 0; i < 8 ; i++)
	{
		(matrix[index].B & dummy) == dummy ? Set1() : Set0();
		dummy = dummy >> 1;
	}
	dummy = 0b10000000;
}

//[0~max번째 소자까지 PrintCell을 실행]
//내부 구조상, 특정 소자의 값만 변경하더라도, 그 소자 이전의 모든 소자들에 직렬로 접근해야한다.
//ex) 5번 소자를 바꾸려면 0,1,2,3,4번 소자에도 기존의 값을 bit stream으로 전달해야한다.
void PrintMatrix(int max)
{
	int i;
	
	for(i = 0; i < max; i++)
	{
		PrintCell(i);
	}
	
}

//[단자 초기화]
void init()
{
	//DDRA = 0b11111111;	//for debug
	DDRB = 0b00000010;	//Data Transport Pin
	
	UCSR0A = 0b00000000;/*Usart Control & Status Register A (initialize)*/
	
	UCSR0B = 0b10011000;	/*Usart Control & Status Register B
	
							 *7 RXCIE0: RX Complete Interrupt Enable
							 *4 RXEN0 : RX Enable
							 *3 TXEN0 : TX Enable
							 *2 UCSZ02: Character Size (UCSZ01, UCSZ00 is at UCSR0C)
							 UCSZ02 UCSZ01 UCSZ00
							 0		1		1		8-bit
							 */
	
	UCSR0C = 0b00000110;	/*Usart Control & Status Register C

							 *2 UCSZ01: Character Size 1:0 (UCSZ02 is at UCSR0B)
							 *1 UCSZ00:
							 */
	
	UBRR0H = BAUD_H;			/*Usart Baud Rate Register 11:0 (15:12 reserved)*/
	UBRR0L = BAUD;
}

//[블루투스 데이터 수신시 인터럽트 발생 : 전체 색상 정보를 설정하고, LED Matrix의 색상 정보를 갱신]
ISR(USART0_RX_vect)
{
	//PORTA = counterColor + 1; //for debug

	rx[counterColor++] = UDR0;
	if(counterColor == 4)
	{
		if(rx[3] == 64) //RESET CODE
		{
			SetMatrix(CLEAR);

			rx[3] = 0;
		}
		else
		{
			SetCell(rx[0],rx[1],rx[2],rx[3]);
		}
		
		counterColor = 0;
		Reset();
		PrintMatrix(MAX);
	}
}

int main(void)
{
	init();
	
	Reset();
	PrintMatrix(MAX);
	
	sei();					//Global Interrupt Enabled
	
	for(;;)
	{
		
	}
	
	return 0;
}