// vim: set colorcolumn=85
// vim: fdm=marker

// {{{ include
#include "koh.h"
#include "koh_common.h"
#include "koh_destral_ecs.h"
#include "koh_destral_ecs_internal.h"
#include "koh_routine.h"
#include "koh_set.h"
#include "koh_strset.h"
#include "koh_table.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// }}}

struct Cell {
    int             value;
    int             from_x, from_y, to_x, to_y;
    bool            moving;
};

struct Triple {
    float dx, dy, dz;
};

struct Node {
    char u;
};

static bool verbose_print = false;

static const de_cp_type cp_triple = {
    .cp_id = 0,
    .cp_sizeof = sizeof(struct Triple),
    .name = "triple",
    .initial_cap = 2000,
};

static const de_cp_type cp_cell = {
    .cp_id = 1,
    .cp_sizeof = sizeof(struct Cell),
    .name = "cell",
    .initial_cap = 20000,
};

static const de_cp_type cp_node = {
    .cp_id = 2,
    .cp_sizeof = sizeof(struct Node),
    .name = "node",
    .initial_cap = 20,
};

/*static HTable *table = NULL;*/

/*
static struct Cell *create_cell_h(de_ecs *r, int x, int y) {

    //if (get_cell_count(mv) >= FIELD_SIZE * FIELD_SIZE)
        //return NULL;

    de_entity en = de_create(r);
    struct Cell *cell = de_emplace(r, en, cp_cell);
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

// TODO: использовать эту функцию для тестирования сложносоставных сущностей
static struct Triple *create_triple(
    de_ecs *r, de_entity en, const struct Triple tr
) {

    assert(r);
    assert(de_valid(r, en));
    struct Triple *triple = de_emplace(r, en, cp_triple);
    munit_assert_ptr_not_null(triple);
    *triple = tr;

    if (verbose_print)
        trace(
            "create_tripe: en %u at (%f, %f, %f)\n", 
            en, triple->dx, triple->dy, triple->dz
        );

    return triple;
}

static struct Cell *create_cell(de_ecs *r, int x, int y, de_entity *e) {

    //if (get_cell_count(mv) >= FIELD_SIZE * FIELD_SIZE)
        //return NULL;

    de_entity en = de_create(r);
    munit_assert(en != de_null);
    struct Cell *cell = de_emplace(r, en, cp_cell);
    munit_assert_ptr_not_null(cell);
    cell->from_x = x;
    cell->from_y = y;
    cell->to_x = x;
    cell->to_y = y;
    cell->value = -1;

    if (e) *e = en;

    if (verbose_print)
        trace(
            "create_cell: en %u at (%d, %d)\n",
            en, cell->from_x, cell->from_y
        );
    return cell;
}

static bool iter_set_add_mono(de_ecs* r, de_entity en, void* udata) {
    StrSet *entts = udata;

    struct Cell *cell = de_try_get(r, en, cp_cell);
    munit_assert_ptr_not_null(cell);

    char repr_cell[256] = {};
    if (verbose_print)
        sprintf(repr_cell, "en %u, cell %d %d %s %d %d %d", 
            en,
            cell->from_x, cell->from_y,
            cell->moving ? "t" : "f",
            cell->to_x, cell->to_y,
            cell->value
        );

    strset_add(entts, repr_cell);
    return false;
}

static bool iter_set_add_multi(de_ecs* r, de_entity en, void* udata) {
    StrSet *entts = udata;

    struct Cell *cell = de_try_get(r, en, cp_cell);
    munit_assert_ptr_not_null(cell);

    struct Triple *triple = de_try_get(r, en, cp_triple);
    munit_assert_ptr_not_null(triple);

    char repr_cell[256] = {};
    if (verbose_print)
        sprintf(repr_cell, "en %u, cell %d %d %s %d %d %d", 
            en,
            cell->from_x, cell->from_y,
            cell->moving ? "t" : "f",
            cell->to_x, cell->to_y,
            cell->value
        );

    char repr_triple[256] = {};
    if (verbose_print)
        sprintf(repr_triple, "en %u, %f %f %f", 
            en,
            triple->dx,
            triple->dy,
            triple->dz
        );

    char repr[strlen(repr_cell) + strlen(repr_triple) + 2];
    memset(repr, 0, sizeof(repr));

    strcat(strcat(repr, repr_cell), repr_triple);

    strset_add(entts, repr);
    return false;
}

/*
static koh_SetAction iter_set_print(
    const void *key, int key_len, void *udata
) {
    munit_assert(key_len == sizeof(de_entity));
    const de_entity *en = key;
    //printf("(%f, %f)\n", vec->x, vec->y);
    printf("en %u\n", *en);
    return koh_SA_next;
}
*/

static MunitResult test_try_get_none_existing_component(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    //for (int i = 0; i < 5000; ++i) {
    for (int i = 0; i < 50; ++i) {
        de_entity en = de_create(r);

        struct Cell *cell;
        struct Triple *triple;

        cell = de_emplace(r, en, cp_cell);
        cell->moving = true;

        cell = NULL;
        cell = de_try_get(r, en, cp_cell);
        assert(cell);

        ///////////// !!!!!
        triple = NULL;
        triple = de_try_get(r, en, cp_triple);
        assert(!triple);
        ///////////// !!!!!

        cell = NULL;
        cell = de_try_get(r, en, cp_cell);
        assert(cell);
    }

    de_ecs_destroy(r);
    return MUNIT_OK;
}

