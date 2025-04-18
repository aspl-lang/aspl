#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef HASHMAP_PREFIX
#error "HASHMAP_PREFIX must be defined"
#endif
#define HASHMAP_CONCAT(a, b) a##_##b
#define _HASHMAP_FUNC_NAME(x, name) HASHMAP_CONCAT(x, name)
#define _HASHMAP_STRUCT_NAME(x, name) HASHMAP_CONCAT(x, name)
#define HASHMAP_FUNC_NAME(name) _HASHMAP_FUNC_NAME(HASHMAP_PREFIX, name)
#define HASHMAP_STRUCT_NAME(name) _HASHMAP_FUNC_NAME(HASHMAP_PREFIX, name)

#ifndef HASHMAP_KEY_TYPE
#error "HASHMAP_KEY_TYPE must be defined"
#endif
#ifndef HASHMAP_KEY_NULL_VALUE
#error "HASHMAP_KEY_NULL_VALUE must be defined"
#endif
#ifndef HASHMAP_VALUE_TYPE
#error "HASHMAP_VALUE_TYPE must be defined"
#endif
#ifndef HASHMAP_VALUE_NULL_VALUE
#error "HASHMAP_VALUE_NULL_VALUE must be defined"
#endif

#ifndef HASHMAP_MALLOC
#define HASHMAP_MALLOC malloc
#endif
#ifndef HASHMAP_FREE
#define HASHMAP_FREE free
#endif
#ifndef HASHMAP_REALLOC
#define HASHMAP_REALLOC realloc
#endif

unsigned int HASHMAP_FUNC_NAME(hash_key)(HASHMAP_KEY_TYPE key);
int HASHMAP_FUNC_NAME(equals_key)(HASHMAP_KEY_TYPE a, HASHMAP_KEY_TYPE b);
unsigned int HASHMAP_FUNC_NAME(hash_value)(HASHMAP_VALUE_TYPE value);
int HASHMAP_FUNC_NAME(equals_value)(HASHMAP_VALUE_TYPE a, HASHMAP_VALUE_TYPE b);

typedef struct HASHMAP_STRUCT_NAME(Pair)
{
    HASHMAP_KEY_TYPE key;
    HASHMAP_VALUE_TYPE value;
} HASHMAP_STRUCT_NAME(Pair);

typedef struct HASHMAP_STRUCT_NAME(Bucket)
{
    HASHMAP_STRUCT_NAME(Pair)** pairs;
    int num_pairs;
} HASHMAP_STRUCT_NAME(Bucket);

typedef struct HASHMAP_STRUCT_NAME(HashMap)
{
    int pair_index;
    HASHMAP_STRUCT_NAME(Pair)** pairs;
    HASHMAP_STRUCT_NAME(Bucket)** buckets;
    int buckets_length;
    int len;
} HASHMAP_STRUCT_NAME(HashMap);

typedef struct HASHMAP_STRUCT_NAME(HashMapConfig)
{
    int initial_capacity;
} HASHMAP_STRUCT_NAME(HashMapConfig);

void HASHMAP_FUNC_NAME(hashmap_rehash)(HASHMAP_STRUCT_NAME(HashMap)* m, int new_capacity);

HASHMAP_STRUCT_NAME(HashMap)* HASHMAP_FUNC_NAME(new_hashmap)(HASHMAP_STRUCT_NAME(HashMapConfig) config)
{
    if (config.initial_capacity <= 0)
    {
        printf("initial_capacity of hashmap must be greater than 0\n");
        return NULL;
    }

    HASHMAP_STRUCT_NAME(Bucket)** buckets = (HASHMAP_STRUCT_NAME(Bucket)**)HASHMAP_MALLOC(config.initial_capacity * sizeof(HASHMAP_STRUCT_NAME(Bucket)*));
    for (int i = 0; i < config.initial_capacity; i++)
    {
        buckets[i] = (HASHMAP_STRUCT_NAME(Bucket)*)HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(Bucket)));
        buckets[i]->pairs = NULL;
        buckets[i]->num_pairs = 0;
    }

    HASHMAP_STRUCT_NAME(HashMap)* hashmap = (HASHMAP_STRUCT_NAME(HashMap)*)HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(HashMap)));
    hashmap->pair_index = 0;
    hashmap->pairs = HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(Pair)*) * config.initial_capacity);
    hashmap->buckets = buckets;
    hashmap->buckets_length = config.initial_capacity;
    hashmap->len = 0;

    return hashmap;
}

