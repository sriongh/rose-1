#pragma fuse lc(vm, cp)
/*#include <stdlib.h>
#include <stdio.h>*/
extern void CompDebugAssertBase(bool expr);
extern void CompShowBase(int v1, int v2);
extern void CompDebugAssertDerived0(bool expr);
extern void CompShowDerived0(int v1, int v2);
extern void CompDebugAssertDerived1(bool expr);
extern void CompShowDerived1(int v1, int v2);
#ifdef Z
extern void CompDebugAssertDerived00(bool expr);
extern void CompShowDerived00(int v1, int v2);
extern void CompDebugAssertDerived01(bool expr);
extern void CompShowDerived01(int v1, int v2);
extern void CompDebugAssertDerived10(bool expr);
extern void CompShowDerived10(int v1, int v2);
extern void CompDebugAssertDerived11(bool expr);
extern void CompShowDerived11(int v1, int v2);
#endif

#define check(MyClass) \
  CompDebugAssert ## MyClass(depth>0 || (ret==correct_result0 || ret==correct_result1 || ret==correct_result2 || ret==correct_result3)); \
  CompShow ## MyClass(depth, ret);

#define fooBody(MyClass, ParentClass, val) \
  virtual int foo(int arg, int correct_result0, int correct_result1, int correct_result2, int correct_result3, int depth) { \
    /*arg = ParentClass::foo(arg, correct_result0, correct_result1, correct_result2, correct_result3, depth+1);*/ \
    int ret = arg+val; \
    check(MyClass); \
    return ret; \
  }

class Base {
  public:
  virtual int foo(int arg, int correct_result0, int correct_result1, int correct_result2, int correct_result3, int depth) {
    //printf("Base::foo(%d)\n", arg);
    int ret=arg+0;
    check(Base);
    return ret=0;
  }
};

class Derived0: public Base {
  public:
  fooBody(Derived0, Base, 0);
};

class Derived1: public Base {
  public:
  fooBody(Derived1, Base, 1);
};

#ifdef Z
class Derived00: public Derived0 {
  public:
  fooBody(Derived00, Derived0, 0);
};

class Derived01: public Derived0 {
  public:
  fooBody(Derived01, Derived0, 1);
};

class Derived10: public Derived1 {
  public:
  fooBody(Derived10, Derived1, 2);
};

class Derived11: public Derived1 {
  public:
  fooBody(Derived11, Derived1, 3);
};
#endif

static void test(Base& b_arg, int correct_result0, int correct_result1, int correct_result2, int correct_result3) {
  int test_param=0;
  test_param = b_arg.foo(test_param, correct_result0, correct_result1, correct_result2, correct_result3, 0);
}

int main() {
  Base b;
  Derived0 d0;
  Derived1 d1;
#ifdef Z
  Derived00 d00;
  Derived01 d01;
  Derived10 d10;
  Derived11 d11;
#endif

#ifdef X
  test(d0, 0, 1, 1, 0);
  test(d1, 0, 1, 1, 0);
#endif

#ifdef Y
  test(d0, 0, 1, 2, 3);
  test(d1, 0, 1, 2, 3);
#endif

#ifdef Z
#ifdef Z1
  test(d00, 0, 1, 0, 1);
  test(d01, 0, 1, 0, 1);
#elif defined(Z2)
  test(d10, 2, 3, 2, 3);
  test(d11, 2, 3, 2, 3);
#elif defined(Z3)
  test(d01, 1, 3, 1, 3);
  test(d11, 1, 3, 1, 3);
#endif
#endif
  return 0;
}
