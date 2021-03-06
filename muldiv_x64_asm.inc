
#pragma message "Using x64 gnu inline asm"

qword muldivR( qword a, qword b, qword range ) {
  qword x, r = qword(range)+1;

//printf( "a=%I64X b=%I64X r=%I64X\n", a,b,r ); fflush(stdout);

  __asm__ ("\
  movq %1,%%rax; \
  mulq %2; \
  cmpq $0,%3; \
  jz   1f;  \
  divq %3; \
  xchg %%rax,%%rdx; \
1:movq %%rdx,%0; \
  " : "=rm"(x) : "rm"(a),"rm"(b),"rm"(r) : "rax","rdx"
  );

  return x;
}

qword mulRdiv( qword a, qword c, qword range ) {
  qword x;
//printf( "a=%I64X r=%I64X c=%I64X\n" , a,range,c ); fflush(stdout);

  if( a==c ) x=range+1; else {

  __asm__ ("\
  movq %1,%%rax; \
  mulq %3; \
  addq %1,%%rax; \
  adcq $0,%%rdx; \
  divq %2; \
  movq %%rax,%0; \
  " : "=rm"(x) : "rm"(a),"rm"(c),"rm"(range) : "rax","rdx"
  );

  }

  return x;
}