int HASHMAP_FUNC_NAME(hashmap_contains_key)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_KEY_TYPE key)
{
    if (m->len == 0)
    {
        return 0;
    }

    unsigned int key_hash = HASHMAP_STRUCT_NAME(hash_key)(key);
    HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[key_hash % m->buckets_length];
    if (bucket == NULL)
    {
        return 0;
    }

    for (int i = 0; i < bucket->num_pairs; i++)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[i];
        if (HASHMAP_STRUCT_NAME(equals_key)(pair->key, key))
        {
            return 1;
        }
    }

    return 0;
}

int HASHMAP_FUNC_NAME(hashmap_contains_value)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_VALUE_TYPE value)
{
    if (m->len == 0)
    {
        return 0;
    }

    for (int i = 0; i < m->buckets_length; i++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[i];
        for (int j = 0; j < bucket->num_pairs; j++)
        {
            HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[j];
            if (HASHMAP_STRUCT_NAME(equals_value)(pair->value, value))
            {
                return 1;
            }
        }
    }

    return 0;
}

HASHMAP_VALUE_TYPE HASHMAP_FUNC_NAME(hashmap_get_value)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_KEY_TYPE key)
{
    if (m->len == 0)
    {
        return HASHMAP_VALUE_NULL_VALUE;
    }

    unsigned int key_hash = HASHMAP_STRUCT_NAME(hash_key)(key);
    HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[key_hash % m->buckets_length];
    if (bucket == NULL)
    {
        return HASHMAP_VALUE_NULL_VALUE;
    }

    for (int i = 0; i < bucket->num_pairs; i++)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[i];
        if (HASHMAP_STRUCT_NAME(equals_key)(pair->key, key))
        {
            return pair->value;
        }
    }

    return HASHMAP_VALUE_NULL_VALUE;
}

HASHMAP_KEY_TYPE HASHMAP_FUNC_NAME(hashmap_get_key)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_VALUE_TYPE value)
{
    if (m->len == 0)
    {
        return HASHMAP_KEY_NULL_VALUE;
    }

    for (int i = 0; i < m->buckets_length; i++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[i];
        for (int j = 0; j < bucket->num_pairs; j++)
        {
            HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[j];
            if (HASHMAP_STRUCT_NAME(equals_value)(pair->value, value))
            {
                return pair->key;
            }
        }
    }

    return HASHMAP_KEY_NULL_VALUE;
}

void HASHMAP_FUNC_NAME(hashmap_set)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_KEY_TYPE key, HASHMAP_VALUE_TYPE value)
{
    unsigned int key_hash = HASHMAP_STRUCT_NAME(hash_key)(key);
    int index = key_hash % m->buckets_length;

    HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[index];
    for (int i = 0; i < bucket->num_pairs; i++)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[i];
        if (HASHMAP_STRUCT_NAME(equals_key)(pair->key, key))
        {
            pair->value = value;
            return;
        }
    }

    HASHMAP_STRUCT_NAME(Pair)* pair = (HASHMAP_STRUCT_NAME(Pair)*)HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(Pair)));
    pair->key = key;
    pair->value = value;

    HASHMAP_STRUCT_NAME(Pair)** new_pairs = (HASHMAP_STRUCT_NAME(Pair)**)HASHMAP_REALLOC(bucket->pairs, (bucket->num_pairs + 1) * sizeof(HASHMAP_STRUCT_NAME(Pair)*));
    new_pairs[bucket->num_pairs] = pair;
    bucket->pairs = new_pairs;
    bucket->num_pairs++;

    HASHMAP_STRUCT_NAME(Pair)** new_all_pairs = (HASHMAP_STRUCT_NAME(Pair)**)HASHMAP_REALLOC(m->pairs, (m->len + 1) * sizeof(HASHMAP_STRUCT_NAME(Pair)*));
    new_all_pairs[m->len] = pair;
    m->pairs = new_all_pairs;

    m->len++;

    const float load_factor_threshold = 0.75;
    if ((float)m->len / m->buckets_length > load_factor_threshold)
    {
        HASHMAP_FUNC_NAME(hashmap_rehash)(m, m->buckets_length * 2);
    }
}

