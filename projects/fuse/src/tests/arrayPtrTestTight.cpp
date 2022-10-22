int main()
{
  int arr[4];
  arr[0]=1; arr[1]=2; arr[2]=3; arr[3]=4;
  int val=0;
  for(int i=0; i<4; ++i) {
    val=val+arr[i];
  }
  return val;
}
