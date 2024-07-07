#include <math.h>

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$pow(ASPL_OBJECT_TYPE* base, ASPL_OBJECT_TYPE* exponent) {
    ASPL_OBJECT_TYPE baseObj = (ASPL_OBJECT_TYPE)*base;
    ASPL_OBJECT_TYPE exponentObj = (ASPL_OBJECT_TYPE)*exponent;
    return ASPL_DOUBLE_LITERAL(pow(ASPL_ACCESS(baseObj).value.float64, ASPL_ACCESS(exponentObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$floor(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(floor(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$round(ASPL_OBJECT_TYPE* value, ASPL_OBJECT_TYPE* digits) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    ASPL_OBJECT_TYPE digitsObj = (ASPL_OBJECT_TYPE)*digits;
    return ASPL_DOUBLE_LITERAL(round(ASPL_ACCESS(valueObj).value.float64 * pow(10, ASPL_ACCESS(digitsObj).value.integer32)) / pow(10, ASPL_ACCESS(digitsObj).value.integer32)); // TODO: Check this
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$sin(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(sin(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$asin(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(asin(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$cos(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(cos(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$acos(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(acos(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$tan(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(tan(ASPL_ACCESS(valueObj).value.float64));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_math$atan(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    return ASPL_DOUBLE_LITERAL(atan(ASPL_ACCESS(valueObj).value.float64));
}