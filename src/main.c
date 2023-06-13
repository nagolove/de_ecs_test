// vim: set colorcolumn=85
// vim: fdm=marker


#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#else
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#endif

#include "munit.h"
#include "koh.h"
#include "raylib.h"
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MunitResult test_emplace_destroy(
    const MunitParameter params[], void* data
) {
    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/emplace_destroy",
    test_emplace_destroy,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "de_ecs", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};



int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
