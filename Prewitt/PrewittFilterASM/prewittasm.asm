.data
padding DB 0  ;takie same dla kazdego watku
rozmiar QWORD 0  ;takie same dla kazdego watku

.code
;width -> rcx
;height -> rdx
;startrow -> r8
;endrow -> r9
;sourceImage -> QWORD PTR[rsp+40]
;targetImage -> QWORD PTR[rsp+48]
PrewittThreadASM proc
 movq xmm0, QWORD PTR[rsp+40]				;adres tablicy z bitmapa
 movq xmm1, QWORD PTR[rsp+48]				;adres tablicy na przeksztalcona bitmape

;padding = ((4 - (width * 3) % 4) % 4);
 xor		rax, rax						;0 -> rax
 mov		rax, 3							;3 -> rax
 push		rdx								;rdx(height) - >stos
 mul		rcx								;rax(3) * rcx(width) -> rdx:rax
 push		rbx								;rbx -> stos
 mov		rbx, 4							;4-> rbx
 div		rbx								;rax/rbx wynik dzielenia -> rax		reszta dzielenia ->	rdx
 mov		rax, 4							;4 -> rax
 sub		rax, rdx						;rax - rdx -> rax
 div		rbx								;rax/rbx wynik dzielenia -> rax		reszta dzielenia ->	rdx
 mov		padding, dl						;padding
 pop		rbx								;stos -> rbx
 pop		rdx								;stos -> rdx(height)


 ;int rozmiar = (width * 3 + padding) * (height);
 push		rbx								;rbx -> stos
 push		rdx								;rdx(height) - >stos
 mov		rax, 3							;3 -> rax
 mul		rcx								;rax(3) * rcx(width) -> rdx:rax
 xor		rbx, rbx						;0 -> rbx
 mov		bl, padding						;padding -> rbx
 add		rax, rbx						;rax + padding -> rax
 pop		rdx								;stos -> rdx(height)
 push		rcx								;rcx(width) - >stos
 mov		rcx, rdx						;rdx(height) -> rcx
 push		rdx								;rdx(height) - >stos
 mul		rcx								;rax * rcx(height) -> rdx:rax
 mov		rozmiar, rax 
 pop		rdx
 pop		rcx
 pop		rbx

 ;zet = startrow * (width * 3 + padding);
 push		rbx								;rbx -> stos
 push		rdx								;rdx(height) - >stos
 mov		rax, 3							;3 -> rax
 mul		rcx								;rax(3) * rcx(width) -> rdx:rax
 xor		rbx, rbx						;0 -> rbx
 mov		bl, padding						;padding -> rbx
 add		rax, rbx						;rax + padding -> rax
 mul		r8								;rax * r8(startrow) -> rdx:rax
 pop		rdx
 pop		rbx


; if ((z >= ((width * 3 + padding) * height))) 
;			return;\
 push		r15
 push		r14
 push		r13	
 push		r12
 push		rbx								;rbx -> stos
 push		rdx								;rdx(height) - >stos
 mov		r13, rax						;r13 przechowuje indeks do operacji na tablicach
 mov		rax, rozmiar
 cmp		r13, rax	
 jae		finish

 ;for (int j = startrow; j < endrow; j++)
  ;for (int i = 0; i < width; i++)
 mov		r10, r8							;r8(startrow) -> r10 (j)
 xor		rbx, rbx
startJLoop:
 xor		rbx, rbx
 cmp		r10, r9							;czy r10(j) >= r9(endrow)
 jge		endJLoop						;jesli tak to skocz do konca jpetli
