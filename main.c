  /*
 * Ordi.c
 *
 * Created: 19/06/2017 07:40:49 p. m.
 * Author : LuisEduardo
 */ 

#include <avr/io.h>
#define ticks(valor) (1/((valor*976)/100))/0.000004
//ADC
void ADC_init( void );
uint16_t ADC_read( uint8_t adc );
uint8_t ADC_validate( uint8_t adc );
//PORTS
void configBit( uint8_t nbit, uint8_t valor );
uint8_t nbit_validate( uint8_t nbit );
uint8_t readBit( uint8_t nbit );
uint8_t value_validate( uint8_t value );
void writeBit( uint8_t nbit, uint8_t valor );
//PWM
void PWM_init( void );
void PWM_OFF( void );
void PWM_ON( void );
uint8_t PWM_validate( uint8_t pwm );
uint8_t PWM_valueValidate( uint8_t value );
//STRINGS
uint16_t atoi( char *str );
void breakstr( char *str );
void getcads( char *miscomandos ,char *str );
uint8_t getCommand( char *str );
void itoa( char *str, uint16_t num, uint8_t base );
uint8_t strcomp( char *str1, char *str2 );
void strcopy( char *des, char *str );
//UART0
char UART0_getchar( void );
void UART0_init( void );
void UART0_gets( char *str );
void UART0_putchar( char dato );
void UART0_puts( char *str );

char commandos[][10]= 
{
	"cf", "wf", "rb", "wp", "rp", "ra"
};
char comando[20];
char str_lect[10];
char miscomandos[5][10];

int main(void)
{
	uint16_t lectura;
	uint8_t op,nbit, valor, pwm0,pwm1;
	char pwmRead[5];

    ADC_init();
    PWM_init();
    UART0_init();
    while (1) 
    {	
    	lectura=0;
    	op = 0;
    	nbit = 0;
		UART0_puts("\n\rEscriba su comando:\n\r");
		UART0_gets(comando);
		breakstr(comando);
    	op = getCommand(miscomandos[0]);
    	nbit = atoi(miscomandos[1]);
    	if( op<4 )
    	{
    		valor = atoi(miscomandos[2]);
    	}
		switch(op)
		{
			case 0:
				(nbit_validate(nbit)&&value_validate(valor))? configBit(nbit,valor):UART0_puts("\n\rError, por favor inténtelo de nuevo.");
				break;
			case 1:
				(nbit_validate(nbit)&&value_validate(valor))? writeBit(nbit,valor):UART0_puts("\n\rError, por favor inténtelo de nuevo.");
				break;
			case 2:
				if(nbit_validate(nbit))
				{
					UART0_puts("\n\r");
					( readBit(nbit) )? UART0_putchar('1'): UART0_putchar('0');
				}else
				{
					UART0_puts("\n\rError, por favor intentelo de nuevo.");
				}
				break;
			case 3: 
				if( PWM_validate(nbit) && PWM_valueValidate(valor) )
				{
					if(!TCCR0B)
					{
						PWM_ON();
					}
							
					if(nbit)
					{
						pwm1 = valor;
						OCR0B = ticks(valor);
					}else
					{
						pwm0 = valor;
						OCR0A = ticks(valor);
					}
					
				}else
				{
					UART0_puts("\n\rError, por favor intentelo de nuevo.");
				}
				break;
			case 4:
				if( PWM_validate(nbit) )
				{	
					if(!TCCR0B)
					{
						UART0_puts("\n\rError, PWM no encendido.");
					}else
					{
						if( nbit )
						{
							itoa(pwmRead,pwm1,10);
						}else
						{
							itoa(pwmRead,pwm0,10);
						}
							UART0_puts("\n\rEl pwm trabaja a: ");
							UART0_puts(pwmRead);
							UART0_puts("% \n\r");
					}
				}else
				{
					UART0_puts("\n\rError de pwm, por favor intentelo de nuevo.");
				}

				break;
			case 5:
				if(ADC_validate(nbit))
				{
					lectura = ADC_read(nbit);
					itoa(str_lect,lectura,10);
					UART0_puts("\n\rMedicion del adc seleccionado es:");
					UART0_puts(str_lect);
				}else
				{
					UART0_puts("\n\rError, por favor intentelo de nuevo.");
				}
				break;
			default:
				UART0_puts("\n\rIntente de nuevo.");
		}
    }
}

