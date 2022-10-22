#pragma fuse lc(t(cp, pt))
void CompDebugAssert(bool);

int main()
{
  int *p, x, y;
  x = 5;
  y = 2;
  p = &x;
  CompDebugAssert(*p+1 == 6);
  return *p+1;
}
 
