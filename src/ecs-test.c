// vim: set colorcolumn=85
// vim: fdm=marker


#include "koh_destral_ecs.h"
#include "koh_table.h"
#include "koh_set.h"
#include "munit.h"
#include "koh.h"
#include "raylib.h"
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Cell {
    int             value;
    int             from_x, from_y, to_x, to_y;
    bool            moving;
};

struct Triple {
    float dx, dy, dz;
};

static bool make_output = false;

static const de_cp_type cmp_triple = {
    .cp_id = 0,
    .cp_sizeof = sizeof(struct Triple),
    .name = "triple",
    .initial_cap = 2000,
};

static const de_cp_type cmp_cell = {
    .cp_id = 1,
    .cp_sizeof = sizeof(struct Cell),
    .name = "cell",
    .initial_cap = 20000,
};

/*static HTable *table = NULL;*/

/*
static struct Cell *create_cell_h(de_ecs *r, int x, int y) {

    //if (get_cell_count(mv) >= FIELD_SIZE * FIELD_SIZE)
        //return NULL;

    de_entity en = de_create(r);
    struct Cell *cell = de_emplace(r, en, cmp_cell);
    munit_assert_ptr_not_null(cell);
    cell->from_x = x;
    cell->from_y = y;
    cell->to_x = rand() % 1000;
    cell->to_y = rand() % 1000;
    cell->value = rand() % 1000;

    assert(table);
    htable_add(table, &en, sizeof(en), cell, sizeof(*cell));

    trace("create_cell: en %ld at (%d, %d)\n", en, cell->from_x, cell->from_y);
    return cell;
}
*/

static struct Triple *create_triple(
    de_ecs *r, de_entity en, const struct Triple tr
) {

    //if (get_cell_count(mv) >= FIELD_SIZE * FIELD_SIZE)
        //return NULL;

    assert(r);
    assert(de_valid(r, en));
    struct Triple *triple = de_emplace(r, en, cmp_triple);
    munit_assert_ptr_not_null(triple);
    *triple = tr;

    if (make_output)
        trace(
            "create_tripe: en %ld at (%d, %d, %d)\n", 
            en, triple->dx, triple->dy, triple->dz
        );

    return triple;
}

static struct Cell *create_cell(de_ecs *r, int x, int y) {

    //if (get_cell_count(mv) >= FIELD_SIZE * FIELD_SIZE)
        //return NULL;

    de_entity en = de_create(r);
    struct Cell *cell = de_emplace(r, en, cmp_cell);
    munit_assert_ptr_not_null(cell);
    cell->from_x = x;
    cell->from_y = y;
    cell->to_x = x;
    cell->to_y = y;
    cell->value = -1;

    if (make_output)
        trace(
            "create_cell: en %ld at (%d, %d)\n",
            en, cell->from_x, cell->from_y
        );
    return cell;
}

static void iter_set_add(de_ecs* r, de_entity en, void* udata) {
    koh_Set *entts = udata;
    set_add(entts, &en, sizeof(en));
}

static MunitResult test_ecs_clone(
    const MunitParameter params[], void* data
) {

    de_ecs *r = de_ecs_make();

    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            struct Cell *cell = create_cell(r, x, y);
            munit_assert_ptr_not_null(cell);
        }
    }
    de_ecs *cloned = de_ecs_clone(r);
    koh_Set *entts1 = set_new();
    koh_Set *entts2 = set_new();

    de_each(r, iter_set_add, entts1);
    de_each(cloned, iter_set_add, entts1);

    munit_assert(set_compare(entts1, entts2));

    set_free(entts1);
    set_free(entts2);
    de_ecs_destroy(r);

    return MUNIT_OK;
}

