#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef SSE
#include <xmmintrin.h>
#endif

#define DUP1(X) X
#define DUP2(X) X X
#define DUP3(X) DUP1(X) DUP2(X)
#define DUP4(X) DUP2(X) DUP2(X)
#define DUP5(X) DUP3(X) DUP2(X)
#define DUP6(X) DUP3(X) DUP3(X)
#define DUP7(X) DUP4(X) DUP3(X)

#define TIME_BEGIN "rdtscp; \nmov %%rax, %%rsi\n"
#define TIME_END "rdtscp; \nsub %%esi, %%eax\n"

#define NUM_TESTS 40
#define LOOP_ITERATIONS 100000

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
    : "rcx", "rdx", "rsi", "memory" );

#define REPEAT_TEST for(int i = -1; i < NUM_TESTS; i++)

#if 0
#define PRINT_TEST_RESULTS(TESTNAME) \
{ uint64_t s = 0; printf("" #TESTNAME ": ["); for(int i = 0; i < NUM_TESTS; i++) { s += tests[i]; printf("%d, ", tests[i]); } printf("] sum: %ull\n", s); }
#else
#define PRINT_TEST_RESULTS(TESTNAME) \
{ uint64_t s = 0; printf("" #TESTNAME ": ["); for(int i = 0; i < NUM_TESTS; i++) { s += tests[i]; printf("%d (%.12g), ", tests[i], test_results[i]); } printf("] sum: %" PRIu64 "\n", s); }
#endif
#define PRINT_SUMMARY(TESTNAME) \
{ uint64_t s = 0; printf("%-16s: ", #TESTNAME); for(int i = 0; i < NUM_TESTS; i++) { s += tests[i]; } printf(" sum: %" PRIu64 "\n", s); }

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


  DO_TEST0(noop_cold, "", "", "");

  for(int j = 0; j < 300000; j++) {
    DO_TEST0(noop, "", "", "");
  }

  DO_TEST_PRINT0(noop_warmish, "", "", "");

  #define fcomp7 DUP7("fcomp;\n")

  DO_TEST_PRINT1(pi_multiply,
      1e-310,
      "fldpi;\n"
      "fldl (%%rbx);\n"
      ,
      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"

      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"
      "fmul %%ST(0), %%ST(1);\n"


      ,
      /* fcomp is the easiest way to pop an item from the stack */
      "fcomp;\n"
      "fstl (%%rbx);\n"
      "fcomp;\n"
      );

#define DO_MUL_PI(NAME, NUM) \
  DO_TEST_PRINT1(pi_multiply_##NAME, \
      NUM, \
      "fldpi;\n" \
      "fldl (%%rbx);\n" \
      , \
      "fmul %%ST(0), %%ST(1);\n" \
      , \
      /* fcomp is the easiest way to pop an item from the stack */ \
      "fcomp;\n" \
      "fstl (%%rbx);\n" \
      "fcomp;\n" \
      );

  //DO_MUL_PI(2, 2);
  //DO_MUL_PI(3, 3);
  //DO_MUL_PI(denorm, 1e-310);

  /* LOOP_INTERNAL should:
   *   operate on ST(0) and ST(1)
   *   not change the stack size
   *   not touch $rax
   */
#define TEST_INTERNAL_LOOP(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  TEST(TESTNAME##_rep, INPUT1, INPUT2, SETUP, \
      "mov $" #LOOP_ITER ", %%rax;\n" \
      "jmp loop%=;" \
      \
      "top%=:" \
      "fcomp;\n" \
      "fcomp;\n" \
      \
      "loop%=:" \
      "fld %%ST(1);\n" \
      "fld %%ST(1);\n" \
      LOOP_INTERNAL \
      "dec %%rax;\n" \
      "jnz top%=;\n" \
      , \
      EPILOGUE \
      "fcomp;\n" \
      "fcomp;\n" \
      );
#define DO_LOOP_TEST(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  REPEAT_TEST { \
    TEST_INTERNAL_LOOP(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE); \
    if (i >= 0) { test_results[i] = result; } \
  }

#define DO_LOOP_TEST_PRINT(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  DO_LOOP_TEST(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  PRINT_TEST_RESULTS(TESTNAME);

#define fmul_loop_test(NAME, INPUT1, INPUT2) \
  DO_LOOP_TEST(NAME##_loop, INPUT1, INPUT2, LOOP_ITERATIONS, \
        "fldl (%%rbx);\n" \
        "fldl (%%rdi);\n" \
        , \
        "fmul %%ST(1), %%ST(0);\n" \
        "fstl (%%rbx);\n" \
        , \
        "fstl (%%rbx);\n" \
        "fcomp;\n" \
        "fcomp;\n" \
        );

#define fmul_loop_test_print(NAME, INPUT1, INPUT2) \
  fmul_loop_test(NAME, INPUT1, INPUT2) \
  PRINT_TEST_RESULTS(NAME)
#define fmul_loop_test_summary(NAME, INPUT1, INPUT2) \
  fmul_loop_test(NAME, INPUT1, INPUT2) \
  PRINT_SUMMARY(NAME)

  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);
  fmul_loop_test(denorm, 7, 1e-310);

  fmul_loop_test_summary(integers, 1, 0);
  fmul_loop_test_summary(denorm, 1e-310, 0.1);
  fmul_loop_test_summary(denorm, 7, 1e-310);
  fmul_loop_test_summary(zero, 0.1, 0.1);
  fmul_loop_test_summary(two, 7, 2);


#define SSE_TEST_INTERNAL_LOOP(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  TEST(TESTNAME##_rep, INPUT1, INPUT2, SETUP, \
      "mov $" #LOOP_ITER ", %%rax;\n" \
      "jmp loop%=;" \
      \
      "top%=:" \
      \
      "loop%=:" \
      "movsd %%xmm2, %%xmm0;\n" \
      "movsd %%xmm3, %%xmm1;\n" \
      LOOP_INTERNAL \
      "dec %%rax;\n" \
      "jnz top%=;\n" \
      , \
      EPILOGUE \
      );

#define DO_SSE_LOOP_TEST(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE) \
  REPEAT_TEST { \
    SSE_TEST_INTERNAL_LOOP(TESTNAME, INPUT1, INPUT2, LOOP_ITER, SETUP, LOOP_INTERNAL, EPILOGUE); \
    if (i >= 0) { test_results[i] = result; } \
  }

#define mulsd_loop_test(NAME, INPUT1, INPUT2) \
  DO_SSE_LOOP_TEST(NAME##_loop, INPUT1, INPUT2, LOOP_ITERATIONS, \
        "movsd (%%rbx), %%xmm2;\n" \
        "movsd (%%rdi), %%xmm3;\n" \
        , \
        "mulsd %%xmm0, %%xmm1;\n" \
        , \
        "movsd %%xmm1, (%%rbx);\n" \
        );

#define mulsd_loop_test_print(NAME, INPUT1, INPUT2) \
  mulsd_loop_test(NAME, INPUT1, INPUT2); \
  PRINT_TEST_RESULTS(NAME);
#define mulsd_loop_test_summary(NAME, INPUT1, INPUT2) \
  mulsd_loop_test(NAME, INPUT1, INPUT2); \
  PRINT_SUMMARY(NAME);

  mulsd_loop_test_summary(sse_base, 1, 0);
  mulsd_loop_test_summary(sse_two, 1, 2);
  mulsd_loop_test_summary(sse_hard, 5.4, 6.7);
  mulsd_loop_test_summary(sse_large, 1e50, 5.3);
  mulsd_loop_test_summary(denorm, 1e-310, 0.1);
  mulsd_loop_test_summary(zero, 0.1, 0.1);

  /*DO_TEST_PRINT(integer_multiply,
      2, 4,
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rbx);\n"
      "fldl (%%rdi);\n"
      ,
      "fmul %%st(1);\n"
      ,
      "fstpl (%%rbx);\n"
      "fcomp;\n"
      );
      */



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
