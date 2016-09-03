
NOINLINE
void put_struct( void* _p, uint l ) {
  byte* p = (byte*)_p;
  if( getoutlen()>=l ) {
    memcpy((byte*)ptr,p,l);
    ptr += l;
  } else {
    uint i;
    for( i=0; i<l; i++ ) {
      put( p[i] );
    }
  }
}

template< typename T >
void put_struct( T& x ) { return put_struct( (byte*)&x, sizeof(x) ); }
