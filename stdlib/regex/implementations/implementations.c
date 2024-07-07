#define REGEXP_MALLOC ASPL_MALLOC
#define REGEXP_FREE ASPL_FREE
#define accept regexp_accept
#include "thirdparty/regexp/regexp.c"
#undef accept

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_regex$match(ASPL_OBJECT_TYPE* regex, ASPL_OBJECT_TYPE* string)
{
    const char* error;
    Reprog* p = regcomp(ASPL_ACCESS(*regex).value.string->str, 0, &error);
    // TODO: Handle errors
    Resub m;
    if (!regexec(p, ASPL_ACCESS(*string).value.string->str, &m, 0))
    {
        for (int i = 0; i < m.nsub; ++i)
        {
            int n = m.sub[i].ep - m.sub[i].sp;
            char* s = ASPL_MALLOC(n + 1);
            memcpy(s, m.sub[i].sp, n);
            s[n] = '\0';
            ASPL_OBJECT_TYPE* list = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 3);
            list[0] = ASPL_INT_LITERAL(m.sub[i].sp - ASPL_ACCESS(*string).value.string->str);
            list[1] = ASPL_INT_LITERAL(m.sub[i].ep - ASPL_ACCESS(*string).value.string->str);
            list[2] = ASPL_STRING_LITERAL(s);
            ASPL_OBJECT_TYPE result = ASPL_LIST_LITERAL("list<integer|string>", 20, list, 3);
            ASPL_FREE(list);
            return result;
        }
    }
    return ASPL_NULL();
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_regex$find_first(ASPL_OBJECT_TYPE* regex, ASPL_OBJECT_TYPE* string)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_regex$replace_first(ASPL_OBJECT_TYPE* regex, ASPL_OBJECT_TYPE* replacement, ASPL_OBJECT_TYPE* string)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_regex$find_all(ASPL_OBJECT_TYPE* regex, ASPL_OBJECT_TYPE* string)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_regex$replace_all(ASPL_OBJECT_TYPE* regex, ASPL_OBJECT_TYPE* replacement, ASPL_OBJECT_TYPE* string)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}