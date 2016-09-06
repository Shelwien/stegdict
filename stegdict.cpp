
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

uint f_DEC;
uint recsize;

NOINLINE
int Process( char* fnam, char* gnam, FILE* h, uint f_cont=0 ) {
  FILE* f = fopen(fnam,"rb"); if( f==0 ) return 2; // input dict
  FILE* g = fopen(gnam,"wb"); if( g==0 ) return 3; // output dict

  uint f_len = flen(f);
  byte* p = new byte[f_len]; if( p==0 ) return 5;
  f_len = fread( p, 1,f_len, f );
  fclose(f);

//printf( "f_len=%i f_cont=%i\n", f_len, f_cont );

  if( f_DEC==0 ) {
    C.Init(p,f_len,recsize);
    if( f_cont ) C.f_cont=f_cont;
    C.processfile( h, g );
  } else {
    D.Init(p,f_len,recsize); D.g=g; 
    if( f_cont ) D.f_cont=f_cont;
    D.processfile( 0, h );
  }

  fclose(g);
  delete p;

  return 0;
}

int main( int argc, char** argv ) {

  if( argc<4 ) return 1;

  recsize=0;
  f_DEC = (argv[1][0]=='d'); 
  if( argv[1][0] ) recsize=atoi(&argv[1][1]);
  FILE* h; FILE* lst=0;

  int r = 0;
  char* fnam = argv[2];
  if( fnam[0]=='@' ) {

    h = fopen(argv[3],f_DEC?"wb":"rb"); if( h==0 ) return 4; // payload

    lst = fopen(fnam+1,"rb"); if( lst==0 ) return 6; // list of inp/out dict files
    uint lstlen = flen(lst);
    char* plst = new char[lstlen+1];
    fread( plst, 1,lstlen, lst );
    fclose(lst);

    char** lstnam = new char*[lstlen]; if( lstnam==0 ) return 7;
    uint*  namlin = new uint[lstlen];  if( namlin==0 ) return 8;

    char* p=0;
    uint i,j,k,x,y, nlin=0, f_quote=0, f_file=1, f_inc=0;
    for( i=0,j=0,k=0; i<lstlen; i++ ) {
      if( plst[i]==13 ) continue;
      if( plst[i]==10 ) { nlin+=f_inc; f_quote=0; f_file=1; f_inc=0; plst[j++]=10; continue; } 
      if( plst[i]=='"' ) { f_quote^=1; continue; }
      if( ((plst[i]==' ')||(plst[i]=='\t')) && (f_quote==0) ) { 
        if( f_file==0 ) plst[j++]=' ';
        f_file=1; continue; 
      };
      if( f_file ) {
        if( p ) p[1]=0; // terminate the previous name
        lstnam[k]=&plst[j]; namlin[k]=nlin;
        k++; f_file=0; f_inc=1;
      }
      p=&plst[j],plst[j++]=plst[i];
    } lstlen=j;
    if( p ) p[1]=0; // terminate the previous name

    //printf( "nlin=%i k=%i\n", nlin, k );
    //for( i=0; i<k; i++ ) printf( "<%s>:%i\n", lstnam[i], namlin[i] );

    char** inam = new char*[nlin]; if( inam==0 ) return 9;
    char** onam = new char*[nlin]; if( onam==0 ) return 10;
    uint*  rsiz = new uint [nlin]; if( rsiz==0 ) return 11;

    // process names, sort into places
    for( i=0,j=0,x=-1,y=-1; i<k; i++ ) {
      char* p = lstnam[i]; uint n = namlin[i];
      if( n!=x ) x=n,j=0,++y,inam[y]=onam[y]=(char*)"",rsiz[y]=0; else ++j; // first name in line... or not
      if( j==1 ) onam[y]=p; else 
      if( j==0 ) { 
        if( p[0]=='#' ) { // syntax like this: #16:filename.ext
          ++p; // skip #
          rsiz[y]=atoi(p); 
          while( (p[0]>='0') && (p[0]<='9') && (p<&plst[lstlen]) ) p++; 
          ++p; // skip : 
        }
        inam[y]=p;
      }
    } if( nlin!=x ) ++y;

    delete lstnam; delete namlin;

    for( i=0; i<y; i++ ) {
      printf( "%c%i <%s> <%s>: ", "cd"[f_DEC], rsiz[i], inam[i], onam[i] );
      recsize = rsiz[i];
      r = Process( inam[i], onam[i], h, (i!=0)|(2*(i!=y-1)) ); 
      if( r ) break;
    }

  } else {
    if( argc<5 ) return 1;
    // single file
    h = fopen(argv[4],f_DEC?"wb":"rb"); if( h==0 ) return 4; // payload
    r = Process( argv[2], argv[3], h );
  }

  return r;
}

