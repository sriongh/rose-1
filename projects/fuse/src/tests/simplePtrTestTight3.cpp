#pragma fuse lc(t(cp, pt))
void CompDebugAssert(bool expr);

int main()
{
  int *p, val1, *q, val2 ;
  val1 = 5;
  p = &val1;
  val1 = *p+1;
  p = &val2;
  val2 = val1 + 1;
  CompDebugAssert(*p + 1 == 8);
  return *p + 1;
}
 
