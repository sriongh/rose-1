#pragma fuse lc(cp, dp, t(pt, cp))

void CompDebugAssert(bool expr);

int main()
{
  int *p, val1,  val2 ;
  val1 = 5;
  val2 = 4;
  if(val1 > 10) {
    p = &val1;
  }
  else {
    p = &val2;
  }
  CompDebugAssert(*p==4);

  return *p + 1;
}
 