static MunitResult test_ecs_clone_multi(
    const MunitParameter params[], void* data
) {

    de_ecs *r = de_ecs_make();

    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            de_entity en = de_null;
            struct Cell *cell = create_cell(r, x, y, &en);
            munit_assert(en != de_null);
            create_triple(r, en, (struct Triple) {
                .dx = x,
                .dy = y,
                .dz = x * y,
            });
            munit_assert_ptr_not_null(cell);
        }
    }
    de_ecs  *cloned = de_ecs_clone(r);
    StrSet *entts1 = strset_new();
    StrSet *entts2 = strset_new();

    de_each(r, iter_set_add_multi, entts1);
    de_each(cloned, iter_set_add_multi, entts2);

    /*
    printf("\n"); printf("\n"); printf("\n");
    set_each(entts1, iter_set_print, NULL);

    printf("\n"); printf("\n"); printf("\n");
    set_each(entts2, iter_set_print, NULL);
    */

    munit_assert(strset_compare(entts1, entts2));

    strset_free(entts1);
    strset_free(entts2);
    de_ecs_destroy(r);
    de_ecs_destroy(cloned);

    return MUNIT_OK;
}

// TODO: Добавить проверку компонент сущностей
static MunitResult test_ecs_clone_mono(
    const MunitParameter params[], void* data
) {

    de_ecs *r = de_ecs_make();

    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            struct Cell *cell = create_cell(r, x, y, NULL);
            munit_assert_ptr_not_null(cell);
        }
    }
    de_ecs  *cloned = de_ecs_clone(r);
    StrSet *entts1 = strset_new();
    StrSet *entts2 = strset_new();

    de_each(r, iter_set_add_mono, entts1);
    de_each(cloned, iter_set_add_mono, entts2);

    /*
    printf("\n"); printf("\n"); printf("\n");
    strset_each(entts1, iter_strset_print, NULL);

    printf("\n"); printf("\n"); printf("\n");
    strset_each(entts2, iter_strset_print, NULL);
    */

    munit_assert(strset_compare(entts1, entts2));

    strset_free(entts1);
    strset_free(entts2);
    de_ecs_destroy(r);
    de_ecs_destroy(cloned);

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

                for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
                        de_view_valid(&v); de_view_next(&v)) {

                    munit_assert(de_valid(r, de_view_entity(&v)));
                    struct Cell *c = de_view_get_safe(&v, cp_cell);
                    munit_assert_ptr_not_null(c);

                    c->moving = false;
                    c->from_x = rand() % 100 + 10;
                    c->from_y = rand() % 100 + 10;
                }

            }

            for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
                    de_view_valid(&v); de_view_next(&v)) {


                munit_assert(de_valid(r, de_view_entity(&v)));
                struct Cell *c = de_view_get_safe(&v, cp_cell);
                munit_assert_ptr_not_null(c);

                if (c->from_x == 10 || c->from_y == 10) {
                    printf("removing entity\n");
                    de_destroy(r, de_view_entity(&v));
                }
            }

        }
    }

    for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
        de_view_valid(&v); de_view_next(&v)) {

        munit_assert(de_valid(r, de_view_entity(&v)));
        struct Cell *c = de_view_get_safe(&v, cp_cell);
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

struct EntityState {
    de_entity   e;
    bool        components_set[3];
    int         components_values[3];
    bool        found;
};

struct TestDestroyCtx {
    de_cp_type          components[3];
    int                 comp_num;
    de_ecs              *r;
    struct koh_Set      *set;
    int                 entt_num, last_comp_value;
    int                 index_target, index_current;
    struct EntityState  estate_target;
};

//__attribute__((unused))
static char *estate2str(const struct EntityState *estate) {

    if (!estate) {
        koh_trap();
    }

    static char buf[128] = {};
    int comp_num = 
        sizeof(estate->components_values) / 
        sizeof(estate->components_values[0]);

    char *pbuf = buf;
    pbuf += sprintf(pbuf, "e %u, ", estate->e);
    for (int i = 0; i < comp_num; i++) {
        if (estate && estate->components_set[i]) {
            pbuf += sprintf(pbuf, "[%u %u]", i, estate->components_values[i]);
        }
    }

    pbuf += sprintf(pbuf, ", found %s", estate->found ? "true" : "false");

    return buf;
}

static koh_SetAction iter_set_search_and_remove(
    const void *key, int key_len, void *udata
) {
    assert(udata);
    assert(key);

    const struct EntityState *key_state = key;
    struct TestDestroyCtx *ctx = udata;

    if (verbose_print) {
        printf("iter_set_search: index_current %d\n", ctx->index_current);
        printf("iter_set_search: index_target %d\n", ctx->index_target);
    }

    if (ctx->index_current == ctx->index_target) {
        if (verbose_print) 
            printf(
                "iter_set_search: %s found and removed\n",
                estate2str(key_state)
            );
        ctx->estate_target = *key_state;
        ctx->estate_target.found = true;
        return koh_SA_remove_break;
    }

    ctx->index_current++;

    return koh_SA_next;
}

// Удаляет одну случайную сущность из системы.
static struct TestDestroyCtx destroy_entt(struct TestDestroyCtx ctx) {
    if (verbose_print)
        printf("destroy_entt:\n");

    if (ctx.entt_num == 0) {
        printf("destroy_entt: ctx.ennt_num == 0\n");
        koh_trap();
    }

    if (verbose_print)
        printf("ctx.ennt_num %d\n", ctx.entt_num);

    memset(&ctx.estate_target, 0, sizeof(ctx.estate_target));
    ctx.estate_target.e = de_null;

    // подготовка к поиску
    ctx.index_current = 0;

    if (ctx.entt_num == 1)
        ctx.index_target = 0;
    else
        ctx.index_target = random() % (ctx.entt_num - 1) + 1;

    if (verbose_print)
        printf("index_target %d\n", ctx.index_target);

    // итератор поиска и удаления одной записи из множества
    set_each(ctx.set, iter_set_search_and_remove, &ctx);

    munit_assert_uint32(ctx.estate_target.e, !=, de_null);
    munit_assert(ctx.estate_target.found);

    if (verbose_print)
        printf("destroy_entt: e %u\n", ctx.estate_target.e);

    de_destroy(ctx.r, ctx.estate_target.e);

    /*
    ctx.estate_target.found = false;
    bool is_removed = set_remove(
        ctx.set, &ctx.estate_target, sizeof(ctx.estate_target)
    );
    */

    //munit_assert(is_removed);
    ctx.entt_num--;
    return ctx;
}

static struct TestDestroyCtx create_one(struct TestDestroyCtx ctx) {
    assert(ctx.r);

    struct EntityState estate = {
        .e = de_create(ctx.r),
    };
    munit_assert_uint32(estate.e, !=, de_null);

    char fingerprint[128] = {};
    char *pfingerprint = fingerprint;

    const int estate_comp_num = 
        sizeof(estate.components_values) / 
        sizeof(estate.components_values[0]);
    assert(estate_comp_num <= ctx.comp_num);

    for (int comp_index = 0; comp_index < ctx.comp_num; comp_index++) {
        // Может создать компонент, а может и нет?
        if ((double)rand() / (double)RAND_MAX < 0.5) 
            continue;

        de_cp_type comp = ctx.components[comp_index];
        int *c = de_emplace(ctx.r, estate.e, comp);
        munit_assert_not_null(c);
        *c = ctx.last_comp_value++;

        estate.components_set[comp_index] = true;
        estate.components_values[comp_index] = *c;

        pfingerprint += sprintf(
            pfingerprint, "[%zu  %d]", comp.cp_id, *c
        );
    }

    if (verbose_print)
        printf("e %u, fingerprint %s\n", estate.e, fingerprint);

    set_add(ctx.set, &estate, sizeof(estate));
    ctx.entt_num++;

    return ctx;
}

static koh_SetAction set_print_each(
    const void *key, int key_len, void *udata
) {
    assert(key);
    printf("    %s\n", estate2str(key));
    return koh_SA_next;
}

static void estate_set_print(koh_Set *set) {
    if (verbose_print) {
        printf("estate {\n");
        set_each(set, set_print_each, NULL);
        printf("} (size = %d)\n", set_size(set));
    }
}

static bool iter_ecs_each(de_ecs *r, de_entity e, void *udata) {
    struct TestDestroyCtx *ctx = udata;

    assert(r);
    assert(udata);
    assert(e != de_null);
    assert(ctx->set);

    struct EntityState estate = { 
        .e = e, 
        .found = false,
    };

    // сборка структуры estate через запрос к ecs
    for (int i = 0; i < ctx->comp_num; i++) {
        if (de_has(r, e, ctx->components[i])) {
            estate.components_set[i] = true;
            int *comp_value = de_try_get(r, e, ctx->components[i]);
            munit_assert_ptr_not_null(comp_value);
            estate.components_values[i] = *comp_value;
        }
    }

    if (verbose_print)
        printf("iter_ecs_each: search estate %s\n", estate2str(&estate));
    bool exists = set_exist(ctx->set, &estate, sizeof(estate));
    //bool exists = false;

    if (verbose_print)
        printf("estate {\n");
    for (struct koh_SetView v = set_each_begin(ctx->set);
        set_each_valid(&v); set_each_next(&v)) {
        const struct EntityState *key = set_each_key(&v);
        
        if (!key) {
            fprintf(stderr, "set_each_key return NULL\n");
            abort();
        }

        if (!memcmp(key, &estate, sizeof(estate))) {
            exists = true;
        }
        if (verbose_print)
            printf("    %s\n", estate2str(key));
    }
    if (verbose_print)
        printf("} (size = %d)\n", set_size(ctx->set));
    
    if (!exists) {
        if (verbose_print)
            printf("iter_ecs_each: not found\n");
        estate_set_print(ctx->set);
        munit_assert(exists);
    } else {
        if (verbose_print)
            printf("iter_ecs_each: EXISTS %s\n", estate2str(&estate));
    }

    return false;
}

// TODO: Не находится один компонент из всех во множестве
struct TestDestroyCtx ecs_check_each(struct TestDestroyCtx ctx) {
    assert(ctx.r);
    de_each(ctx.r, iter_ecs_each, &ctx);
    return ctx;
}

static bool iter_ecs_counter(de_ecs *r, de_entity e, void *udata) {
    int *counter = udata;
    (*counter)++;
    return false;
}

struct TestDestroyOneRandomCtx {
    de_entity   *entts;
    int         entts_len;
    de_cp_type  comp_type;
};

// Все сущности имеют один компонент
static bool iter_ecs_check_entt(de_ecs *r, de_entity e, void *udata) {
    struct TestDestroyOneRandomCtx *ctx = udata;
    for (int i = 0; i < ctx->entts_len; i++) {
        if (ctx->entts[i] == e) {
            munit_assert_true(de_has(r, e, ctx->comp_type));
        }
    }
    munit_assert(false);
    return false;
}

// Сложный тест, непонятно, что он делает
static MunitResult test_destroy_one_random(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    int entts_len = 4000;
    de_entity entts[entts_len];

    const static de_cp_type comp = {
        .cp_id = 0,
        .cp_sizeof = sizeof(int),
        .initial_cap = 1000,
        .name = "main component",
    };

    struct TestDestroyOneRandomCtx ctx = {
        .entts_len = entts_len,
        .entts = entts,
        .comp_type = comp,
    };

    for (int i = 0; i < entts_len; i++)
        entts[i] = de_null;
    //memset(entts, 0, sizeof(entts[0]) * entts_len);

    const int cycles = 1000;

    int comp_value_index = 0;
    for (int j = 0; j < cycles; j++) {

        // Добавляет в массив случайное количество сущносте с прикрепленной
        // компонентой
        int new_num = random() % 10;
        for (int i = 0; i < new_num; i++) {

            bool created = false;
            for (int k = 0; k < entts_len; ++k) {
                if (created)
                    break;

                if (entts[k] == de_null) {
                    //printf("creation cycle, k %d\n", k);
                    entts[k] = de_create(r);
                    munit_assert_uint32(entts[k], !=, de_null);
                    int *comp_value = de_emplace(r, entts[k], comp);
                    munit_assert_ptr_not_null(comp_value);
                    *comp_value = comp_value_index++;
                    created = true;
                }
            }
            munit_assert_true(created);

        }

        // Удаление случайно количество сущностей
        int destroy_num = random() % 5;
        for (int i = 0; i < destroy_num; ++i) {
            for (int k = 0; k < entts_len; ++k) {
                if (entts[k] != de_null) {
                    de_destroy(r, entts[k]);
                    entts[k] = de_null;
                }
            }
        }

    }

    // Все сущности имеют один компонент, проверка
    de_each(r, iter_ecs_check_entt, &ctx);

    // Удаление всех сущностей
    for (int k = 0; k < entts_len; ++k) {
        if (entts[k] != de_null)
            de_destroy(r, entts[k]);
    }

    // Проверка - сущностей не должно остаться
    int counter = 0;
    de_each(r, iter_ecs_counter, &counter);
    if (counter) {
        printf("test_destroy_one: counter %d\n", counter);
    }
    munit_assert_int(counter, ==, 0);

    de_ecs_destroy(r);
    return MUNIT_OK;
}

static MunitResult test_destroy_one(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    int entts_num = 40;
    de_entity entts[entts_num];
    memset(entts, 0, sizeof(entts[0]) * entts_num);

    const int cycles = 10;

    const static de_cp_type comp = {
        .cp_id = 0,
        .cp_sizeof = sizeof(int),
        .initial_cap = 1000,
        .name = "main component",
    };

    for (int j = 0; j < cycles; j++) {
        for (int i = 0; i < entts_num; i++) {
            entts[i] = de_create(r);
            munit_assert_uint32(entts[i], !=, de_null);
            int *comp_value = de_emplace(r, entts[i], comp);
            munit_assert_ptr_not_null(comp_value);
            *comp_value = i;
        }
        for (int i = 0; i < entts_num; i++) {
            de_destroy(r, entts[i]);
        }
    }

    int counter = 0;
    de_each(r, iter_ecs_counter, &counter);
    if (counter) {
        printf("test_destroy_one: counter %d\n", counter);
    }
    munit_assert_int(counter, ==, 0);

    de_ecs_destroy(r);
    return MUNIT_OK;
}

static MunitResult test_destroy_zero(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    int entts_num = 400;
    de_entity entts[entts_num];
    memset(entts, 0, sizeof(entts[0]) * entts_num);

    const int cycles = 1000;

    for (int j = 0; j < cycles; j++) {
        for (int i = 0; i < entts_num; i++) {
            entts[i] = de_create(r);
            munit_assert_uint32(entts[i], !=, de_null);
        }
        for (int i = 0; i < entts_num; i++) {
            de_destroy(r, entts[i]);
        }
    }

    int counter = 0;
    de_each(r, iter_ecs_counter, &counter);
    munit_assert_int(counter, ==, 0);

    de_ecs_destroy(r);
    return MUNIT_OK;
}

/*
    Задача - протестировать уничтожение сущностей вместе со всеми связанными
    компонентами.
    --
    Создается определенное количество сущностей, к каждой крепится от одного
    до 3х компонент.
    --
    Случайным образом удаляются несколько сущностей.
    Случайным образом добавляются несколько сущностей.
    --
    Проверка, что состояние ecs контейнера соответствует ожидаемому.
    Проверка происходит через de_view c одним компонентом
 */
__attribute__ ((unused)) static MunitResult test_destroy(
    const MunitParameter params[], void* data
) {
    printf("de_null %u\n", de_null);
    srand(time(NULL));

    /*struct StrSet *set = strset_new();*/
    /*struct koh_Set *set_ecs = set_new();*/

    // {{{ components
    const static de_cp_type comp1 = {
        .cp_id = 0,
        .cp_sizeof = sizeof(int),
        .initial_cap = 10000,
        .name = "comp1",
    };

    const static de_cp_type comp2 = {
        .cp_id = 1,
        .cp_sizeof = sizeof(int),
        .initial_cap = 10000,
        .name = "comp2",
    };

    const static de_cp_type comp3 = {
        .cp_id = 2,
        .cp_sizeof = sizeof(int),
        .initial_cap = 10000,
        .name = "comp3",
    };
    // }}}

    struct TestDestroyCtx ctx = {
        .r = de_ecs_make(),
        .set = set_new(NULL),
        .entt_num = 0,
        .comp_num = 3,
    };
    ctx.components[0] = comp1;
    ctx.components[1] = comp2;
    ctx.components[2] = comp3;

    printf("\n");

    int entities_num = 10;
    int cycles = 5;

    for (int i = 0; i < cycles; ++i) {
        for (int j = 0; j < entities_num; j++)
            ctx = create_one(ctx);

        /*
        for (int j = 0; j < entities_num / 2; j++)
            ctx = destroy_entt(ctx);
        // */

        //create_one(&ctx);
        ctx = destroy_entt(ctx);

        ctx = ecs_check_each(ctx);

    }

    set_free(ctx.set);
    de_ecs_destroy(ctx.r);
    return MUNIT_OK;
}

static MunitResult test_emplace_destroy(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();
    de_entity entts[1000] = {0};
    int entts_num = 0;

    for (int k = 0; k < 3; k++) {

        for (int x = 0; x < 50; x++) {
            for (int y = 0; y < 50; y++) {
                struct Cell *cell = create_cell(r, x, y, NULL);
                munit_assert_ptr_not_null(cell);
            }
        }

        for (int j = 0; j < 3; j++) {

            for (int i = 0; i < 5; i++) {

                for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
                        de_view_valid(&v); de_view_next(&v)) {

                    munit_assert(de_valid(r, de_view_entity(&v)));
                    struct Cell *c = de_view_get_safe(&v, cp_cell);
                    munit_assert_ptr_not_null(c);

                    c->moving = false;
                    c->from_x = rand() % 100 + 10;
                    c->from_y = rand() % 100 + 10;
                }

            }

            for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
                    de_view_valid(&v); de_view_next(&v)) {


                munit_assert(de_valid(r, de_view_entity(&v)));
                struct Cell *c = de_view_get_safe(&v, cp_cell);
                munit_assert_ptr_not_null(c);

                if (c->from_x == 10 || c->from_y == 10) {
                    if (verbose_print) 
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
            munit_assert(de_has(r, entts[i], cp_cell));
            de_destroy(r, entts[i]);
        }
    }
    */

    for (de_view v = de_create_view(r, 1, (de_cp_type[1]) { cp_cell }); 
        de_view_valid(&v); de_view_next(&v)) {

        munit_assert(de_valid(r, de_view_entity(&v)));
        struct Cell *c = de_view_get_safe(&v, cp_cell);
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

static MunitResult test_has(
    const MunitParameter params[], void* data
) {

    {
        de_ecs *r = de_ecs_make();
        de_entity e = de_create(r);
        de_emplace(r, e, cp_triple);
        //memset(tr, 1, sizeof(*tr));
        munit_assert(de_has(r, e, cp_triple) == true);
        munit_assert(de_has(r, e, cp_cell) == false);
        de_ecs_destroy(r);
    }

    {
        de_ecs *r = de_ecs_make();
        de_entity e = de_create(r);
        de_emplace(r, e, cp_triple);
        de_emplace(r, e, cp_cell);
        //memset(tr, 1, sizeof(*tr));
        munit_assert(de_has(r, e, cp_triple) == true);
        munit_assert(de_has(r, e, cp_cell) == true);
        de_ecs_destroy(r);
    }

    {
        de_ecs *r = de_ecs_make();
        de_entity e = de_create(r);
        de_emplace(r, e, cp_triple);
        de_emplace(r, e, cp_cell);
        //memset(tr, 1, sizeof(*tr));
        munit_assert(de_has(r, e, cp_triple) == true);
        munit_assert(de_has(r, e, cp_cell) == true);
        de_ecs_destroy(r);
    }

    {
        de_ecs *r = de_ecs_make();

        if (verbose_print)
            de_ecs_print(r);
        de_entity e1 = de_create(r);
        de_emplace(r, e1, cp_triple);

        if (verbose_print)
            de_ecs_print(r);
        de_entity e2 = de_create(r);
        de_emplace(r, e2, cp_cell);

        if (verbose_print)
            de_ecs_print(r);
        de_entity e3 = de_create(r);
        de_emplace(r, e3, cp_node);

        if (verbose_print)
            de_ecs_print(r);

        if (verbose_print) {
            de_storage_print(r, cp_triple);
            de_storage_print(r, cp_cell);
            de_storage_print(r, cp_node);
        }

        munit_assert(de_has(r, e1, cp_triple) == true);
        munit_assert(de_has(r, e1, cp_cell) == false);
        munit_assert(de_has(r, e2, cp_triple) == false);
        munit_assert(de_has(r, e2, cp_cell) == true);

        //de_destroy(r, e1);
        de_remove_all(r, e2);
        if (verbose_print)
            de_ecs_print(r);
        de_remove_all(r, e1);
        if (verbose_print)
            de_ecs_print(r);
        de_remove_all(r, e3);
        if (verbose_print)
            de_ecs_print(r);
        //de_destroy(r, e2);

        if (verbose_print) {
            de_storage_print(r, cp_triple);
            de_storage_print(r, cp_cell);
            de_storage_print(r, cp_node);
        }

        munit_assert(de_valid(r, e1));
        munit_assert(de_valid(r, e2));
        munit_assert(de_valid(r, e3));
        munit_assert((e1 != e2) && (e1 != e3));

        if (verbose_print)
            de_ecs_print(r);
        
        munit_assert(de_orphan(r, e1) == true);
        munit_assert(de_orphan(r, e2) == true);
        munit_assert(de_orphan(r, e3) == true);

        if (verbose_print)
            de_ecs_print(r);

        munit_assert(de_has(r, e1, cp_triple) == false);
        //munit_assert(de_has(r, e2, cp_cell) == false);
        //munit_assert(de_has(r, e3, cp_node) == false);

        de_ecs_destroy(r);
    }

    // XXX: Что если к одной сущности несколько раз цеплять компонент одного и
    // того же типа?
    {
        de_ecs *r = de_ecs_make();
        const int num = 100;
        de_entity ennts[num];
        for (int i = 0; i < num; i++) {
            de_entity e = de_create(r);
            de_emplace(r, e, cp_triple);
            ennts[i] = e;
        }

        for (int i = 0; i < num; i++) {
            de_entity e = ennts[i];
            munit_assert(de_has(r, e, cp_triple) == true);
            munit_assert(de_has(r, e, cp_cell) == false);
        }

        de_view_single v = de_create_view_single(r, cp_triple);
        for(; de_view_single_valid(&v); de_view_single_next(&v)) {
            de_entity e = de_view_single_entity(&v);
            munit_assert(de_has(r, e, cp_triple) == true);
            munit_assert(de_has(r, e, cp_cell) == false);
        }

        de_ecs_destroy(r);
    }



    return MUNIT_OK;
}

/*
static HTableAction iter_table_cell(
    const void *key, int key_len, void *value, int value_len, void *udata
) {
    de_entity e = *(de_entity*)key;
    printf("iter_table_cell: e %u\n", e);
    return HTABLE_ACTION_NEXT;
}
*/

static MunitResult test_view_get(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    size_t total_num = 1000;
    size_t idx = 0;
    size_t cell_num = 100;
    size_t triple_cell_num = 600;
    size_t triple_num = 300;
    munit_assert(total_num == triple_num + cell_num + triple_cell_num);

    de_entity *ennts_all = calloc(total_num, sizeof(de_entity));

    HTable *table_cell = htable_new(NULL);
    HTable *table_triple = htable_new(NULL);
    HTable *table_triple_cell = htable_new(NULL);

    // Создать случайное количество сущностей.
    // Проход при помощи de_view_single

    // Часть сущностей с компонентом cp_cell
    for (int i = 0; i < cell_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);
        struct Cell *cell = de_emplace(r, e, cp_cell);
        // {{{
        cell->from_x = rand() % 1000;
        cell->from_y = rand() % 1000;
        cell->to_x = rand() % 1000;
        cell->to_y = rand() % 1000;
        cell->value = rand() % 1000;
        // }}}
        htable_add(table_cell, &e, sizeof(e), cell, sizeof(*cell));
    }

    printf("--------------------------\n");
    //htable_each(table_cell, iter_table_cell, NULL);
    printf("table_cell count %d\n", htable_count(table_cell));
    printf("--------------------------\n");

    struct Couple {
        struct Cell     cell;
        struct Triple   triple;
    };

    // Часть сущностей с компонентами cp_cell и cp_triple
    for (int i = 0; i < triple_cell_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);

        struct Triple *triple = de_emplace(r, e, cp_triple);
        // {{{
        triple->dx = rand() % 1000;
        triple->dy = rand() % 1000;
        triple->dz = rand() % 1000;
        // }}}
        struct Cell *cell = de_emplace(r, e, cp_cell);
        // {{{
        cell->from_x = rand() % 1000;
        cell->from_y = rand() % 1000;
        cell->to_x = rand() % 1000;
        cell->to_y = rand() % 1000;
        cell->value = rand() % 1000;
        // }}}
        
        struct Couple x = {
            .cell = *cell,
            .triple = *triple,
        };

        htable_add(table_triple_cell, &e, sizeof(e), &x, sizeof(x));
    }

    // Часть сущностей с компонентом cp_triple
    for (int i = 0; i < triple_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);

        struct Triple *triple = de_emplace(r, e, cp_triple);
        // {{{
        triple->dx = rand() % 1000;
        triple->dy = rand() % 1000;
        triple->dz = rand() % 1000;
        // }}}
        htable_add(table_triple, &e, sizeof(e), triple, sizeof(*triple));
    }

    /*
    if (verbose_print)
        htable_print(table_triple);
        */

    {
        de_view v = de_create_view(r, 1, (de_cp_type[]){cp_cell});
        for (; de_view_valid(&v); de_view_next(&v)) {
            de_entity e = de_view_entity(&v);
            const struct Cell *cell1 = de_view_get(&v, cp_cell);
            size_t sz = sizeof(e);
            //printf("e %u\n", e);
            const struct Cell *cell2 = htable_get(table_cell, &e, sz, NULL);

            // Обработка cp_triple + cp_cell
            munit_assert_not_null(cell1);
            if (!cell2) {
                munit_assert(de_has(r, e, cp_triple));
            } else {
                //munit_assert(de_has(r, e, cp_triple));
                munit_assert(!memcmp(cell1, cell2, sizeof(*cell1)));
            }
        }
    }
    // */

    {
        de_view v = de_create_view(r, 1, (de_cp_type[]){cp_triple});
        for (;de_view_valid(&v); de_view_next(&v)) {
            de_entity e = de_view_entity(&v);
            const struct Triple *tr1 = de_view_get(&v, cp_triple);
            size_t sz = sizeof(e);
            const struct Triple *tr2 = htable_get(table_triple, &e, sz, NULL);
            munit_assert_not_null(tr1);

            // Обработка cp_triple + cp_cell
            if (!tr2) {
                munit_assert(de_has(r, e, cp_triple));
            } else {
                munit_assert(!memcmp(tr1, tr2, sizeof(*tr1)));
            }

        }
    }

    // Часть сущностей с компонентами cp_cell и cp_triple
    {
        de_view v = de_create_view(r, 2, (de_cp_type[]){cp_triple, cp_cell});
        for (;de_view_valid(&v); de_view_next(&v)) {
            de_entity e = de_view_entity(&v);
            const struct Triple *tr = de_view_get(&v, cp_triple);
            const struct Triple *cell = de_view_get(&v, cp_cell);
            size_t sz = sizeof(e);

            const struct Couple *x = htable_get(
                table_triple_cell, &e, sz, NULL
            );

            //munit_assert_not_null(tr);
            //munit_assert_not_null(cell);
            if (tr && cell) {
                munit_assert_not_null(x);
                munit_assert(!memcmp(tr, &x->triple, sizeof(*tr)));
                munit_assert(!memcmp(cell, &x->cell, sizeof(*cell)));
            } else if (!tr) {
                munit_assert(de_has(r, e, cp_cell));
            } else if (!cell) {
                munit_assert(de_has(r, e, cp_triple));
            }
        }
    }
    htable_free(table_cell);
    htable_free(table_triple);
    htable_free(table_triple_cell);
    free(ennts_all);
    //free(ennts_triple);
    //free(ennts_cell);
    //free(ennts_triple_cell);
    de_ecs_destroy(r);
    return MUNIT_OK;
}

