#pragma fuse lc(ccs, ccs, cp)
void CompDebugAssert(bool);

/*template<typename Type1, typename Type2>
class B {
  public:
  Type1 b1;
  Type2 b2;
};
*/
template<typename Type1, typename Type2>
class C {
  public:
  int c1; 
//  B<Type1, Type2> c2[10][10];
  int c3; 
};

void baz1() {
  C<char, double> x;
  x.c1 = 1;
  CompDebugAssert(x.c1==1);
/*  C<char, double> x;
  x.c2[0][1].b1=1;
  x.c2[1][2].b2=2.0;

  CompDebugAssert(x.c2[0][1].b1==1);
  CompDebugAssert(x.c2[1][2].b2==2.0);

  CompDebugAssert(x.c2[x.c2[0][1].b1]
                      [(int)(x.c2[1][2].b2)].b2==2.0);*/
}

// Almost identical to baz1(), with a few changes to the types to force another version of the
// above templated classes to be generated. This tests our ability disambiguate the different copies.
void baz2() {
  C<char, double> x;
  x.c1 = 1;
  CompDebugAssert(x.c1==1);
/*  C<char, double> x;
  x.c2[0][1].b1=1;
  x.c2[1][2].b2=2.0;

  CompDebugAssert(x.c2[0][1].b1==1);
  CompDebugAssert(x.c2[1][2].b2==2.0);

  CompDebugAssert(x.c2[x.c2[0][1].b1]
                      [(int)(x.c2[1][2].b2)].b2==2.0);*/
}

