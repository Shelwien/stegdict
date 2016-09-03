
template < class Model, int inpbufsize=1<<16, int outbufsize = 1<<16 > 
struct CoroFileProc : Model {

  typedef Model Base;
  using Base::f_quit;
  using Base::coro_init;
  using Base::coro_call;
  using Base::addinp;
  using Base::addout;
  using Base::outbeg;
  using Base::outptr;

  ALIGN(4096) byte inpbuf[inpbufsize];
  ALIGN(4096) byte outbuf[outbufsize];

  void processfile( FILE* f, FILE* g ) {
    uint l,r;
    coro_init();
    addout( outbuf, outbufsize );
    while( 1 ) {
      r = coro_call(this); 
      if( r==1 ) {
        l = fread( inpbuf, 1, inpbufsize, f );
        addinp( inpbuf, l ); 
        if( l==0 ) {
          f_quit=1; 
          addinp( (byte*)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8 );
        }
      } else { // r0=quit, r3=error
        l = outptr-outbeg;
        if( l>0 ) fwrite( (byte*)outbeg, 1,l, g ); // flush
        if( r!=2 ) break;
        addout( outbuf, outbufsize );
      } // if
    } // while
  } // func

};