static MunitResult test_view_single_get(
    const MunitParameter params[], void* data
) {
    de_ecs *r = de_ecs_make();

    size_t total_num = 1000;
    size_t idx = 0;
    size_t cell_num = 100;
    size_t triple_cell_num = 600;
    size_t triple_num = 300;
    munit_assert(total_num == triple_num + cell_num + triple_cell_num);

    de_entity *ennts_all = calloc(total_num, sizeof(de_entity));

    HTable *table_cell = htable_new(NULL);
    HTable *table_triple = htable_new(NULL);

    // Создать случайное количество сущностей.
    // Проход при помощи de_view_single

    // Часть сущностей с компонентом cp_cell
    for (int i = 0; i < cell_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);
        struct Cell *cell = de_emplace(r, e, cp_cell);
        // {{{
        cell->from_x = rand() % 1000;
        cell->from_y = rand() % 1000;
        cell->to_x = rand() % 1000;
        cell->to_y = rand() % 1000;
        cell->value = rand() % 1000;
        // }}}
        htable_add(table_cell, &e, sizeof(e), cell, sizeof(*cell));
    }

    printf("--------------------------\n");
    //htable_each(table_cell, iter_table_cell, NULL);
    printf("table_cell count %d\n", htable_count(table_cell));
    printf("--------------------------\n");

    struct Couple {
        struct Cell     cell;
        struct Triple   triple;
    };

    // Часть сущностей с компонентами cp_cell и cp_triple
    for (int i = 0; i < triple_cell_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);

        struct Triple *triple = de_emplace(r, e, cp_triple);
        // {{{
        triple->dx = rand() % 1000;
        triple->dy = rand() % 1000;
        triple->dz = rand() % 1000;
        // }}}
        struct Cell *cell = de_emplace(r, e, cp_cell);
        // {{{
        cell->from_x = rand() % 1000;
        cell->from_y = rand() % 1000;
        cell->to_x = rand() % 1000;
        cell->to_y = rand() % 1000;
        cell->value = rand() % 1000;
        // }}}
        
        /*
        struct Couple x = {
            .cell = *cell,
            .triple = *triple,
        };
        */

        //htable_add(table_triple_cell, &e, sizeof(e), &x, sizeof(x));
    }

    // Часть сущностей с компонентом cp_triple
    for (int i = 0; i < triple_num; ++i) {
        de_entity e = ennts_all[idx++] = de_create(r);

        struct Triple *triple = de_emplace(r, e, cp_triple);
        // {{{
        triple->dx = rand() % 1000;
        triple->dy = rand() % 1000;
        triple->dz = rand() % 1000;
        // }}}
        htable_add(table_triple, &e, sizeof(e), triple, sizeof(*triple));
    }

    /*
    if (verbose_print)
        htable_print(table_triple);
        */

    {
        de_view v = de_create_view(r, 1, (de_cp_type[]){cp_cell});
        for (; de_view_valid(&v); de_view_next(&v)) {
            de_entity e = de_view_entity(&v);
            const struct Cell *cell1 = de_view_get(&v, cp_cell);
            size_t sz = sizeof(e);
            //printf("e %u\n", e);
            const struct Cell *cell2 = htable_get(table_cell, &e, sz, NULL);

            // Обработка cp_triple + cp_cell
            munit_assert_not_null(cell1);
            if (!cell2) {
                munit_assert(de_has(r, e, cp_triple));
            } else {
                //munit_assert(de_has(r, e, cp_triple));
                munit_assert(!memcmp(cell1, cell2, sizeof(*cell1)));
            }
        }
    }
    // */

    {
        de_view v = de_create_view(r, 1, (de_cp_type[]){cp_triple});
        for (;de_view_valid(&v); de_view_next(&v)) {
            de_entity e = de_view_entity(&v);
            const struct Triple *tr1 = de_view_get(&v, cp_triple);
            size_t sz = sizeof(e);
            const struct Triple *tr2 = htable_get(table_triple, &e, sz, NULL);
            munit_assert_not_null(tr1);

            // Обработка cp_triple + cp_cell
            if (!tr2) {
                munit_assert(de_has(r, e, cp_triple));
            } else {
                munit_assert(!memcmp(tr1, tr2, sizeof(*tr1)));
            }

        }
    }

    htable_free(table_cell);
    htable_free(table_triple);
    free(ennts_all);
    de_ecs_destroy(r);
    return MUNIT_OK;
}