int HASHMAP_FUNC_NAME(hashmap_remove)(HASHMAP_STRUCT_NAME(HashMap)* m, HASHMAP_KEY_TYPE key)
{
    unsigned int key_hash = HASHMAP_STRUCT_NAME(hash_key)(key);
    int index = key_hash % m->buckets_length;

    HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[index];
    if (bucket == NULL)
    {
        return 0;
    }

    for (int i = 0; i < bucket->num_pairs; i++)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[i];
        if (HASHMAP_STRUCT_NAME(equals_key)(pair->key, key))
        {
            for (int j = 0; j < m->len; j++)
            {
                if (m->pairs[j] == pair)
                {
                    for (int k = j; k < m->len - 1; k++)
                    {
                        m->pairs[k] = m->pairs[k + 1];
                    }
                    break;
                }
            }
            m->len--;
            if (bucket->num_pairs == 1)
            {
                HASHMAP_FREE(bucket->pairs);
                bucket->pairs = NULL;
            }
            else
            {
                HASHMAP_STRUCT_NAME(Pair)** new_pairs = (HASHMAP_STRUCT_NAME(Pair)**)HASHMAP_MALLOC((bucket->num_pairs - 1) * sizeof(HASHMAP_STRUCT_NAME(Pair)*));
                for (int j = 0, k = 0; j < bucket->num_pairs; j++)
                {
                    if (bucket->pairs[j] != pair)
                    {
                        new_pairs[k] = bucket->pairs[j];
                        k++;
                    }
                }
                HASHMAP_FREE(bucket->pairs);
                bucket->pairs = new_pairs;
            }
            bucket->num_pairs--;
            HASHMAP_FREE(pair);

            const float load_factor_threshold = 0.25;
            if ((float)m->len / m->buckets_length < load_factor_threshold)
            {
                int new_capacity = m->buckets_length / 2;
                if (new_capacity < 1)
                {
                    new_capacity = 1;
                }
                HASHMAP_FUNC_NAME(hashmap_rehash)(m, new_capacity);
            }

            return 1;
        }
    }

    return 0;
}

void HASHMAP_FUNC_NAME(hashmap_rehash)(HASHMAP_STRUCT_NAME(HashMap)* m, int new_capacity)
{
    HASHMAP_STRUCT_NAME(Bucket)** old_buckets = m->buckets;
    HASHMAP_STRUCT_NAME(Pair)** old_pairs = m->pairs;
    int old_buckets_length = m->buckets_length;
    int old_length = m->len;

    m->pairs = (HASHMAP_STRUCT_NAME(Pair)**)HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(Pair)*));
    m->buckets = (HASHMAP_STRUCT_NAME(Bucket)**)HASHMAP_MALLOC(new_capacity * sizeof(HASHMAP_STRUCT_NAME(Bucket)*));
    m->buckets_length = new_capacity;
    m->len = 0;

    for (int i = 0; i < new_capacity; i++)
    {
        m->buckets[i] = (HASHMAP_STRUCT_NAME(Bucket)*)HASHMAP_MALLOC(sizeof(HASHMAP_STRUCT_NAME(Bucket)));
        m->buckets[i]->pairs = NULL;
        m->buckets[i]->num_pairs = 0;
    }

    for (int i = 0; i < old_length; i++)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = old_pairs[i];
        HASHMAP_FUNC_NAME(hashmap_set)(m, pair->key, pair->value);
    }

    for (int i = 0; i < old_length; i++)
    {
        HASHMAP_FREE(old_pairs[i]);
    }

    HASHMAP_FREE(old_pairs);

    for (int i = 0; i < old_buckets_length; i++)
    {
        HASHMAP_FREE(old_buckets[i]->pairs);
        HASHMAP_FREE(old_buckets[i]);
    }

    HASHMAP_FREE(old_buckets);
}

