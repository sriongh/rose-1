
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

  return *p + 1;
}
 