startILoop:
 cmp		rbx, rcx						;czy rbx (i) >= rcx(width)
 jge		endILoop						;jesli tak to skocz do konca ipetli


 ;if ((i == 0) || (i == width - 1) || (j == 0) || (j == height - 1))
 mov		rax, 1	
 xorps		xmm2, xmm2
 pinsrd		xmm2, eax, 0					;valx = 1
 pinsrd		xmm2, eax, 2					;valy = 1
 pinsrd		xmm2, eax, 1					;valx = 1
 pinsrd		xmm2, eax, 3					;valy = 1
 test		rbx, rbx						;czy rbx(i) == 0
 jz			gradientBorder					;jesli tak to skok do gradient
 mov		r11, rcx						;rcx(width) -> r11
 dec		r11								;r11(width)--
 cmp		rbx, r11						;jesli rbx(i)== r11(width-1)
 je			gradientBorder					;to skocz do gradient
 test		r10, r10						;czy r10(j) == 0
 jz			gradientBorder					;jesli tak to skok do gradient
 pop		rdx
 mov		r11, rdx						;rdx(height) -> r11
 push		rdx
 dec		r11								;r11(height)--
 cmp		r10, r11						;jesli rbx(i)== r11(height-1)
 je			gradientBorder					;to skocz do gradient


 xor		rax, rax		
 pinsrd		xmm2, eax, 0					;valx1 = 0
 pinsrd		xmm2, eax, 2					;valy1 = 0
 pinsrd		xmm2, eax, 1					;valx2 = 0
 pinsrd		xmm2, eax, 3					;valy2 = 0 


 ;width + width + width + padding + z + 3 >= rozmiar to return 
 xor		rax, rax
 xor		r12, r12
 mov		r12b, padding					;padding -> r12
 mov		al, r12b						;r12(padding) -> rax
 add		rax, r13						;rax(padding) + r13(zet)
 add		rax, 3							;rax(padding + zet) + 3
 add		rax, rcx						;rax(padding + zet + 3) + rcx(width)
 add		rax, rcx						;rax(padding + zet + 3 + width) + rcx(width)
 add		rax, rcx						;rax(padding + zet + 3 + width + width) + rcx(width)
 xor		r15, r15
 mov		r15, rozmiar
 cmp		rax, r15						;jesli rax(index) >= r15(rozmiar)
 jnb		finish							;to skocz do finish


 ;           3        2        1       0 
 ;xmm4 = 00000000-00000000-00000000-00000000 
 ;         valy2    valy1   valx2     valx1              

 ;xValue = xValue + tablica[paddingt + z - 3 - width - width - width];
 ;yValue = yValue + tablica[padding + z - 3 - width - width - width];
 xor		rax, rax
 xor		r12, r12
 mov		r12b, padding					;padding -> r12
 mov		al, r12b						;r12(padding) -> rax
 add		rax, r13						;rax(padding) + r13(zet)
 sub		rax, 3							;rax(padding + zet) - 3
 sub		rax, rcx						;rax(padding + zet - 3) - rcx(width)
 sub		rax, rcx						;rax(padding + zet - 3 - width) - rcx(width)
 sub		rax, rcx						;rax(padding + zet - 3 - width - width) - rcx(width)
 xor		r15, r15
 xorps		xmm4, xmm4
 xor		r14, r14
 movq		r15, xmm0						;xmm0(sourceImage[0]) -> r15
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(00-00-00-valx1)
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14	
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-valx2-valx1)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-valx2-valx1)
 addps		xmm2, xmm4						;xmm2(00-00-00-00) + xmm4(valy2-valy1-valx2-valx1)


; yValue = yValue + tablica[padding + z - width - width - width];
;xValue = xValue + tablica[width + width + width + padding + z - 3]; 
 add		rax, 3							;rax(padding + zet - 3 - width - width - width) + 3
 xorps		xmm4, xmm4
 xor		r14, r14
 movq		r15, xmm0						;xmm0(sourceImage[0]) -> r15
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-00-00)
 sub		rax, 3							;rax(padding + zet - width - width - width) - 3
 add		rax, rcx
 add		rax, rcx
 add		rax, rcx
 add		rax, rcx
 add		rax, rcx
 add		rax, rcx						;rax(padding + zet - 3 + width + width) + rcx(width)
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-valx1)
 addps		xmm2, xmm4						;xmm2(xx-xx-xx-xx) + xmm4(valy2-valy1-valx2-valx1)


 ;yValue = yValue + tablica[padding + z + 3 - width - width - width];
 ;xValue = xValue + tablica[z - 3];
 add		rax, 6
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx						;rax(padding + z + 3 - width - width) - rcx(width)
 xorps		xmm4, xmm4
 xor		r14, r14
 movq		r15, xmm0						;xmm0(sourceImage[0]) -> r15
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-00-00)
 xor		rax, rax
 mov		rax, r13
 sub		rax, 3							;rax(z) - 3
 mov		r14b, byte PTR[r15 + rax]		;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-valx1)
 addps		xmm2, xmm4						;xmm2(xx-xx-xx-xx) + xmm4(valy2-valy1-valx2-valx1)


 ;xValue = xValue - tablica[z + 3];
; yValue = yValue - tablica[width + width + width + padding + z];
 add		rax, 6
 xorps		xmm4, xmm4
 xor		r14, r14
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(00-00-00-valx1)
 sub		rax, 3
 add		rax, r12
 add		rax, rcx
 add		rax, rcx
 add		rax, rcx
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-valx2-valx1)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-valx2-valx1)
 subps		xmm2, xmm4						;xmm2(xx-xx-xx-xx) - xmm4(valy2-valy1-valx2-valx1)

 ;xValue = xValue - tablica[width + width + width + padding + z + 3];
; yValue = yValue - tablica[width + width + width + padding + z + 3];
 add		rax, 3
 xorps		xmm4, xmm4
 xor		r14, r14
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(00-00-00-valx1)
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-valx2-valx1)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-valx2-valx1)
 subps		xmm2, xmm4						;xmm2(xx-xx-xx-xx) - xmm4(valy2-valy1-valx2-valx1)


 ;xValue = xValue - tablica[padding + z + 3 - width - width - width];