static MunitResult test_sparse_ecs(
    const MunitParameter params[], void* data
) {
    // проверка на добавление содержимого
    de_ecs *r = de_ecs_make();
    de_entity e1, e2, e3;

    e1 = de_create(r);

/*
   de_emplace

#0  de_sparse_emplace (s=0x60f000000068, e=0) at /home/nagolove/caustic/src/koh_destral_ecs.c:130
#1  0x0000555555661eba in de_storage_emplace (s=0x60f000000040, e=0)
#2  0x0000555555666c70 in de_emplace (r=0x60e000000040, e=0, cp_type=...)
#3  0x0000555555623c3c in test_sparse_ecs (params=0x0, data=0x555555912a60)
#4  0x00005555556288b4 in munit_test_runner_exec (runner=0x7ffff4e00080, 

$2 = {sparse = 0x632000000800, sparse_size = 1, sparse_cap = 20000, dense = 0x632000018800, dense_size = 1, 
  dense_cap = 20000, initial_cap = 20000}

$2 = {sparse = 0x632000000800, sparse_size = 1, sparse_cap = 20000, dense = 0x632000018800, dense_size = 1, 
  dense_cap = 20000, initial_cap = 20000}

 */

    de_emplace(r, e1, cp_cell);

    munit_assert_not_null(de_get(r, e1, cp_cell));
    de_destroy(r, e1);
    munit_assert(!de_valid(r, e1));

    e2 = de_create(r);
    de_emplace(r, e2, cp_cell);
    munit_assert_not_null(de_get(r, e2, cp_cell));
    de_destroy(r, e2);
    munit_assert(!de_valid(r, e2));

    e3 = de_create(r);
    de_emplace(r, e3, cp_cell);
    munit_assert_not_null(de_get(r, e3, cp_cell));
    de_destroy(r, e3);
    munit_assert(!de_valid(r, e3));

    de_ecs_destroy(r);

    return MUNIT_OK;
}

