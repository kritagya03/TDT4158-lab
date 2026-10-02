// palinfinder.s, provided with Lab1 in TDT4258 autumn 2026
.global _start


_start:
	//Initialization
	ldr r0, =input //r0 is the address to the first char of "input"
	mov r2, #0 //r2 is the length of "input"
	mov r1, r0 //r1 is the pointer walking through "input", to find length
	
	bl check_input
	
	cmp r2, #4
	blt is_no_palindrom //it is not a palindrom if length is less than 4
	
	mov r1, r0 //r1 leftmost char address
	add r2, r0, r2
	sub r2, r2, #1 //r2 rightmost char address
	
	bl check_palindrom

	
check_input:
	ldrb r3, [r1] //load the current char of "input"
	cmp r3, #0 //is it the null byte?
	beq return_to_link //yes -> finished counting
	
	add r2, r2, #1 //+1 length
	add r1, r1, #1 //next char
	b check_input
	
	
check_palindrom:
	cmp r1, r2
	bge is_palindrom //is palindom if r1 and r2 meet in the middle
	
	ldrb r3, [r1] //left char
	ldrb r4, [r2] //right char
	
	//ignore space, left char
	cmp r3, #' ' 
	beq skip_left
	
	//ignore space, right char
	cmp r4, #' '
	beq skip_right
	
	//convert to lowercase, left char
	convert_left:
	cmp r3, #'A'
	blt convert_right //if r3 < 'A', don't convert it
	cmp r3, #'Z'
	bgt convert_right //if r3 > 'Z', don't convert it
	add r3, r3, #32
	
	//convert to lowercase, right char
	convert_right:
	cmp r4, #'A'
	blt convert_done //if r4 < 'A', don't convert it
	cmp r4, #'Z'
	bgt convert_done //if r4 < 'A', don't convert it
	add r4, r4, #32
	
	convert_done:
	//wildcards, left char
	cmp r3, #'?'
	beq chars_match
	cmp r3, #'%'
	beq chars_match //if char '?' or '%', instant match
	
	//wildcards, right char
	cmp r4, #'?'
	beq chars_match
	cmp r4, #'%'
	beq chars_match //if char '?' or '%', instant match
	
	//no wildcards, then normal comparison
	cmp r3, r4
	bne is_no_palindrom
	
	chars_match:
	add r1, r1, #1 //next char to the right
	sub r2, r2, #1 //next char to the left
	b check_palindrom


is_palindrom:
	ldr r5, =0xff200000 //load address of LED data register
	mov r6, #0b0000011111 //five rightmost LEDs
	str r6, [r5] //store it in the LED data register address

	ldr r5, =0xff201000 //load address of JTAG UART data register
	ldr r6, =pal_msg //r6 is the address of the to the first char of "pal_msg
	
	print_loop:
		ldrb r7, [r6] //load the current char of "pal_msg"
		cmp r7, #0 //is it the null byte?
		beq _exit //yes -> exit program
		
		str r7, [r5] //send char to JTAG UART
		add r6, r6, #1 //next char
		b print_loop
	
is_no_palindrom:
	ldr r5, =0xff200000 //load address of LED data register
	mov r6, #0b1111100000 //five leftmost LEDs
	str r6, [r5] //store it in the LED data register address
	
	ldr r5, =0xff201000
	ldr r6, =not_pal_msg
	
	no_print_loop:
		ldrb r7, [r6] //load the current char of "not_pal_msg"
		cmp r7, #0 //is it the null byte?
		beq _exit //yes -> exit program
		
		str r7, [r5] //send char to JTAG UART
		add r6, r6, #1 //next char
		b no_print_loop
	

skip_left:
	add r1, r1, #1
	b check_palindrom
	
skip_right:
	sub r2, r2, #1
	b check_palindrom
	
return_to_link:
	bx lr
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
	
	pal_msg: .asciz "Palindrom detected\n"
	not_pal_msg: .asciz "Not a palindrom\n"
.end
