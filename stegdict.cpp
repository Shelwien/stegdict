
#define INC_FLEN
#define INC_LOG2I
#include "common.inc"
#include "coro2b.inc"
//#include "coro2_pin.inc"

#include "sh_v1m128.inc"

#include "idx_tree.inc"
#include "sh_qsort.inc"

#include "model.inc"

#include "coro_fp2.inc"

static union {
ALIGN(4096) CoroFileProc< Model<0> > C;
ALIGN(4096) CoroFileProc< Model<1> > D;
};

enum { inpbufsize = 1<<16, outbufsize = 1<<16 };
ALIGN(4096) byte inpbuf[inpbufsize];
ALIGN(4096) byte outbuf[outbufsize];

int main( int argc, char** argv ) {

  if( argc<5 ) return 1;

  uint f_DEC = (argv[1][0]=='d'); 
  FILE* f = fopen(argv[2],"rb"); if( f==0 ) return 2; // input dict
  FILE* g = fopen(argv[3],"wb"); if( g==0 ) return 3; // output dict
  FILE* h = fopen(argv[4],f_DEC?"wb":"rb"); if( h==0 ) return 4; // payload

  uint f_len = flen(f);
  byte* p = new byte[f_len]; if( p==0 ) return 5;
  f_len = fread( p, 1,f_len, f );

  if( f_DEC==0 ) {
    C.Init(p,f_len); C.processfile( h, g );
  } else {
    D.Init(p,f_len); D.g=g;
    D.processfile( 0, h );
  }

  fclose(f);
  fclose(g);

  return 0;
}

