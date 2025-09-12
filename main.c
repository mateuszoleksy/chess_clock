#include <avr/io.h>
#include <util/delay.h>

int sec=0;
int set=0;

// Konfiguracja portĂłw
void initPorts() {
	// Konfiguracja portu E jako wyjĹcie (sterowanie segmentami)
	DDRE = 0xFF;
	PORTE = 0x00;

	// Konfiguracja portu D jako wyjĹcie (wybĂłr segmentu)
	DDRD = 0xFF;
	PORTD = 0x00;
}

const uint8_t keyMapSet[4][4] = {
	{ 10,  3,  2,  1 },
	{ 11,  6,  5,  4 },
	{ 12,  9,  8,  7},
	{ 15, 14, 13, 0 }
};

// Odczyt klawiatury matrycowej
uint8_t readKeypad() {
	// kolumny wejscie
	uint8_t col = 0;
	uint8_t row = 0;
	uint8_t row2 = 0;
	DDRB = 0x0f;
	DDRC = 0xf0;
	PORTC = 0x0f;
	col = PINC & 0x0f;
	DDRC = 0x0f;
	PORTC = 0xf0;
	row = PINC & 0xf0;
	DDRB = 0xf0;
	PORTB = 0x0f;
	row2 = PINB & 0x0f;
	if (row2 == 13)
		row = 1;
	if (row2 == 14)
		row = 0;
	if (row == 16)
		row = 2;
	if (row == 32)
		row = 3;
	switch(col)
	{
		case 0:
			return 17;
		case 1:
		col = 0;
		break;
		case 2:
		col = 1;
		break;
		case 4:
		col = 2;
		break;
		case 8:
		col = 3;
		break;
	}
	return keyMapSet[col][row];
}

// Mapowanie cyfr na segmenty 7-segmentowe
const uint8_t digitToSegment[10] = {
	0x7E, // 0
	0x30, // 1
	0x6D, // 2
	0x79, // 3
	0x33, // 4
	0x5B, // 5
	0x5F, // 6
	0x70, // 7
	0x7F, // 8
	0x7B,  // 9
	0x77, // a
	0x1F, // b
	0x4E, // c
	0x3D, // d
	0x4F, // e
	0x47 // e
};

// WyĹwietlanie cyfry na wyĹwietlaczu 7-segmentowym
void displayDigit(uint8_t digit, uint8_t position, uint8_t point) {
	PORTD = ~digitToSegment[digit]; // Ustawienie segmentĂłw
	if (point)
		PORTD &= ~(1<<PORTD7);
	PORTE = ~position; // Ustawienie aktywnego segmentu (anody)
	_delay_ms(20); // KrĂłtkie opĂłĹşnienie dla stabilnoĹci wyĹwietlania
	sec+=1;
};

void wait(unsigned int delay)
{
	volatile unsigned int i;
	for(i=0; i<delay; i++);
};

