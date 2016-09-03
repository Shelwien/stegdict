
struct idx_tree {

  uint* idx;
  uint idxSize;
  uint N;
  uint L2N;

  uint Init( uint _N ) {
    uint i,r=0;
    N = _N;
    L2N = log2i(N);
    idxSize = ((1<<L2N)+(N>>1));
    idx = new uint[idxSize]; // plain 
    if( idx==0 ) r=1; else for( i=1; i<idxSize; i++ ) idx[i] = 1<<(L2N-log2i(i));
    return r;
  }

  // full index to reduced
  uint Translate( uint i ) {
    uint j,b,x,k;
    for( j=L2N,x=1,k=0; j!=-1; j-- ) {
      b = (i>>j)&1; // index bit
      if( b ) k+=idx[x];
      x += x+b;
    }
    return k;
  }

  // reduced to full index
  uint Lookup( uint i ) {
    uint j,b,x;
    for( j=L2N,x=1; j!=-1; j-- ) {
      b = (i>=idx[x]) ? i-=idx[x],1 : 0;
      x += x+b;
    }
    return x ^ (2<<L2N);
  }

  void Remove( uint i ) {
    uint j,b,x,k;
    for( j=L2N,x=1,k=0; j!=-1; j-- ) {
      b = (i>>j)&1; // index bit
      if( b==0 ) idx[x]--;
      x += x+b;
    }
  }

};