; yValue = yValue - tablica[width + width + width + padding + z - 3];
 sub		rax, 6
 xorps		xmm4, xmm4
 xor		r14, r14
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 2					;r14(sourceImage[rax]) -> xmm4(00-00-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 3					;r14(sourceImage[rax]) -> xmm4(00-valy1-00-00)

 add		rax, 6
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 sub		rax, rcx
 mov		r14b, byte PTR[r15 + rax]
 pinsrd		xmm4, r14d, 0					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-00)
 mov		r14b, byte PTR[r15 + rax + 3]	;r15(sourceImage[rax]) -> r14
 pinsrd		xmm4, r14d, 1					;r14(sourceImage[rax]) -> xmm4(valy2-valy1-00-valx1)
 subps		xmm2, xmm4						;xmm2(xx-xx-xx-xx) - xmm4(valy2-valy1-valx2-valx1)
 jmp		gradient

gradientBorder:

;dla obramowania
;color = sqrt(xValue * xValue + yValue * yValue);
 pmulld		xmm2, xmm2						;xmm2 * xmm2
 pextrd		edx, xmm2, 0					;xValue1^2 -> rdx
 pextrd		eax, xmm2, 2					;yValue1^2 -> rax
 add		eax, edx
 xorps		xmm3, xmm3
 cvtsi2sd	xmm3, eax						;int z rax -> float xmm3
 sqrtsd		xmm3, xmm3	
 xor		rax, rax
 cvtsd2si   eax, xmm3						;float z xmm3 -> int rax
 mov		r15, rax

 

;target[z] = color;
;target[z + 1] = color;
;target[z + 2] = color;
 xor		rax, rax
 movq		r14, xmm1						;xmm1(targetImage[0]) -> r14
 xor		rdx, rdx
 mov		rdx, r13						;r13(z) -> rdx
 mov		byte PTR[r14 + rdx], r15b		;r15 - > targetImage[rdx]
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		r13, rdx
 inc		rbx								;i++
 jmp		startILoop

 gradient:
 ;color = sqrt(xValue * xValue + yValue * yValue);
 pmulld		xmm2, xmm2						;xmm2 * xmm2
 pextrd		edx, xmm2, 0					;xValue1^2 -> rdx
 pextrd		eax, xmm2, 2					;yValue1^2 -> rax
 add		eax, edx
 xorps		xmm3, xmm3
 cvtsi2sd	xmm3, eax						;int z rax -> float xmm3
 sqrtsd		xmm3, xmm3	
 xor		rax, rax
 cvtsd2si   eax, xmm3						;float z xmm3 -> int rax
 mov		r15, rax

;target[z] = color;
;target[z + 1] = color;
;target[z + 2] = color;
 xor		rax, rax
 movq		r14, xmm1						;xmm1(targetImage[0]) -> r14
 xor		rdx, rdx
 mov		rdx, r13						;r13(z) -> rdx
 mov		byte PTR[r14 + rdx], r15b		;r15 - > targetImage[rdx]
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		r13, rdx
 inc		rbx								;i++
 mov		r11, rcx						;rcx(width) -> r11
 dec		r11								;r11(width)--
 cmp		rbx, r11						;jesli rbx(i)== r11(width-1)
 je			startILoop						;to skocz do startILoop



 xor		rax, rax
 xor		rdx, rdx
 pextrd		edx, xmm2, 1					;xValue2^2 -> rdx
 pextrd		eax, xmm2, 3					;yValue2^2 -> rax
 add		eax, edx
 xorps		xmm3, xmm3
 cvtsi2sd	xmm3, eax						;int z rax -> float xmm3
 sqrtsd		xmm3, xmm3	

;target[z] = color;
;target[z + 1] = color;
;target[z + 2] = color;
 xor		rax, rax
 cvtsd2si   eax, xmm3						;float z xmm3 -> int rax
 mov		r15, rax
 xor		rax, rax
 movq		r14, xmm1						;xmm1(targetImage[0]) -> r14
 xor		rdx, rdx
 mov		rdx, r13						;r13(z) -> rdx
 mov		byte PTR[r14 + rdx], r15b		;r15 - > targetImage[rdx]
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		byte PTR[r14 + rdx], r15b 
 inc		rdx
 mov		r13, rdx
 inc		rbx								 ;i++
 jmp		startILoop

endILoop:
;z = z + padding;
xor			rdx, rdx
mov			rdx, r13						;r13(z) -> rdx
xor			rax, rax
mov			al, padding						;padding -> rax
add			edx, eax						;z = z + padding
mov			r13, rdx						;z -> r13
inc			r10								;j++
jmp			startJLoop


endJLoop:

finish:
 pop		rdx
 pop		rbx
 pop		r12
 pop		r13
 pop		r14
 pop		r15

finishtest:
 ret	
PrewittThreadASM endp
end