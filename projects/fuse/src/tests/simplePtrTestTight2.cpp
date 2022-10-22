#pragma fuse lc(t(cp, pt))
bool CompDebugAssert(bool expr);

int main()
{
  int *p, x, y, *q;
  x = 5;
  y = 2;
  p = &x;
  q = p;
  CompDebugAssert(*q+1 == 6);
  return *q+1;
}
 
