#define Window sqlite3Window
#include "thirdparty/sqlite/sqlite3.c"
#undef Window

// TODO: Tell SQLite to use GC Boehm

const char* ASPL_util_sqlite$clone_string(char* original) {
    size_t length = strlen(original);
    const char* copy = ASPL_MALLOC(length);
    memcpy((void*)copy, (const void*)original, length);
    return copy;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$database$open(ASPL_OBJECT_TYPE* file) {
    sqlite3* db;
    int rc = sqlite3_open(ASPL_ACCESS(*file).value.string->str, &db);
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(db);
        sqlite3_close(db);
        ASPL_PANIC("Cannot open database: %s", errmsg);
        return ASPL_UNINITIALIZED;
    }
    return ASPL_HANDLE_LITERAL(db);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$database$prepare(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* query) {
    sqlite3* db = ASPL_ACCESS(*handle).value.handle;
    const char* sql = ASPL_ACCESS(*query).value.string->str;
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        const char* errmsg = ASPL_util_sqlite$clone_string((char*)sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        ASPL_PANIC("Cannot prepare statement: %s", errmsg);
        return ASPL_UNINITIALIZED;
    }
    return ASPL_HANDLE_LITERAL(stmt);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$database$query(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* query) {
    sqlite3* db = ASPL_ACCESS(*handle).value.handle;
    const char* sql = ASPL_ACCESS(*query).value.string->str;
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        const char* errmsg = ASPL_util_sqlite$clone_string((char*)sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        ASPL_PANIC("Cannot prepare statement: %s", errmsg);
        return ASPL_UNINITIALIZED;
    }
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
    return ASPL_HANDLE_LITERAL(stmt);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$database$close(ASPL_OBJECT_TYPE* handle) {
    sqlite3_close(ASPL_ACCESS(*handle).value.handle);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$statement$bind(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* parameter, ASPL_OBJECT_TYPE* value) {
    sqlite3_stmt* stmt = ASPL_ACCESS(*handle).value.handle;
    const char* param_name = ASPL_ACCESS(*parameter).value.string->str;
    int index = sqlite3_bind_parameter_index(stmt, param_name);
    if (index == 0) {
        ASPL_PANIC("Parameter not found: %s", param_name);
        return ASPL_UNINITIALIZED;
    }
    int rc;
    switch (ASPL_ACCESS(*value).kind) {
    case ASPL_OBJECT_KIND_NULL:
        rc = sqlite3_bind_null(stmt, index);
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        rc = sqlite3_bind_int(stmt, index, ASPL_ACCESS(*value).value.integer32);
        break;
    case ASPL_OBJECT_KIND_LONG:
        rc = sqlite3_bind_int(stmt, index, ASPL_ACCESS(*value).value.integer64);
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        rc = sqlite3_bind_double(stmt, index, ASPL_ACCESS(*value).value.float32);
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        rc = sqlite3_bind_double(stmt, index, ASPL_ACCESS(*value).value.float64);
        break;
    case ASPL_OBJECT_KIND_STRING:
        rc = sqlite3_bind_text(stmt, index, ASPL_ACCESS(*value).value.string->str, -1, SQLITE_TRANSIENT);
        break;
    case ASPL_OBJECT_KIND_LIST:
    {
        char* blob = ASPL_MALLOC(ASPL_ACCESS(*value).value.list->length);
        for (int i = 0; i < ASPL_ACCESS(*value).value.list->length; i++) {
            blob[i] = ASPL_ACCESS(ASPL_LIST_GET(*value, ASPL_INT_LITERAL(i))).value.integer8;
        }
        rc = sqlite3_bind_blob(stmt, index, blob, ASPL_ACCESS(*value).value.list->length, SQLITE_TRANSIENT);
        break;
    }
    default:
        ASPL_PANIC("Unsupported bind value kind: %d", ASPL_ACCESS(*value).kind);
        return ASPL_UNINITIALIZED;
    }
    if (rc != SQLITE_OK) {
        const char* errmsg = ASPL_util_sqlite$clone_string((char*)sqlite3_errmsg(sqlite3_db_handle(stmt)));
        sqlite3_finalize(stmt);
        sqlite3_close(sqlite3_db_handle(stmt));
        ASPL_PANIC("Cannot bind parameter: %s", errmsg);
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$statement$execute(ASPL_OBJECT_TYPE* handle) {
    sqlite3_stmt* stmt = ASPL_ACCESS(*handle).value.handle;
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
    return ASPL_HANDLE_LITERAL(stmt);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$response$fetch_row(ASPL_OBJECT_TYPE* handle) {
    sqlite3_stmt* stmt = ASPL_ACCESS(*handle).value.handle;
    int rc;
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int column_count = sqlite3_column_count(stmt);
        ASPL_OBJECT_TYPE* columns = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * column_count * 2);
        for (int j = 0; j < column_count; j++) {
            columns[j * 2] = ASPL_STRING_LITERAL(sqlite3_column_name(stmt, j));
            int type = sqlite3_column_type(stmt, j);
            switch (type) {
            case SQLITE_NULL:
                columns[j * 2 + 1] = ASPL_NULL();
                break;
            case SQLITE_INTEGER:
                columns[j * 2 + 1] = ASPL_LONG_LITERAL(sqlite3_column_int64(stmt, j));
                break;
            case SQLITE_FLOAT:
                columns[j * 2 + 1] = ASPL_DOUBLE_LITERAL(sqlite3_column_double(stmt, j));
                break;
            case SQLITE_TEXT:
                columns[j * 2 + 1] = ASPL_STRING_LITERAL((const char*)sqlite3_column_text(stmt, j));
                break;
            case SQLITE_BLOB:
            {
                const char* blob = sqlite3_column_blob(stmt, j);
                ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * sqlite3_column_bytes(stmt, j));
                for (int i = 0; i < sqlite3_column_bytes(stmt, j); i++) {
                    array[i] = ASPL_BYTE_LITERAL(blob[i]);
                }
                columns[j * 2 + 1] = ASPL_LIST_LITERAL("list<byte>", 10, array, sqlite3_column_bytes(stmt, j));
                break;
            }
            default:
                ASPL_PANIC("Unsupported SQLite datatype: %d", type);
            }
        }
        return ASPL_MAP_LITERAL("map<string, any>", 16, columns, column_count);
    }
    else {
        sqlite3_finalize(stmt);
        return ASPL_NULL();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_sqlite$response$finalize(ASPL_OBJECT_TYPE* handle) {
    sqlite3_stmt* stmt = ASPL_ACCESS(*handle).value.handle;
    sqlite3_finalize(stmt);
    return ASPL_UNINITIALIZED;
}