static MunitResult test_sparse_1(
    const MunitParameter params[], void* data
) {
    // проверка на добавление содержимого
    de_sparse s = {};
    de_entity e1, e2, e3;

    e1 = de_make_entity(
        (de_entity_id) { .id = 100, }, (de_entity_ver) { .ver = 0 }
    );
    e2 = de_make_entity(
        (de_entity_id) { .id = 10, }, (de_entity_ver) { .ver = 0 }
    );
    e3 = de_make_entity(
        (de_entity_id) { .id = 11, }, (de_entity_ver) { .ver = 0 }
    );

    de_sparse_init(&s, 10);

    de_sparse_emplace(&s, e1);
    munit_assert(de_sparse_contains(&s, e1));

    de_sparse_emplace(&s, e2);
    munit_assert(de_sparse_contains(&s, e2));

    de_sparse_emplace(&s, e3);
    munit_assert(de_sparse_contains(&s, e3));

    de_sparse_destroy(&s);

    return MUNIT_OK;
}

static MunitResult test_sparse_2_non_seq_idx(
    const MunitParameter params[], void* data
) {
    // проверка на добавление и удаление содержимого
    de_sparse s = {};
    de_entity e1, e2, e3;

    e1 = de_make_entity(
        (de_entity_id) { .id = 1, }, (de_entity_ver) { .ver = 0 }
    );
    e2 = de_make_entity(
        (de_entity_id) { .id = 0, }, (de_entity_ver) { .ver = 0 }
    );
    e3 = de_make_entity(
        (de_entity_id) { .id = 3, }, (de_entity_ver) { .ver = 0 }
    );

    de_sparse_init(&s, 1);

    // Проверка, можно-ли добавлять и удалять циклически
    for (int i = 0; i < 10; i++) {
        //printf("test_sparse_2:%s\n", de_sparse_contains(&s, e1) ? "true" : "false");
        de_sparse_emplace(&s, e1);
        de_sparse_emplace(&s, e2);
        de_sparse_emplace(&s, e3);

        de_sparse_remove(&s, e1);
        /*de_sparse_remove(&s, e1);*/
        /*de_sparse_remove(&s, e1);*/

        // XXX: Почему элемент не удаляется?
        //munit_assert(!de_sparse_contains(&s, e1));
        //printf("test_sparse_2:%s\n", de_sparse_contains(&s, e1) ? "true" : "false");
        munit_assert(!de_sparse_contains(&s, e1));

        de_sparse_remove(&s, e2);
        //munit_assert(!de_sparse_contains(&s, e2));
        munit_assert(!de_sparse_contains(&s, e2));

        de_sparse_remove(&s, e3);
        //munit_assert(!de_sparse_contains(&s, e3));
        munit_assert(!de_sparse_contains(&s, e3));
    }

    de_sparse_destroy(&s);

    return MUNIT_OK;
}

