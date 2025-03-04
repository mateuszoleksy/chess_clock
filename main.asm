.include"m328pbdef.inc"

	prime: .DB 0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B


;TODO endtime
.DSEG
delay1: .BYTE 1 ; zmienna w sram na delay
player: .BYTE 1 ; zmienna przechowujaca numer gracza
endtime: .BYTE 1 ; zmienna przechowujaca informacjê ze o tym ze jeden czas wynosi 0
status: .BYTE 1 ; normal 1 /set 2 /pause 3
minutes: .BYTE 1 ; minuty
seconds: .BYTE 1 ; sekundy


.CSEG
.org 0x50
jmp begin
.org PCINT1addr
rjmp keypad_ISR ;Keypad External Interrupt Request

;-----------------------------------------------------------------------------------
;inicjalizacja 
begin:
; ******** na start daje mu miodu i niech bedzie to 5 minut ******** mo¿e to byæ ustawienie domyœlne
; S¹ to dwa liczniki, które wraz z uruchomieniem zegara dekrementuj¹ siê albo jeden albo drugi
; -------- liczba sekund nr 1 --------
ldi r16, low(300)
ldi r17, high(300)
add XL, r16
adc XH, r17

; -------- liczba sekund nr 2 --------
ldi r16, low(300)
ldi r17, high(300)
add YL, r16
adc YH, r17

; ******** ustawiamy domyœlny delay = 1 sekundê ********
ldi r23, 1
sts delay1, r23
; ******** 
ldi r23, 3
sts status, r23
; ******** ustawiamy domyœlnie endtime = 0 ********
ldi r23, 0
sts endtime, r23
; ******** ustawiamy domyœlnie player = 1 ********
ldi r23, 1
sts player, r23

; ******** wyswietlacz ********
ldi r17, 0xff
out ddrd, r17
out ddre, r17


;
;
;
; ******** obs³uga klawiatury ********

; Set Stack Pointer to top of RAM
ldi r16, high(RAMEND)
out SPH, r16
ldi r16, low(RAMEND)
out SPL, r16
;SET UP 6 LEDS
;SET UP KEYPAD, 2 rows x 4 cols
;Set rows as inputs and columns as outputs
ldi r20, 0x0f
out ddrc, r20
;Set rows to high (pull ups) and columns to low
ldi r20, 0xf0
out portc, r20
;Select rows as interrupt triggers
ldi r20, (1<<pcint13)|(1<<pcint12)
sts pcmsk1, r20
;Enable pcint1
ldi r20, (1<<pcie1)
sts pcicr, r20
;Reset register for output
ldi r18, 0x00
;Global Enable Interrupt
Sei


;-----------------------------------------------------------------------------------
;Petla main loop 
.org 0x100

loop:


call math
call display

rjmp loop

;-----------------------------------------------------------------------------------
;Keypad Interrupt Service Routine
keypad_ISR:
;Disable pcint interrupts on portc
ldi r20, 0
sts pcmsk1, r20
;Set rows as outputs and columns as inputs
ldi r20, 0xf0
out ddrc, r20
;Set columns to high (pull ups) and rows to low
ldi r20, 0x0f
out portc, r20
;Read Port C. Columns code in low nibble
in r16, pinc
;Store columns code to r18 on low nibble
mov r18, r16
andi r18, 0x0f
;Set rows as inputs and columns as outputs
ldi r20, 0x0f
out ddrc, r20
;Set rows to high (pull ups) and columns to low
ldi r20, 0x30
out portc, r20
;Read Port C. Rows code in high nibble
in r16, pinc
;Merge with previous read
andi r16, 0x30
add r18, r16
;Restore rows as pcint interrupt triggers
ldi r20, (1<<pcint13)|(1<<pcint12)
sts pcmsk1, r20
reti
;-----------------------------------------------------------------------------------


;funkcja która ustawia pauze
start:
push r20
push r17
push r19
push r21
push r22
push r23
push r24

