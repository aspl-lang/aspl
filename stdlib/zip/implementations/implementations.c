#define MINIZ_MALLOC ASPL_MALLOC
#define MINIZ_FREE ASPL_FREE
#include "thirdparty/miniz/miniz.c"

#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#endif
// TODO: Are these includes necessary/correct?

// TODO: Support "/" instead of "\\" on Windows

void create_directory_recursive(const char* path) {
    // find parent directory
    char* parent_path = ASPL_MALLOC(strlen(path) + 1);
    strcpy(parent_path, path);
#ifdef _WIN32
    char* last_slash = strrchr(parent_path, '\\');
#else
    char* last_slash = strrchr(parent_path, '/');
#endif
    if (last_slash != NULL) {
        *last_slash = '\0';
    }

    // create parent directory
#ifdef _WIN32
    if (_access(parent_path, F_OK) != 0) {
        create_directory_recursive(parent_path);
    }
#else
    if (access(parent_path, F_OK) != 0) {
        create_directory_recursive(parent_path);
    }
#endif

    // create directory
#ifdef _WIN32
    if (_access(path, F_OK) != 0) {
        CreateDirectory(path, NULL);
    }
#else
    if (access(path, F_OK) != 0) {
        mkdir(path, 0777);
    }
#endif
}

void unzip_file(const char* zip_path, const char* folder_path) {
#ifdef _WIN32
    if (_access(folder_path, F_OK) != 0) {
        CreateDirectory(folder_path, NULL);
    }
#else
    if (access(folder_path, F_OK) != 0) {
        mkdir(folder_path, 0777);
    }
#endif

    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, zip_path, 0)) {
        printf("Failed to initialize zip reader\n");
        printf("Path: %s\n", zip_path);
        return;
    }

    int num_files = mz_zip_reader_get_num_files(&zip_archive);
    for (int i = 0; i < num_files; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            printf("Failed to read file stat\n");
            mz_zip_reader_end(&zip_archive);
            return;
        }

        char* file_path = ASPL_MALLOC(strlen(folder_path) + strlen(file_stat.m_filename) + 2);
#ifdef _WIN32
        sprintf(file_path, "%s\\%s", folder_path, file_stat.m_filename);
#else
        sprintf(file_path, "%s/%s", folder_path, file_stat.m_filename);
#endif

        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
            create_directory_recursive(file_path);
        }
        else {
            char* parent_path = ASPL_MALLOC(strlen(file_path) + 1);
            strcpy(parent_path, file_path);
#ifdef _WIN32
            char* last_slash = strrchr(parent_path, '\\');
#else
            char* last_slash = strrchr(parent_path, '/');
#endif
            if (last_slash != NULL) {
                *last_slash = '\0';
            }

            create_directory_recursive(parent_path);

#ifdef _WIN32
            if (_access(file_path, F_OK) != 0) {
                FILE* fp = fopen(file_path, "wb");
                if (fp != NULL) {
                    fclose(fp);
                }
                else {
                    ASPL_PANIC("Failed to create file '%s'", file_path);
                }
            }
#else
            if (access(file_path, F_OK) != 0) {
                FILE* fp = fopen(file_path, "w");
                if (fp != NULL) {
                    fclose(fp);
                }
                else {
                    ASPL_PANIC("Failed to create file '%s'", file_path);
                }
            }
#endif


            if (!mz_zip_reader_extract_to_file(&zip_archive, i, file_path, 0)) {
                printf("Failed to extract file from zip archive: %s\n", file_path);
                printf("Error: %s\n", mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)));
            }

            ASPL_FREE(parent_path);
        }

        ASPL_FREE(file_path);
    }

    mz_zip_reader_end(&zip_archive);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_zip$extract_file_to_directory(ASPL_OBJECT_TYPE* filePath, ASPL_OBJECT_TYPE* directoryPath) {
    unzip_file(ASPL_ACCESS(*filePath).value.string->str, ASPL_ACCESS(*directoryPath).value.string->str);
    return ASPL_UNINITIALIZED;
}

