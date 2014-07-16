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
double test_results[NUM_TESTS];

double result = 0;
double input2 = 0;

/* Here is the register setup:
 *   rbx - pointer to first double argument
 *   edi - pointer to second double argument
 *
 * The result should be put into the location pointed to by rbx.
 */

#define TEST(TESTNAME, INPUT1, INPUT2, SETUP, ASM, EPILOGUE) \
  result = INPUT1; \
  input2 = INPUT2; \
  __asm volatile( \
    SETUP \
    TIME_BEGIN \
    ASM \
    TIME_END \
    EPILOGUE \
    : "=a" (tests[i]) \
    : "b" (&result), "D" (&input2) \
    : "%rax", "%rcx", "%rdx", "%rsi", "memory");

#define REPEAT_TEST for(int i = -1; i < NUM_TESTS; i++)
#define PRINT_TEST_RESULTS(TESTNAME) \
  printf("" #TESTNAME ": ["); for(int i = 0; i < NUM_TESTS; i++) { printf("%d, ", tests[i] ); } printf("]\n"); \

#define DO_TEST(TESTNAME, INPUT1, INPUT2, SETUP, ASM, EPILOGUE) \
  REPEAT_TEST { \
    TEST(TESTNAME, INPUT1, INPUT2, SETUP, ASM, EPILOGUE); \
    if (i >= 0) { test_results[i] = result; } \
  }
#define DO_TEST1(TESTNAME, INPUT1, SETUP, ASM, EPILOGUE) \
  DO_TEST(TESTNAME, INPUT1, 0, SETUP, ASM, EPILOGUE)
#define DO_TEST0(TESTNAME, SETUP, ASM, EPILOGUE) \
  DO_TEST(TESTNAME, 0, 0, SETUP, ASM, EPILOGUE)

#define DO_TEST_PRINT(TESTNAME, INPUT1, INPUT2, SETUP, ASM, EPILOGUE) \
  DO_TEST(TESTNAME, INPUT1, INPUT2, SETUP, ASM, EPILOGUE) \
  PRINT_TEST_RESULTS(TESTNAME);
#define DO_TEST_PRINT1(TESTNAME, INPUT1, SETUP, ASM, EPILOGUE) \
  DO_TEST_PRINT(TESTNAME, INPUT1, 0, SETUP, ASM, EPILOGUE);
#define DO_TEST_PRINT0(TESTNAME, SETUP, ASM, EPILOGUE) \
  DO_TEST_PRINT(TESTNAME, 0,0, SETUP, ASM, EPILOGUE);

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


  DO_TEST_PRINT0(noop_cold, "", "", "");

  for(int j = 0; j < 300000; j++) {
    DO_TEST0(noop, "", "", "");
  }

  DO_TEST_PRINT0(noop_warmish, "", "", "");

  DO_TEST_PRINT(pi_multiply,
      2, 0,
      "fldl (%%rbx);\n"
      "fldpi;\n"
      ,
      "fmul %%st(1);\n"
      ,
      /* fcomp is the easiest way to pop an item from the stack */
      "fstpl (%%rbx);\n"
      "fcomp;\n"
      );

  DO_TEST_PRINT(integer_multiply,
      2, 4,
      "fldl (%%rbx);\n"
      "fldl (%%rdi);\n"
      ,
      "fmul %%st(1);\n"
      ,
      "fstpl (%%rbx);\n"
      "fcomp;\n"
      );

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