/*
static MunitResult test_emplace_destroy_with_hash(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();
    table = htable_new(&(struct HTableSetup) {
        .cap = 30000,
        .on_remove = NULL,
    });

    for (int k = 0; k < 10; k++) {

        for (int x = 0; x < 100; x++) {
            for (int y = 0; y < 100; y++) {
                struct Cell *cell = create_cell(r, x, y);
                munit_assert_ptr_not_null(cell);
            }
        }

        for (int j = 0; j < 10; j++) {

            for (int i = 0; i < 10; i++) {

                for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
                        de_view_valid(&v); de_view_next(&v)) {

                    munit_assert(de_valid(r, de_view_entity(&v)));
                    struct Cell *c = de_view_get_safe(&v, cmp_cell);
                    munit_assert_ptr_not_null(c);

                    c->moving = false;
                    c->from_x = rand() % 100 + 10;
                    c->from_y = rand() % 100 + 10;
                }

            }

            for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
                    de_view_valid(&v); de_view_next(&v)) {


                munit_assert(de_valid(r, de_view_entity(&v)));
                struct Cell *c = de_view_get_safe(&v, cmp_cell);
                munit_assert_ptr_not_null(c);

                if (c->from_x == 10 || c->from_y == 10) {
                    printf("removing entity\n");
                    de_destroy(r, de_view_entity(&v));
                }
            }

        }
    }

    for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
        de_view_valid(&v); de_view_next(&v)) {

        munit_assert(de_valid(r, de_view_entity(&v)));
        struct Cell *c = de_view_get_safe(&v, cmp_cell);
        munit_assert_ptr_not_null(c);

        munit_assert_int(c->from_x, >=, 10);
        munit_assert_int(c->from_x, <=, 10 + 100);
        munit_assert_int(c->from_y, >=, 10);
        munit_assert_int(c->from_y, <=, 10 + 100);
        munit_assert(c->moving == false);
    }

    de_ecs_destroy(r);
    htable_free(table);
    table = NULL;
    return MUNIT_OK;
}
*/

static MunitResult test_emplace_destroy(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();
    de_entity entts[1000] = {0};
    int entts_num = 0;

    for (int k = 0; k < 3; k++) {

        for (int x = 0; x < 100; x++) {
            for (int y = 0; y < 100; y++) {
                struct Cell *cell = create_cell(r, x, y);
                munit_assert_ptr_not_null(cell);
            }
        }

        for (int j = 0; j < 3; j++) {

            for (int i = 0; i < 10; i++) {

                for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
                        de_view_valid(&v); de_view_next(&v)) {

                    munit_assert(de_valid(r, de_view_entity(&v)));
                    struct Cell *c = de_view_get_safe(&v, cmp_cell);
                    munit_assert_ptr_not_null(c);

                    c->moving = false;
                    c->from_x = rand() % 100 + 10;
                    c->from_y = rand() % 100 + 10;
                }

            }

            for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
                    de_view_valid(&v); de_view_next(&v)) {


                munit_assert(de_valid(r, de_view_entity(&v)));
                struct Cell *c = de_view_get_safe(&v, cmp_cell);
                munit_assert_ptr_not_null(c);

                if (c->from_x == 10 || c->from_y == 10) {
                    if (make_output) 
                        printf("removing entity\n");
                    de_destroy(r, de_view_entity(&v));
                } else {
                    if (entts_num < sizeof(entts) / sizeof(entts[0])) {
                        entts[entts_num++] = de_view_entity(&v);
                    }
                }
            }

        }
    }

    /*
    for (int i = 0; i < entts_num; ++i) {
        if (de_valid(r, entts[i])) {
            munit_assert(de_has(r, entts[i], cmp_cell));
            de_destroy(r, entts[i]);
        }
    }
    */

    for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cmp_cell }); 
        de_view_valid(&v); de_view_next(&v)) {

        munit_assert(de_valid(r, de_view_entity(&v)));
        struct Cell *c = de_view_get_safe(&v, cmp_cell);
        munit_assert_ptr_not_null(c);

        munit_assert_int(c->from_x, >=, 10);
        munit_assert_int(c->from_x, <=, 10 + 100);
        munit_assert_int(c->from_y, >=, 10);
        munit_assert_int(c->from_y, <=, 10 + 100);
        munit_assert(c->moving == false);
    }

    de_ecs_destroy(r);
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
  {
    (char*) "/ecs_clone",
    test_ecs_clone,
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