void add_directory_to_archive(mz_zip_archive* zip_archive, const char* directory) {
#ifdef _WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", directory);
    if ((hFind = FindFirstFile(search_path, &FindFileData)) != INVALID_HANDLE_VALUE) {
        do {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0) {
                    char subfolder_path[MAX_PATH];
                    snprintf(subfolder_path, sizeof(subfolder_path), "%s\\%s", directory, FindFileData.cFileName);
                    add_directory_to_archive(zip_archive, subfolder_path);
                }
            }
            else {
                char file_path[MAX_PATH];
                snprintf(file_path, sizeof(file_path), "%s\\%s", directory, FindFileData.cFileName);
                if (mz_zip_writer_add_file(zip_archive, file_path, file_path, NULL, 0, MZ_DEFAULT_COMPRESSION) != MZ_TRUE) {
                    printf("Failed to add file to zip archive: %s\n", file_path);
                }
            }
        } while (FindNextFile(hFind, &FindFileData) != 0);
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        printf("Failed to open directory\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry->d_name);

        if (mz_zip_writer_add_file(zip_archive, entry->d_name, file_path, NULL, 0, MZ_DEFAULT_COMPRESSION) != MZ_TRUE) {
            printf("Failed to add file to zip archive: %s\n", file_path);
        }
    }

    closedir(dir);
#endif
}

void zip_folder(const char* zip_path, const char* folder_path) {
#ifdef _WIN32
    if (_access(zip_path, F_OK) != 0) {
        FILE* fp = fopen(zip_path, "wb");
        if (fp != NULL) {
            fclose(fp);
        }
        else {
            ASPL_PANIC("Failed to create zip archive '%s'", zip_path);
        }
    }
#else
    if (access(zip_path, F_OK) != 0) {
        FILE* fp = fopen(zip_path, "w");
        if (fp != NULL) {
            fclose(fp);
        }
        else {
            ASPL_PANIC("Failed to create zip archive '%s'", zip_path);
        }
    }
#endif

    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_writer_init_file(&zip_archive, zip_path, 0)) {
        ASPL_PANIC("Failed to initialize zip writer for '%s'", zip_path);
    }

    add_directory_to_archive(&zip_archive, folder_path);

    if (!mz_zip_writer_finalize_archive(&zip_archive)) {
        ASPL_PANIC("Failed to finalize zip archive '%s'", zip_path);
        return;
    }

    mz_zip_writer_end(&zip_archive);
}

void zip_files(const char* zip_path, const char** file_paths, int amount) {
#ifdef _WIN32
    if (_access(zip_path, F_OK) != 0) {
        FILE* fp = fopen(zip_path, "wb");
        if (fp != NULL) {
            fclose(fp);
        }
        else {
            ASPL_PANIC("Failed to create zip archive '%s'", zip_path);
        }
    }
#else
    if (access(zip_path, F_OK) != 0) {
        FILE* fp = fopen(zip_path, "w");
        if (fp != NULL) {
            fclose(fp);
        }
        else {
            ASPL_PANIC("Failed to create zip archive '%s'", zip_path);
        }
    }
#endif

    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_writer_init_file(&zip_archive, zip_path, 0)) {
        ASPL_PANIC("Failed to initialize zip writer for '%s'", zip_path);
    }

    for (int i = 0; i < amount; i++) {
        if (mz_zip_writer_add_file(&zip_archive, file_paths[i], file_paths[i], NULL, 0, MZ_DEFAULT_COMPRESSION) != MZ_TRUE) {
            ASPL_PANIC("Failed to add file to zip archive: %s", file_paths[i]);
        }
    }

    if (!mz_zip_writer_finalize_archive(&zip_archive)) {
        ASPL_PANIC("Failed to finalize zip archive '%s'", zip_path);
        return;
    }

    mz_zip_writer_end(&zip_archive);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_zip$create_archive_from_directory(ASPL_OBJECT_TYPE* directoryPath, ASPL_OBJECT_TYPE* filePath) {
    zip_folder(ASPL_ACCESS(*directoryPath).value.string->str, ASPL_ACCESS(*filePath).value.string->str);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_zip$create_archive_from_files(ASPL_OBJECT_TYPE* files, ASPL_OBJECT_TYPE* filePath) {
    char** filePaths = ASPL_MALLOC(sizeof(char*) * ASPL_ACCESS(*files).value.list->length);
    for (int i = 0; i < ASPL_ACCESS(*files).value.list->length; i++) {
        filePaths[i] = ASPL_ACCESS(ASPL_ACCESS(*files).value.list->value[i]).value.string->str;
    }
    zip_files(ASPL_ACCESS(*filePath).value.string->str, (const char**)filePaths, ASPL_ACCESS(*files).value.list->length);
    ASPL_FREE(filePaths);
    return ASPL_UNINITIALIZED;
}