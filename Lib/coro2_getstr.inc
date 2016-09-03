
NOINLINE
uint get_struct( void* _p, uint l ) {
  uint r=0;
  byte* p = (byte*)_p;
  if( getinplen()>=l ) {
    memcpy(p,(byte*)ptr,l);
    ptr += l;
  } else {
    uint i,c;
    for( i=0; i<l; i++ ) {
      c=get(); if( c==-1 ) { r=1; break; }
      p[i]=c;
    }
  }
  return r;
}

template< typename T >
uint get_struct( T& x ) { return get_struct( (byte*)&x, sizeof(x) ); }

