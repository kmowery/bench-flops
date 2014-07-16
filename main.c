#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef SSE
#include <xmmintrin.h>
#endif

#define TIME_BEGIN "rdtscp; \nmov%%rax, %%rsi\n"
#define TIME_END "rdtscp; \nsub %%esi, %%eax\n"

#define NUM_TESTS 40

#define DEF_TESTS(TESTNAME) int32_t TESTNAME##_tests[NUM_TESTS];
int32_t tests[NUM_TESTS];

#define TEST(TESTNAME, SETUP, ASM) __asm volatile( SETUP TIME_BEGIN ASM TIME_END : "=a" (tests[i]) : "b" (&result) : "%rax", "%rcx", "%rdx", "%rsi", "memory");
#define REPEAT_TEST for(int i = 0; i < NUM_TESTS; i++)
#define PRINT_TEST_RESULTS(TESTNAME) printf("" #TESTNAME ": ["); for(int i = 0; i < NUM_TESTS; i++) { printf("%d, ", tests[i] ); } printf("]\n");

#define DO_TEST(TESTNAME, SETUP, ASM) \
  REPEAT_TEST { \
    TEST(TESTNAME, SETUP, ASM); \
  }

#define DO_TEST_PRINT(TESTNAME, SETUP, ASM) \
  DO_TEST(TESTNAME, SETUP, ASM) \
  PRINT_TEST_RESULTS(TESTNAME);

union conv {
  uint64_t x;
  double f;
};

int main() {

#ifdef SSE
  // flush SSE denorms to zero
  _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
#endif

  //union conv c;
  //double f1, f2, f3;

  //printf("len uint64: %lu\n", sizeof(c.x));
  //printf("len dbl:    %lu\n", sizeof(c.f));

  //c.f = atof("1e-310");

  //printf("int:   0x%016" PRIX64 "\n", c.x);
  //printf("float: %f\n", c.f);
  //printf("caml:  %.12g\n", c.f);


  //f1 = c.f+0;
  //f2 = c.f*1;

  //f3 = f1 + f2;

  //printf("%.12g\n", f1);
  //printf("%.12g\n", f2);
  //printf("%.12g\n", f3);
  // will print the byte of the double, with a compiler error
  //printf("f3:   0x%016" PRIX64 "\n", f3);
  //
  //
  //


  // WARM UP PROCESSOR, seems to reduce rdtsc variability
  int a = 0,b = 1,c = 1;

  for(int i = 0; i < 300000; i++) {
    a = b + c;
    c = b;
    b = a;
  }

  double result = 0;

  DO_TEST_PRINT(noop, "", "");

  //__asm volatile(
  //    "fldl (%%rbx);\n"

  //    "fldpi;\n"


  //    TIME_BEGIN
  //    "fmul %%st(1);\n"
  //    TIME_END

  //    "fstl (%%rbx);\n"

  //  : "=a" (diff) //, "=b" (result)
  //  : "b" (&result)
  //  : "%esi", "memory" );

  //printf("diff: 0x%x\n", diff);
  //printf("%.12g\n", result);
  //printf("%.12g\n", 1.2);


  return 0;
}