static MunitResult test_sparse_2(
    const MunitParameter params[], void* data
) {
    // проверка на добавление и удаление содержимого
    de_sparse s = {};
    de_entity e1, e2, e3;

    e1 = de_make_entity(
        (de_entity_id) { .id = 1, }, (de_entity_ver) { .ver = 0 }
    );
    e2 = de_make_entity(
        (de_entity_id) { .id = 2, }, (de_entity_ver) { .ver = 0 }
    );
    e3 = de_make_entity(
        (de_entity_id) { .id = 3, }, (de_entity_ver) { .ver = 0 }
    );

    de_sparse_init(&s, 1);

    // Проверка, можно-ли добавлять и удалять циклически
    for (int i = 0; i < 10; i++) {
        //printf("test_sparse_2:%s\n", de_sparse_contains(&s, e1) ? "true" : "false");
        de_sparse_emplace(&s, e1);
        de_sparse_emplace(&s, e2);
        de_sparse_emplace(&s, e3);

        //printf("\ntest_sparse_2: de_sparse_index %zu\n", de_sparse_index(&s, e1));
        //munit_assert(de_sparse_index(&s, e1))
        de_sparse_remove(&s, e1);
        //de_sparse_remove(&s, e1);
        //de_sparse_remove(&s, e1);

        // XXX: Почему элемент не удаляется?
        //munit_assert(!de_sparse_contains(&s, e1));
        //printf("test_sparse_2: de_sparse_index %zu\n", de_sparse_index(&s, e1));
        //printf("test_sparse_2:%s\n", de_sparse_contains(&s, e1) ? "true" : "false");

        munit_assert(!de_sparse_contains(&s, e1));

        de_sparse_remove(&s, e2);
        //munit_assert(!de_sparse_contains(&s, e2));
        munit_assert(!de_sparse_contains(&s, e2));

        de_sparse_remove(&s, e3);
        //munit_assert(!de_sparse_contains(&s, e3));
        munit_assert(!de_sparse_contains(&s, e3));
    }

    de_sparse_destroy(&s);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {

    {
      (char*) "/sparse_ecs",
      test_sparse_ecs,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },


    {
      (char*) "/sparse_1",
      test_sparse_1,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },

    {
      (char*) "/test_sparse_2_non_seq_idx",
      test_sparse_2_non_seq_idx,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },


    {
      (char*) "/sparse_2",
      test_sparse_2,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },


    // FIXME:
    {
      (char*) "/has",
      test_has,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },
    // */

    // FIXME:
    {
      (char*) "/view_get",
      test_view_get,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL
    },

    // FIXME:
      {
        (char*) "/view_single_get",
        test_view_single_get,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
      },
    // */

  {
    (char*) "/try_get_none_existing_component",
    test_try_get_none_existing_component,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/destroy_one_random",
    test_destroy_one_random,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/destroy_one",
    test_destroy_one,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/destroy_zero",
    test_destroy_zero,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

   //FIXME:
  {
    (char*) "/destroy",
    test_destroy,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },
  // */

  {
    (char*) "/emplace_destroy",
    test_emplace_destroy,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/ecs_clone_mono",
    test_ecs_clone_mono,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/ecs_clone_multi",
    test_ecs_clone_multi,
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
