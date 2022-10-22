#pragma fuse lc(t(cp, pt))
void CompDebugAssert(bool expr);
int main()
{
  int *p, val1, *q, val2 ;
  val1 = 5;
  val2 = 0;
  p = &val1;
  val1 = *p+1;
  p = &val2;
  val2 = *p + 1;
  CompDebugAssert((val1+val2)==7);
  return val2 + 1;
}
 
