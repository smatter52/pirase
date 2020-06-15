/* setjmp.s for Rasberry Pi V4B */

.text

/* r0-r3 and ip are tmp and do not need preservation */
.global rase_setjmp
.p2align	2
.type		rase_setjmp, %function
rase_setjmp:
	.fnstart
	mov	ip, r0      /* *jmp_buf */
	/* Save sp and lr */
	str	sp, [ip], #4
	str	lr, [ip], #4
	/* Save registers */
	stmia	ip!, {r4, r5, r6, r7, r8, r9, r10, r11} ;

	/* Store the VFP registers. */
/*	vstmia ip!, {d8-d15}    not this time */

 	mov	r0, #0
	bx lr
	.fnend

.text
.global	rase_longjmp
.p2align	2
.type		rase_longjmp, %function
rase_longjmp:
	.fnstart
	mov	ip,r0    /* *jmp_buf */
	ldr 	sp,[ip],#4
	ldr	r3,[ip],#4
/* restore registers */
	ldmia	ip!, {r4, r5, r6, r7, r8, r9, r10, r11} ;
/* Return the result argument, or 1 if it is zero.  */
	mov	r0, r1
	bne	1f
	mov	r0, #1
1:
	bx	r3
	.fnend

