
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
  using Base::Coroutine::inpptr;
  using Base::Coroutine::inpend;
  using Base::Coroutine::coro_init;

  template< class TMP, int idx_type=0 >
  struct CMP {
    // Returns neg if 1<2, 0 if 1=2, pos if 1>2.
    static int  c( TMP* A, uint x, uint y ) {
      if( x==y ) return 0;
      byte* restrict f_ptr = A->f_ptr;
//      byte* restrict f_end = A->f_end;
      uint* restrict word_ofs = A->word_ofs;
      uint* restrict word_len = A->word_len;
      uint* restrict idx_map = A->idx_map;
      byte* restrict a = &f_ptr[ word_ofs[idx_map[x]] ];
      byte* restrict a_end = a + word_len[idx_map[x]];
      byte* restrict b = &f_ptr[ word_ofs[idx_map[y]] ];
      byte* restrict b_end = b + word_len[idx_map[y]];
      int dir=1; if( a>b ) SWAP(a,b),SWAP(a_end,b_end),dir=-1;
      for( ; (b<b_end) && (a[0]==b[0]); a++,b++ );
      if( (a>=a_end) && (b<b_end) ) return -dir;
      return ((b>=b_end)||(a[0]==b[0]))? (idx_map[x]<idx_map[y]?-1:1) : (a[0]<b[0]) ? -dir : dir;
    }
    static void s( TMP* A, uint x, uint y ) {
      if( idx_type==0 ) SWAP( A->idx_map[x], A->idx_map[y] );
      if( idx_type==1 ) {
        SWAP( A->word_ofs[x], A->word_ofs[y] );
        SWAP( A->word_len[x], A->word_len[y] );
      }
    }
  };

  uint rec_size;

  uint  f_len;
  byte* f_ptr;
  byte* f_end;

  uint Nwords;
  uint* word_ofs;
  uint* word_len;

  uint* idx_map;
  char* f_dup; // dup flags

  idx_tree T;

  FILE* g;

  uint f_cont;
  byte* inpptr_save;
  byte* inpend_save;

  uint Init( byte* _f_ptr, uint _f_len, uint _rec_size=0 ) {
    uint r=0;
    f_cont = 0;

    coro_init();
    f_len = _f_len; f_ptr = _f_ptr; f_end = &f_ptr[f_len];
    rec_size=_rec_size; // 0 means variable, terminated with \n

//printf( "rec_size=%i\n", rec_size );

    uint c,i;
    byte* p;
    byte* q = f_end;

    if( rec_size==0 ) {
      for( i=1,p=f_ptr; p<q; p++ ) if( c=(p[0]=='\n') ) i++; // count the words
      Nwords = i-c;
    } else {
      Nwords = f_len/rec_size + (f_len%rec_size>0);
    }

    word_ofs = new uint[(Nwords+1)*2];

    if( word_ofs==0 ) r=1; else {
      word_len = &word_ofs[Nwords+1];

      word_ofs[0]=0;
      if( rec_size==0 ) {
        for( i=1,p=f_ptr; p<q; p++ ) if( p[0]=='\n' ) word_ofs[i++]=p+1-f_ptr; // count the words
      } else {
        for( i=0,p=f_ptr; p<q; p+=rec_size ) word_ofs[i++]=p-f_ptr; // count the words
      }
      word_ofs[Nwords]=f_len;

      for( i=0; i<Nwords; i++ ) word_len[i]=word_ofs[i+1]-word_ofs[i];

//printf( "Nwords=%i\n", Nwords );

      r = T.Init( Nwords );
      if( r ) delete word_ofs;

      if( (r==0) /*&& (f_DEC==1)*/ ) {
        idx_map = new uint[Nwords*1+(Nwords+sizeof(uint)-1)/sizeof(uint)]; if( idx_map==0 ) { delete T.idx; r=1; } else {
          f_dup = (char*)&idx_map[Nwords*1];
        }
      }

    }

    return r;
  }

  int word_cmp( uint x, uint y ) {
    byte* restrict a = &f_ptr[ word_ofs[idx_map[x]] ];
    byte* restrict b = &f_ptr[ word_ofs[idx_map[y]] ];
    byte* restrict a_end = a + word_len[idx_map[x]];
    byte* restrict b_end = b + word_len[idx_map[y]];
    int dir=1; if( a>b ) SWAP(a,b),SWAP(a_end,b_end),dir=-1;
    for( ; (b<b_end) && (a[0]==b[0]); a++,b++ );
    if( (a>=a_end) && (b<b_end) ) return -dir;
    return ((b>=b_end)||(a[0]==b[0]))? 0 : (a[0]<b[0]) ? -dir : dir;
  }

  void do_process() {

    uint i,j;
    uint N = Nwords;
    uint c,d;
    uint base,rlen;

//printf( "f_DEC=%i\n", f_DEC ); fflush(stdout);

    printf( "Sorting %i words...", Nwords ); fflush(stdout);
    for( i=0; i<Nwords; i++ ) idx_map[i]=i;
    if( f_DEC==0 ) sh_qsort< CMP<Model,1> >( this, 0, Nwords-1 ); // restore word order; only for encoding;
    if( f_DEC==1 ) sh_qsort< CMP<Model,0> >( this, 0, Nwords-1 ); // normal sort, keeping the original permutation info
    printf( "\b\b Done\n" ); fflush(stdout);

    for( i=1; i<Nwords; i++ ) f_dup[i]=(word_cmp(i-1,i)==0); f_dup[0]=0;

    if( (f_cont&1)==0 ) rc_Init(); else {
      inpptr = inpptr_save;
      inpend = inpend_save;
    }

    if( f_DEC==0 ) {

      for( i=0; i<Nwords; i++ ) {

        if( f_dup[i]==0 ) {
          base=0; // reset base for non
          for( j=i+1,rlen=0; j<Nwords; j++ ) if( f_dup[j]==0 ) break; else rlen++; // count following dups
        } else rlen--; // maintain the number of following dups

        if( N-i<=1 ) c=0; else {
          c = FFNum>8? 0 : rc_GetFreq(N-i-base-rlen); // c'th word to i'th index
          rc_Process( c, 1, N-i-base-rlen );
          c += base;
        }
        base = c; // store current pos in case of dups; base itself is deleted, so we can reuse it, just not below it

        c = T.Lookup(c);

        idx_map[c] = i; // i'th word to c'th pos

        T.Remove(c); // c'th pos can't be used anymore
      }

      // write the permutated dictionary
      for( i=0; i<Nwords; i++ ) {
        byte* p = &f_ptr[ word_ofs[idx_map[i]] ];
        byte* p_end = p + word_len[idx_map[i]];
        for( p=p; p<p_end; p++ ) put(p[0]);
      }

    } else {

      for( i=0; i<Nwords; i++ ) {

        if( f_dup[i]==0 ) {
          base=0; // reset base for non
          for( j=i+1,rlen=0; j<Nwords; j++ ) if( f_dup[j]==0 ) break; else rlen++; // count following dups
        } else rlen--; // maintain the number of following dups

        if( N-i>1 ) {
          c = idx_map[i];
          uint d = T.Translate(c);
          rc_Process( d-base, 1, N-i-base-rlen );
          T.Remove( c );
          base = d;
        }

        byte* p = &f_ptr[ word_ofs[idx_map[i]] ];
        byte* p_end = p + word_len[idx_map[i]];
        for( p=p; p<p_end; p++ ) putc(p[0],g);

      }

    }

    if( (f_cont&2)==0 ) rc_Quit();
    inpptr_save = (byte*)inpptr;
    inpend_save = (byte*)inpend;

    yield(this,0);
  }

};
