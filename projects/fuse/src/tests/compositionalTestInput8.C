#pragma fuse lc(ccs, cp)
void CompDebugAssert(bool);

int i=1, k=3;
int j=2;

void foo() {
  int a=0;
  CompDebugAssert((i+j)==3);
}
