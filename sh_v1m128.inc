
#if (defined __GNUC__) || (!defined X64)
  #ifdef X64
  #pragma message "Using __int128 type for int64*int64/int64 operations"
  // if __int128 available, just use it
  typedef unsigned __int128 hword;
  static qword muldivR( qword a, qword b, qword range ) { return (hword(a)*b)/(hword(range)+1); }
  static qword mulRdiv( qword a, qword c, qword range ) { return (hword(a)*(hword(range)+1))/c; }
  #else
  // if there's no __int128 and no x64 asm support, then use this
  #include "muldiv_x86.inc"
  #endif
#else
  #ifdef X64
  // if __int128 isn't available, use x64 asm
//  #include "muldiv_x86.inc"
  #include "muldiv_x64_asm.inc"
  #else
  #endif
#endif

template< int rc_ProcMode >
struct Rangecoder_SH1m128 : Coroutine {

  enum {
    SCALElog = 15,
    SCALE    = 1<<SCALElog
  };

  enum {
    NUM   = 8,
//    sTOP  = 0x0100000000000000ULL,
//    Thres = 0xFF00000000000000ULL,
    MSByte= 24+32
  };

  static const qword sTOP  = 0x0100000000000000ULL;
  static const qword Thres = 0xFF00000000000000ULL;

//  int   rc_ProcMode; // 0=encode, 1=decode;
  qword low;
  qword Carry;
  qword code; 
  qword range;

  uint FFNum;
  uint Cache;


  void rc_Process( qword cumFreq, qword freq, qword totFreq ) {
    qword tmp = mulRdiv(cumFreq,totFreq,range); // + 1;
    if( rc_ProcMode ) code-=tmp; else {
      //lowc+=tmp;
      qword old=low; low+=tmp; if( low<old ) Carry++;
    }
    range = mulRdiv(cumFreq+freq,totFreq,range)-1 - tmp;
    if( rc_ProcMode ) 
      while( range<sTOP ) {
        range = (range<<8)+0xFF; // (range+1)*256-1=range*256+255
        uint c=get(); FFNum += (c==-1);
        (code<<=8)+=byte(c);
      }
    else 
      while( range<sTOP ) range=(range<<8)+0xFF, ShiftLow();
  }


  void rc_Arrange( qword totFreq ) {}

  qword rc_GetFreq( qword totFreq ) {
    qword r = muldivR(code,totFreq,range);
    //printf( "GetFreq: %I64X\n", r );
    return r;
  }


  void rc_BProcess( qword freq, int& bit ) { 
    qword x[] = { 0, freq, SCALE };
    if( rc_ProcMode ) {
      qword count = rc_GetFreq( SCALE );
      bit = (count>=freq);
    }
    rc_Process( x[bit], x[1+bit]-x[bit], SCALE );
  }

  void ShiftLow( void ) {
    if( low<Thres || Carry ) {
      if( Cache!=-1 ) put( Cache+Carry );
      for (;FFNum != 0;FFNum--) put( Carry-1 ); // (Carry-1)&255;
      Cache = low>>MSByte;
      Carry = 0;
    } else FFNum++;
    low<<=8;
  }


  void rcInit( void ) { 
    range = -1;
    low   = 0;
    FFNum = 0;
    Carry = 0;    
    Cache = -1;
  }
  
  void rc_Init() {
    rcInit();
    //rc_ProcMode = _rc_ProcMode;
    if( rc_ProcMode==1 ) {
      for(int _=0; _<NUM; _++) (code<<=8)+=get(); 
    }
  }

  void rc_Quit( void ) {
    if( rc_ProcMode==0 ) {
      uint i, n = NUM;

      // cache .. FF x FFNum .. low
      qword high=low+range,High=(high<low),tmp=high; high++; High|=(high<tmp);
      qword llow=low+range/2; Carry|=(llow<low);
      qword mask,nmask,nlow;

      mask=0xFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFFFFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;
      mask=0xFFFFFFFFFFFFFFFFU; nlow=llow|mask; if( (Carry<High) || (nlow<high) ) llow=nlow,n--;

      low = llow;
      if( low==0xFFFFFFFFFFFFFFFFU ) FFNum=0; // they're also FFs

//printf( "Carry=%I64X\n", Carry );
//printf( "%02X %i %016I64X\n", Cache+Carry, FFNum+Carry, low );
//printf( "%02X %i %016I64X\n", Cache+High, FFNum+High, high );

      // write one byte less if possible, last byte is usually partial anyway
      if( (n==0) ) if( FFNum==0 ) Cache=-1; else FFNum--;
      if( n>0 ) n--;

      if( Cache!=-1 ) put( Cache+Carry );
      for( i=0; i<FFNum; i++ ) put( 0xFF+Carry );
      for( i=0; i<n; i++ ) put( low>>MSByte ), low<<=8;

//      qword old=low; low+=range; if( low<old ) Carry++;
//      for( i=0; i<n+1; i++ ) ShiftLow();
    }
  } // quit

};