cp r19, r18
andi r19, 0b00010010
cpi r19, 0b00010010
breq pause
rjmp start_return

pause:
lds r20, status
cpi r20, 1
breq start_zero
cpi r20, 3
breq start_one

start_one:
ldi r20, 1
sts status, r20
rjmp start_return

start_zero:
ldi r20, 3
sts status, r20

start_return:
pop r24
pop r23
pop r22
pop r21
pop r19
pop r17
pop r20

ret 

; funkcja która wybiera gracza
switch:
push r20
push r17
push r19
push r21
push r22
push r23
push r24

cp r19, r18
andi r19, 0b00010001
cpi r19, 0b00010001
breq s_player1

cp r19, r18
andi r19, 0b00100001
cpi r19, 0b00100001
breq s_player2

s_player1:
ldi r20, 1
sts player, r20
rjmp s_return

s_player2:
ldi r20, 2
sts player, r20
rjmp s_return

s_return:
pop r24
pop r23
pop r22
pop r21
pop r19
pop r17
pop r20

ret

; funkcja która wprowadza dane z klawiatury tj, ktora ustawia wielkosc minmut i sekund
keypad:
push r20
push r17
push r19
push r21
push r22
push r23
push r24

lds r21, player
clc
cp r19, r18
andi r19, 0b00101000
cpi r19, 0b00101000
breq k_minutes

cp r19, r18
andi r19, 0b00100100
cpi r19, 0b00100100
breq k_seconds

cp r19, r18
andi r19, 0b00100010
cpi r19, 0b00100010
breq k_second


k_minutes:
cpi r21, 1
ldi r20, 60
brne k_player_m

adc XL, r20
rjmp k_return

k_player_m:
adc YL, r20
rjmp k_return

k_seconds:
cpi r21, 1
ldi r20, 10
brne k_player_s

adc XL, r20
rjmp k_return

k_player_s:
adc YL, r20
rjmp k_return

k_second:
cpi r21, 1
ldi r20, 1
brne k_player_s2

adc XL, r20
rjmp k_return

k_player_s2:
adc YL, r20
rjmp k_return

k_return:
pop r24
pop r23
pop r22
pop r21
pop r19
pop r18
pop r17
pop r20

ret


;funkcja która nam liczy pozostaly czas jednego z dwoch graczy
timer:
push r20
push r17
push r18
push r19
push r21
push r22
push r23
push r24

lds r20, player
ldi r19, 1

cpi r21, 3
breq t_return

clc
cpi r20, 1
breq t_player1
cpi r20, 2
breq t_player2

t_player1: 
sbc XL, r19
call delay
rjmp t_prereturn 

t_player2:
sbc YL, r19
call delay
rjmp t_prereturn

t_prereturn: 
brcc t_return
ldi r19, 1
sts endtime, r19

t_return: 
pop r24
pop r23
pop r22
pop r21
pop r19
pop r18
pop r17
pop r20

ret

;funkcja która przelicza nam minuty na sekundy i dodaje liczbe sekund
math:
push r20
push r17
push r19
push r21
push r22
push r23
push r24

lds r21, status
lds r20, player
;ile minut i jaki gracz?
clc
ldi r22, 0
cpi r20, 1
breq m_player1
cpi r20, 2
breq m_player2

m_player1:
;zmienna 22 przechowuje liczbe minut a zmienna XL liczbe sekund po tym
ldi r19, 60
cp ZL, XL
cp ZH, XH
sbc XL, r19
brcs m_minute1

inc r22
rjmp m_player1

m_minute1:
clc
adc XL, r19
sts minutes, r22
 

ldi r22, 0
m_seconds1:
ldi r19, 1
sbc XL, r19
brcs m_second1

inc r22
rjmp m_seconds1

m_second1:
clc
adc XL, r19
sts seconds, r22
cp XL, ZL
cp XH, ZH

rjmp m_return

m_player2:
ldi r19, 60
cp ZL, YL
cp ZH, YH
sbc YL, r19
brcs m_minute2