// Funkcja gĹĂłwna
int main() {
	initPorts();
	
	long timeLeft[2][2] = {{1, 0}, {1, 0}}; // Czas dla dwĂłch graczy (musi byÄ zdefiniowany odpowiednio)
	uint8_t currentPlayer = 0;
	uint8_t key;
	uint8_t pause = 0;
	uint8_t old_key = 0;
	uint8_t blink = 0;
	uint8_t number = 0;
	uint8_t flashing = 0;
	int old_value=0;
	int display;
	while (1) {
		key = readKeypad();
		if (!set)
		{
			if (key == 17)
			{
				old_key = 17;
			}
			if (key == 10 && old_key != 10)
			{
				currentPlayer = 1 - currentPlayer;
				old_key = 10;
			}
			if (key == 3 && old_key != 3)
			{
				pause = 1 - pause;
				old_key = 3;
			}
			if (key == 4 && old_key != 4)
			{
				if (timeLeft[currentPlayer][0]+1 < 99)
				timeLeft[currentPlayer][0] += 1;
				old_key = 4;
			}
			if (key ==5 && old_key != 5)
			{
				if (timeLeft[currentPlayer][1] + 10000 <= 59000)
				{
					timeLeft[currentPlayer][1] += 10000;
				}	else
				{
					timeLeft[currentPlayer][0] += 1;
					timeLeft[currentPlayer][1] = 0;
				}
				old_key = 5;
			}
			if (key == 6 && old_key != 6)
			{
				if (timeLeft[currentPlayer][1]+1000 <= 59000)
				{
					timeLeft[currentPlayer][1] += 1000;
				}	else
				{
					timeLeft[currentPlayer][0] += 1;
					timeLeft[currentPlayer][1] = 0;
				}
				old_key = 6;
			}
			if (key == 7 && old_key != 7)
			{
				if (timeLeft[currentPlayer][0] > 0)
					timeLeft[currentPlayer][0] -= 1;
				old_key = 7;
			}
			if (key == 8 && old_key != 8)
			{
				if (timeLeft[currentPlayer][1]-10000 > 0)
				{
					timeLeft[currentPlayer][1] -= 10000;
				}	else if (timeLeft[currentPlayer][0] > 0)
				{
					timeLeft[currentPlayer][0] -= 1;
					timeLeft[currentPlayer][1] = 60000;
				}
				old_key = 8;
			}
			if (key == 9 && old_key != 9)
			{
				if (timeLeft[currentPlayer][1]-1000 > 0)
				{
					timeLeft[currentPlayer][1] -= 1000;
				}	else if (timeLeft[currentPlayer][0] > 0)
				{
					timeLeft[currentPlayer][0] -= 1;
					timeLeft[currentPlayer][1] = 60000;
				}
				old_key = 9;
			}
			if (key == 2 && old_key != 2)
			{
				timeLeft[currentPlayer][1] = 0;
				old_key = 2;
			}
			if (key == 1 && old_key != 1)
			{
				timeLeft[currentPlayer][0] = 0;
				old_key = 1;
			}
			if (key == 14 && old_key != 14)
			{
				timeLeft[currentPlayer][0] = timeLeft[1-currentPlayer][0];
				timeLeft[currentPlayer][1] = timeLeft[1-currentPlayer][1];
				old_key = 14;
			}
			if (key == 15 && old_key != 15)
			{
				set = 1;
				old_key = 15;
				flashing = 1;
				old_value = 0;
				pause = 0;
				_delay_ms(2000);
			}
		} else if (set)
		{
			
			if (old_key != key)
			{
				if (key==15)
				{
					set = 0;
					flashing = 0;
					old_key = 15;
					number = 0;
					_delay_ms(2000);
				}
				if (key==14)
				{
					timeLeft[currentPlayer][0] = timeLeft[1-currentPlayer][0];
					timeLeft[currentPlayer][1] = timeLeft[1-currentPlayer][1];
				}
				if (key==10)
				{
					currentPlayer = 1 - currentPlayer;
				}
				old_key = key;
				
				if (key >= 0 && key <= 9)
				{
					number++;
					switch (number)
					{
						case 1:
						timeLeft[currentPlayer][0] = 0;
						timeLeft[currentPlayer][0] += key;
						old_value = key;
						break;
						case 2:
						timeLeft[currentPlayer][0] = 0;
						timeLeft[currentPlayer][0] += key;
						timeLeft[currentPlayer][0] += old_value*10;
						old_value = 0;
						break;
						case 3:
						timeLeft[currentPlayer][1] = 0;
						timeLeft[currentPlayer][1] += key*1000;
						old_value = key;
						break;
						case 4:
						timeLeft[currentPlayer][1] = 0;
						timeLeft[currentPlayer][1] += key*1000;
						if (old_value >= 6 && timeLeft[currentPlayer][0] < 99)
						{
							timeLeft[currentPlayer][0] += 1;
							
						} else
						{
							for (int i=0; i<old_value;i++)
							{
								timeLeft[currentPlayer][1] += 10000;
							};
						}
						number = 0;
						set = 0;
						flashing = 0;
						_delay_ms(2000);
						break;
					};
				}
			}
		}
		
		sec = 0;
		if (!timeLeft[currentPlayer][0] && !timeLeft[currentPlayer][1] || set)
		{
			blink++;
			if (blink == 1000)
			blink = 0;
		} else
		{
			blink = 1;
		}
		if (flashing && !pause)
		{
			if (blink == 0) {
				DDRE = 0x00;
				if (!set)
				{
					_delay_ms(500);
				}
				else
				{
					_delay_ms(1000);
				}
			}else
			{
				DDRE = 0xFF;
			}
		}
		display = timeLeft[currentPlayer][1]/1000;
		if (display == 60)
		{
			displayDigit(0,1,0);
			displayDigit(0,2,0);
		} else
		{
			displayDigit(display%10,1,0);
			if (timeLeft[currentPlayer][0] || (display/10) || set)
			displayDigit(display/10,2,0);
		}
		display = timeLeft[currentPlayer][0];
		if ((display%10) || display/10 || set)
		displayDigit(display%10, 4,1);
		if (display/10 || set)
		displayDigit(display/10,8,0);
		
		if (pause)
		{
			if (timeLeft[currentPlayer][1]-sec < 0)
			{
				if (timeLeft[currentPlayer][0])
				{
					timeLeft[currentPlayer][0] -= 1;
					timeLeft[currentPlayer][1] = 60000;
				} else
				{
					pause = 0;
					flashing = 1;
				}
				
			} else
			timeLeft[currentPlayer][1] -= sec;
		}
	}
}

