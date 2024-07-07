#include <stdlib.h>
#include <time.h>

int aspl_util_rand$generate_int() {
    static char inited = 0;
    if (!inited) {
        srand(time(NULL));
        inited = 1;
    }
    return rand();
}

// TODO: These functions are not uniform and can only generate numbers up to the signed 32-bit integer limit

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_random$range$byte(ASPL_OBJECT_TYPE* min, ASPL_OBJECT_TYPE* max)
{
    ASPL_OBJECT_TYPE minObj = (ASPL_OBJECT_TYPE)*min;
    ASPL_OBJECT_TYPE maxObj = (ASPL_OBJECT_TYPE)*max;
    return ASPL_INT_LITERAL(aspl_util_rand$generate_int() % (ASPL_ACCESS(maxObj).value.integer8 - ASPL_ACCESS(minObj).value.integer8) + ASPL_ACCESS(minObj).value.integer8);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_random$range$int(ASPL_OBJECT_TYPE* min, ASPL_OBJECT_TYPE* max)
{
    ASPL_OBJECT_TYPE minObj = (ASPL_OBJECT_TYPE)*min;
    ASPL_OBJECT_TYPE maxObj = (ASPL_OBJECT_TYPE)*max;
    return ASPL_INT_LITERAL(aspl_util_rand$generate_int() % (ASPL_ACCESS(maxObj).value.integer32 - ASPL_ACCESS(minObj).value.integer32) + ASPL_ACCESS(minObj).value.integer32);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_random$range$long(ASPL_OBJECT_TYPE* min, ASPL_OBJECT_TYPE* max)
{
    ASPL_OBJECT_TYPE minObj = (ASPL_OBJECT_TYPE)*min;
    ASPL_OBJECT_TYPE maxObj = (ASPL_OBJECT_TYPE)*max;
    return ASPL_LONG_LITERAL(aspl_util_rand$generate_int() % (ASPL_ACCESS(maxObj).value.integer64 - ASPL_ACCESS(minObj).value.integer64) + ASPL_ACCESS(minObj).value.integer64);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_random$range$float(ASPL_OBJECT_TYPE* min, ASPL_OBJECT_TYPE* max)
{
    ASPL_OBJECT_TYPE minObj = (ASPL_OBJECT_TYPE)*min;
    ASPL_OBJECT_TYPE maxObj = (ASPL_OBJECT_TYPE)*max;
    return ASPL_FLOAT_LITERAL((float)aspl_util_rand$generate_int() / ((float)RAND_MAX / (ASPL_ACCESS(maxObj).value.float32 - ASPL_ACCESS(minObj).value.float32)) + ASPL_ACCESS(minObj).value.float32);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_random$range$double(ASPL_OBJECT_TYPE* min, ASPL_OBJECT_TYPE* max)
{
    ASPL_OBJECT_TYPE minObj = (ASPL_OBJECT_TYPE)*min;
    ASPL_OBJECT_TYPE maxObj = (ASPL_OBJECT_TYPE)*max;
    return ASPL_DOUBLE_LITERAL((double)aspl_util_rand$generate_int() / ((double)RAND_MAX / (ASPL_ACCESS(maxObj).value.float64 - ASPL_ACCESS(minObj).value.float64)) + ASPL_ACCESS(minObj).value.float64);
}