inc r22
rjmp m_player2

m_minute2:
clc
adc YL, r19
sts minutes, r22
 

ldi r22, 0
m_seconds2:
ldi r19, 1
sbc YL, r19
brcs m_second2

inc r22
rjmp m_seconds2

m_second2:
clc
adc XL, r19
sts seconds, r22
cp YL, ZL
cp YH, ZH

rjmp m_return

m_return:
pop r24
pop r23
pop r22
pop r21
pop r19
pop r17
pop r20

ret

;funkcja która wyœwietla dwa tryby pause/normal
display:
push r20
push r17
push r19
push r21
push r22
push r23
push r24

lds r20, status
cpi r20, 1
breq d_normal
cpi r20, 3
breq d_pause

d_normal:


rjmp d_return

d_pause:


rjmp d_return



d_return:
			; najpierw trzeba wyodrebnic ile mamy sekund 10/1 
			lds r16, seconds
			ldi r19, 10
			ldi r21, 0
			d_seconds:
			clc
			sbc r16, r19
			brcs d_second

			inc r21
			rjmp d_seconds

			d_second:
			clc 
			adc r16, r19


			;numer 1
			ldi r19, 0x01
			com r19
			out porte, r19
			mov r20, r16
			andi r20, 0x0f
			ldi ZL, low(2*prime)
			add ZL, r20
			lpm r20, z
			com r20
			out portd, r20

			call d_delay

			;numer 2
			mov r20, r21
			andi r20, 0x0f
			ldi r19, 0x02
			com r19
			out porte, r19
			ldi ZL, low(2*prime)
			add ZL, r20
			lpm r20, z
			com r20
			out portd, r20

			call d_delay

			; najpierw trzeba wyodrebnic ile mamy minut 10/1 
			lds r17, minutes
			ldi r19, 10
			ldi r21, 0
			d_minutes:
			clc
			sbc r17, r19
			brcs d_minute

			inc r21
			rjmp d_minutes

			d_minute:
			clc 
			adc r17, r19

			;numer 3
			numer3:
			mov r20, r17
			andi r20, 0x0f
			ldi r19, 0x04
			com r19
			out porte, r19
			ldi ZL, low(2*prime)
			add ZL, r20
			lpm r20, z
			com r20
			out portd, r20

			call d_delay

			;numer 4
			numer4:
			mov r20, r21
			andi r20, 0x0f
			ldi r19, 0x08
			com r19
			out porte, r19
			ldi ZL, low(2*prime)
			add ZL, r20
			lpm r20, z
			com r20
			out portd, r20

			call d_delay

pop r24
pop r23
pop r22
pop r21
pop r19
pop r17
pop r20
ret


;delay no tak!
delay:

push r20
push r17
push r18
push r19
push r21
push r22
push r23
push r24
ldi		r20, 1
ldi		r17, 200
ldi		r18, 200
ldi		r19, 100

lds r20, delay1
loop_seconds: 
	loop1: ldi r17, 200
		loop3:
			dec r17
			brne loop3
		dec r18
		brne loop1
	ldi r18, 200
	dec r19
	brne loop1
ldi r19, 100
dec r20
brne loop_seconds

pop r24
pop r23
pop r22
pop r21
pop r19
pop r18
pop r17
pop r20
ret


;display delay mniejszy
d_delay:
push r20
push r17
push r18
push r19
push r21
push r22
push r23
push r24
ldi		r20, 1
ldi		r17, 10
ldi		r18, 10
ldi		r19, 100

lds r20, delay1
d_loop_seconds: 
	d_loop1: ldi r17, 10
		d_loop3:
			dec r17
			brne d_loop3
		dec r18
		brne d_loop1
	ldi r18, 10
	dec r19
	brne d_loop1
ldi r19, 100
dec r20
brne d_loop_seconds

pop r24
pop r23
pop r22
pop r21
pop r19
pop r18
pop r17
pop r20
ret

stop: rjmp stop