unsigned int HASHMAP_FUNC_NAME(hashmap_hash)(HASHMAP_STRUCT_NAME(HashMap)* m)
{
    unsigned int i = 0;
    for (int j = 0; j < m->buckets_length; j++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[j];
        for (int k = 0; k < bucket->num_pairs; k++)
        {
            HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[k];
            i += i * 31 + HASHMAP_STRUCT_NAME(hash_key)(pair->key);
            i += i * 31 + HASHMAP_STRUCT_NAME(hash_value)(pair->value);
        }
    }
    return i;
}

int HASHMAP_FUNC_NAME(hashmap_equals)(HASHMAP_STRUCT_NAME(HashMap)* a, HASHMAP_STRUCT_NAME(HashMap)* b)
{
    if (a->len != b->len)
    {
        return 0;
    }

    for (int i = 0; i < a->buckets_length; i++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = a->buckets[i];
        for (int j = 0; j < bucket->num_pairs; j++)
        {
            HASHMAP_STRUCT_NAME(Pair)* pair = bucket->pairs[j];
            if (!HASHMAP_FUNC_NAME(hashmap_contains_key)(b, pair->key))
            {
                return 0;
            }
            HASHMAP_VALUE_TYPE b_value = HASHMAP_FUNC_NAME(hashmap_get_value)(b, pair->key);
            if (b_value == HASHMAP_VALUE_NULL_VALUE || !HASHMAP_STRUCT_NAME(equals_value)(pair->value, b_value))
            {
                return 0;
            }
        }
    }

    return 1;
}

HASHMAP_STRUCT_NAME(HashMap)* HASHMAP_FUNC_NAME(hashmap_reverse)(HASHMAP_STRUCT_NAME(HashMap)* m)
{
    HASHMAP_STRUCT_NAME(HashMap)* reverse = HASHMAP_FUNC_NAME(new_hashmap)((HASHMAP_STRUCT_NAME(HashMapConfig)) { m->buckets_length });

    for (int i = m->len - 1; i >= 0; i--)
    {
        HASHMAP_STRUCT_NAME(Pair)* pair = m->pairs[i];
        HASHMAP_FUNC_NAME(hashmap_set)
            (reverse, pair->key, pair->value);
    }

    return reverse;
}

HASHMAP_STRUCT_NAME(HashMap)* HASHMAP_FUNC_NAME(hashmap_clear)(HASHMAP_STRUCT_NAME(HashMap)* m)
{
    for (int i = 0; i < m->buckets_length; i++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[i];
        for (int j = 0; j < bucket->num_pairs; j++)
        {
            HASHMAP_FREE(bucket->pairs[j]);
        }
        HASHMAP_FREE(bucket->pairs);
        bucket->pairs = NULL;
        bucket->num_pairs = 0;
    }

    HASHMAP_STRUCT_NAME(Pair)** new_pairs = (HASHMAP_STRUCT_NAME(Pair)**)HASHMAP_REALLOC(m->pairs, sizeof(HASHMAP_STRUCT_NAME(Pair)*));
    m->pairs = new_pairs;
    m->len = 0;

    return m;
}

HASHMAP_STRUCT_NAME(Pair)* HASHMAP_FUNC_NAME(hashmap_next)(HASHMAP_STRUCT_NAME(HashMap)* m)
{
    if (m->len == 0)
    {
        return NULL;
    }

    if (m->pair_index == m->len)
    {
        m->pair_index = 0;
        return NULL;
    }

    HASHMAP_STRUCT_NAME(Pair)* pair = m->pairs[m->pair_index];
    m->pair_index++;
    return pair;
}

void HASHMAP_FUNC_NAME(hashmap_free_hashmap)(HASHMAP_STRUCT_NAME(HashMap)* m)
{
    for (int i = 0; i < m->buckets_length; i++)
    {
        HASHMAP_STRUCT_NAME(Bucket)* bucket = m->buckets[i];
        for (int j = 0; j < bucket->num_pairs; j++)
        {
            HASHMAP_FREE(bucket->pairs[j]);
        }
        HASHMAP_FREE(bucket->pairs);
        HASHMAP_FREE(bucket);
    }
    HASHMAP_FREE(m->buckets);
    HASHMAP_FREE(m->pairs);
    HASHMAP_FREE(m);
}
