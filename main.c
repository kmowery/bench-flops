#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef SSE
#include <xmmintrin.h>
#endif

#define TIME_BEGIN "rdtsc;\nmov%%rax, %%rsi\n"
#define TIME_END "rdtsc;\nsub %%esi, %%eax\n"

union conv {
  uint64_t x;
  double f;
};

int main() {
  union conv c;
  double f1, f2, f3;

#ifdef SSE
  // flush SSE denorms to zero
  _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
#endif

  printf("len uint64: %lu\n", sizeof(c.x));
  printf("len dbl:    %lu\n", sizeof(c.f));

  c.f = atof("1e-310");

  printf("int:   0x%016" PRIX64 "\n", c.x);
  printf("float: %f\n", c.f);
  printf("caml:  %.12g\n", c.f);


  f1 = c.f+0;
  f2 = c.f*1;

  f3 = f1 + f2;

  printf("%.12g\n", f1);
  printf("%.12g\n", f2);
  printf("%.12g\n", f3);
  // will print the byte of the double, with a compiler error
  //printf("f3:   0x%016" PRIX64 "\n", f3);

  uint32_t diff;
  double result = 2;

  __asm volatile(


      "fldl (%%rbx);\n"

      TIME_BEGIN
      "fldpi;\n"

      TIME_END

      "fmul %%st(1);\n"

      "fstl (%%rbx);\n"

    : "=a" (diff) //, "=b" (result)
    : "b" (&result)
    : "%esi" );

  printf("diff: 0x%x\n", diff);
  printf("%.12g\n", result);
  printf("%.12g\n", 1.2);


  return 0;
}
