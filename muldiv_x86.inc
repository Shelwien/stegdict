
#pragma message "Using x86 workaround for int64*int64/int64 operations"

qword multdiv( qword a, qword b, qword c ) {
  const qword base = 1ULL<<32;
  const qword maxdiv = (base-1)*base + (base-1);

  // First get the easy thing
  qword res = (a/c) * b + (a%c) * (b/c);
  a %= c;
  b %= c;
  if( (a==0) || (b==0) ) return res;

  // Is it easy to compute what remain to be added?
  if( c<base ) return res + (a*b/c);

  // Now 0 < a < c, 0 < b < c, c >= 1ULL
  // Normalize
  qword norm = maxdiv/c;
  c *= norm;
  a *= norm;

  // split into 2 digits
  qword ah = a / base, al = a % base;
  qword bh = b / base, bl = b % base;
  qword ch = c / base, cl = c % base;

  // compute the product
  qword p0 = al*bl;
  qword p1 = p0 / base + al*bh;
  p0 %= base;
  qword p2 = p1 / base + ah*bh;
  p1 = (p1 % base) + ah * bl;
  p2 += p1 / base;
  p1 %= base;
  // p2 holds 2 digits, p1 and p0 one

  // first digit is easy, not null only in case of overflow
  qword q2 = p2 / c;
  p2 = p2 % c;

  // second digit, estimate
  qword q1 = p2 / ch;
  // and now adjust
  qword rhat = p2 % ch;

  // the loop can be unrolled, it will be executed at most twice for
  // even bases -- three times for odd one -- due to the normalisation above
  while( (q1>=base) || ((rhat<base) && (q1*cl>rhat*base+p1)) ) { q1--; rhat+=ch; }

  // subtract 
  p1 = ((p2 % base) * base + p1) - q1 * cl;
  p2 = (p2 / base * base + p1 / base) - q1 * ch;
  p1 = p1 % base + (p2 % base) * base;

  // now p1 hold 2 digits, p0 one and p2 is to be ignored
  qword q0 = p1 / ch;
  rhat = p1 % ch;
  while( (q0>=base) || ((rhat<base) && (q0*cl>rhat*base+p0)) ) { q0--; rhat+=ch; }

  // we don't need to do the subtraction (needed only to get the remainder,
  // in which case we have to divide it by norm)
  return res + q0 + q1 * base; // + q2 *base*base
}

//typedef unsigned __int128 hword;
//struct Hword { qword L; qword H; };

void ADC( qword& Carry, qword& a, qword b ) {
  qword old = a; 
  a += b; if( a<old ) ++Carry;
}

qword mulQQ( qword a, qword b ) {
  //Hword r;
  qword CMid=0,CHai=0;
  qword ah = a>>32, al = uint(a);
  qword bh = b>>32, bl = uint(b);

  qword ah_bh = ah*bh; // <<64
  qword al_bl = al*bl; // <<0
  qword al_bh = al*bh; // <<32
  qword ah_bl = ah*bl; // <<32

  ADC( CMid, al_bh, ah_bl ); // al*bh+ah*bl with carry in CMid, still <<32

  ah_bl=al_bl; ah_bl>>=32; // high dword of al*bl
  //al_bl = uint(al_bl); // high dword already went to ah_bl

  ADC( CMid, al_bh, ah_bl ); // al*bh+ah*bl+(al*bl>>32) with carry in CMid, still <<32

  CMid<<=32; // because al*bh are shifted like that
  CMid |= al_bh>>32; // move high dword there

  //al_bh = uint(al_bh);
  //al_bh<<=32;
  //al_bh |= al_bl; // two low dwords

  ADC( CHai, ah_bh, CMid ); // ah*bh + ((al*bh+ah*bl+(al*bl>>32))>>32) in ah_bh

  //r.L = al_bh;
  //r.H = ah_bh;
  //return r;
  return ah_bh;
}

// (hword(a)*b)/(hword(range)+1); 
qword muldivR( qword a, qword b, qword range ) { 
  qword r;
  range++;
  if( range==0 ) {
    //Hword a1 = mulQQ( a, b );
    //r = a1.H;
    r = mulQQ(a,b); // >>64
  } else { 
    r = multdiv( a, b, range );
  }
  return r;
}

// (hword(a)*(hword(range)+1))/c; 
qword mulRdiv( qword a, qword c, qword range ) { 
  qword r;
  range++;

  if( a==c ) r=range; else

  if( range==0 ) {
    // a*(b-c)/c+a = a*b/c
    r = multdiv( a, -c, c ) + a;
  } else {
    r = multdiv( a, range, c );
  }
  return r;
}
