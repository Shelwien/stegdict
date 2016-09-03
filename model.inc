
template< int ProcMode >
struct Model : Rangecoder_SH1m128<1-ProcMode> {
  enum{ f_DEC = ProcMode };

  typedef Rangecoder_SH1m128<1-ProcMode> Base;
  using Base::rc_GetFreq;
  using Base::rc_Process;
  using Base::rc_Init;
  using Base::rc_Quit;
  using Base::FFNum;
  using Base::Coroutine::get;
  using Base::Coroutine::put;
  using Base::Coroutine::f_quit;
  using Base::Coroutine::coro_init;

  struct TMP { byte* f_ptr; byte* f_end; uint* word_ofs; uint* idx_map; };
  struct CMP {
    // Returns neg if 1<2, 0 if 1=2, pos if 1>2.
    static int  c( TMP* A, uint x, uint y ) {
      if( x==y ) return 0;
      //pdirdata a = A[x];
      //pdirdata b = A[y];
      //return (a==b)?0:(a<b)?-1:1;
      byte* restrict f_ptr = A->f_ptr;
      byte* restrict f_end = A->f_end;
      uint* restrict word_ofs = A->word_ofs;
      uint* restrict idx_map = A->idx_map;
      byte* restrict a = &f_ptr[ word_ofs[idx_map[x]] ];
      byte* restrict b = &f_ptr[ word_ofs[idx_map[y]] ];
      int dir=1; if( a>b ) SWAP(a,b),dir=-1;
      for( ; (b<f_end) && (a[0]==b[0]); a++,b++ ) if( a[0]==10 ) break; 
      return ((b>=f_end)||(a[0]==b[0]))? 0 : (a[0]<b[0]) ? -dir : dir;
    }
    static void s( TMP* A, uint x, uint y ) {
      SWAP( A->idx_map[x], A->idx_map[y] );
    }
  };

  uint  f_len;
  byte* f_ptr;
  byte* f_end;

  uint Nwords;
  uint* word_ofs;

  uint* idx_map;
  uint* idx_idx; // inverse table

  idx_tree T;

  FILE* g;

  uint Init( byte* _f_ptr, uint _f_len ) {
    uint r=0;

    coro_init();
    f_len = _f_len; f_ptr = _f_ptr; f_end = &f_ptr[f_len];

    uint c,i;
    byte* p;
    byte* q = f_end;
    for( i=1,p=f_ptr; p<q; p++ ) if( c=(p[0]=='\n') ) i++; // count the words
    Nwords = i-c;

    word_ofs = new uint[Nwords+1];
    if( word_ofs==0 ) r=1; else {
      word_ofs[0]=0;
      for( i=1,p=f_ptr; p<q; p++ ) if( p[0]=='\n' ) word_ofs[i++]=p+1-f_ptr; // count the words
//printf( "Nwords=%i\n", Nwords );

      r = T.Init( Nwords );
      if( r ) delete word_ofs;

      if( (r==0) && (f_DEC==1) ) {
        idx_map = new uint[Nwords*2]; if( idx_map==0 ) { delete T.idx; r=1; } else {
          idx_idx = &idx_map[Nwords];
          for( i=0; i<Nwords; i++ ) idx_map[i]=i;
        }
      }
    }

    return r;
  }

  void do_process() {

    uint i;
    uint N = Nwords;
    uint c,d;

//printf( "f_DEC=%i\n", f_DEC ); fflush(stdout);

    rc_Init();

    if( f_DEC==0 ) {

      for( i=0; i<Nwords; i++ ) {
        if( N-i<=1 ) c=0; else {
          c = FFNum>8? 0 : rc_GetFreq(N-i); // c'th word to i'th index
          rc_Process( c, 1, N-i );
        }

        c = T.Lookup(c);
        byte* p = &f_ptr[ word_ofs[c] ];
        for( p=p; p<f_end; p++ ) { put( p[0] ); if( p[0]=='\n' ) break; /*putc(p[0],stdout);*/ }

        T.Remove(c);
      }


    } else {

      printf( "Sorting %i words...", Nwords ); fflush(stdout);
      TMP tmp = { f_ptr,f_end,word_ofs,idx_map };
      sh_qsort<CMP>( &tmp, 0, Nwords-1 );
      printf( "\b\b Done\n" ); fflush(stdout);

      for( i=0; i<Nwords; i++ ) idx_idx[idx_map[i]]=i;

      for( i=0; i<Nwords; i++ ) {
        c = idx_map[i];
        byte* p = &f_ptr[ word_ofs[c] ];
        for( p=p; p<f_end; p++ ) { putc(p[0],g); if( p[0]=='\n' ) break; }

        if( N-i>1 ) {
          c = idx_idx[i];
          uint d = T.Translate(c);
          rc_Process( d, 1, N-i );
          T.Remove( c );
        }
      }

    }

    rc_Quit();

    yield(this,0);
  }

};