//ADC 
void ADC_init( void )
{
	ADCSRB = (1<<MUX5);
	ADCSRA = (7<<ADPS0); //128 ps
	DIDR2 = 0xFF; 		//Digital disable
}
uint16_t ADC_read( uint8_t adc )
{
	uint16_t lectura;
	ADMUX = (2<<REFS0)|(adc<<MUX0);
	ADCSRA |= (1<<ADEN)|(1<<ADSC);
	while( ADCSRA&(1<<ADSC) );	//mientras se hace la lectura
	lectura = ADCL;			
	lectura += (ADCH<<8);
	ADCSRA &=~(1<<ADEN);		//apagando adc
	return lectura;
}
uint8_t ADC_validate( uint8_t adc )
{
	return ( (adc<8)&&(adc>=0) )? 1:0;
}
//PORTS
void configBit( uint8_t nbit, uint8_t valor )
{
	(valor)?(DDRF |= (1<<nbit)):(DDRF &= ~(1<<nbit));
}
uint8_t nbit_validate( uint8_t nbit )
{
	return( (nbit>=0)&&(nbit<8) )? 1:0;
}
uint8_t readBit( uint8_t nbit )
{
	return (PINF&(1<<nbit))? 1:0;
}
uint8_t value_validate( uint8_t value )
{
	return( (value==1)||(value==0) )? 1:0;
}
void writeBit( uint8_t nbit, uint8_t valor )
{
	(valor)?(PORTF|=(1<<nbit)):(PORTF&=~(1<<nbit));
}
//PWM
void PWM_init()
{
	DDRB = (1<<PB7); //PWM0
	DDRG = (1<<PG5); //PWM1s
	TCCR0A = (2<<COM0A0)|(2<<COM0B0)|(2<<WGM00);
}
void PWM_ON( void )
{
	TCCR0B = (3<<CS00);
}
void PWM_OFF( void )
{
	TCCR0B = 0;
}
uint8_t PWM_validate( uint8_t pwm)
{
	return(pwm == 1 || pwm == 0)? 1:0;
}
uint8_t PWM_valueValidate( uint8_t value )
{
	return(value>=0 && value <=100)? 1:0;
}
//STRINGS===========================
void breakstr( char *str )
{
	char tmp[10];
	int i = 0;
	do{
		getcads(tmp,str);
		if(*tmp != '\0')
		{
			strcopy(miscomandos[i++],tmp);
		}
	}while( *tmp != '\0');
}
void getcads( char *substr ,char *str )
{
	char *temp;
	temp = str;
	while( *temp == ' ') temp++;
	while( (*temp != ' ')&&(*temp != '\0') )
	{
		*substr++ = *temp++;
	}
	*substr = '\0';
	strcopy(str,temp);
}
uint8_t getCommand(char *str)
{
	int i = 0;
	while( i < 6 )
	{
		if( strcomp(str,commandos[i]) )
		{
			return i;
		}
		i++;
	}
	return 6;
}
uint8_t strcomp( char *str1, char *str2 )
{
	while( *str1 && *str2)
	{
		if(*str1 != *str2)
		{
			return 0;
		}
		str1++;
		str2++;
	}
	return( (*str1||*str2)&&((*str1 == '\0')||(*str2 == '\0')) )? 0:1;
}
uint16_t atoi( char *str )
{	
	uint16_t num = 0;
	while(*str)
	{
		num = (num*10)+((*str++)-'0');		
	}
	return num;
}
void itoa( char *str, uint16_t num, uint8_t base )
{
	int i = 0, j = 0, aux;
	char dato;
	while( num )
	{
		aux = num/base;
		( (num%=base)>9 )?(*str++ = num+'7'):(*str++= num+'0');
		j++;
		num = aux;	
	}
	*str = '\0';
	str-= j;
	j--;
	while( i<j )
	{
		dato = str[i];
		str[i++] = str[j];
		str[j--] = dato;
	}
}
void strcopy( char *des, char *str )
{
	while( *str )
	{
		*des++ = *str++;
	}
	*des = '\0';
}
//UART===============================
void UART0_init( void )
{
	UCSR0A = (1<<U2X0);
	UCSR0B = (1<<TXEN0)|(1<<RXEN0);
	UCSR0C = (3<<UCSZ00);
	UBRR0 = 103;
}
void UART0_puts( char *str )
{
	while( *str )
	{
		UART0_putchar(*str++);
	}
}
void UART0_putchar( char dato )
{
	while( !(UCSR0A&(1<<UDRE0)) );
	UDR0 = dato;
}
char UART0_getchar( void )
{
	while( !(UCSR0A&(1<<RXC0)) );
	return UDR0;
}
void UART0_gets( char *str )
{
	char dato;
	int cnt = 0;
	while( dato != 13)
	{
		dato = UART0_getchar();
		if(dato != 8 && dato != 13)
		{
			UART0_putchar(dato);
			*str++ = dato;
			cnt++;

		}else if(dato == 8 && cnt >0 )
		{
			UART0_putchar('\b');
			UART0_putchar(' ');
			UART0_putchar(8);
			*--str = ('\0');
		}
	}
	*str = '\0';
}
