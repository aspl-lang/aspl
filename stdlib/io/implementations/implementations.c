#include <dirent.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <glob.h>
#endif
#ifdef __APPLE__
#include <libproc.h>
#endif

char* aspl_util_io$windowsify_path(ASPL_OBJECT_TYPE path) {
    char* newPath = ASPL_MALLOC(ASPL_ACCESS(path).value.string->length + 1);
    strcpy(newPath, ASPL_ACCESS(path).value.string->str);
    for (int i = 0; i < ASPL_ACCESS(path).value.string->length; i++)
    {
        if (newPath[i] == '/')
        {
            newPath[i] = '\\';
        }
    }
    return newPath;
}

char* aspl_util_io$unixify_path(ASPL_OBJECT_TYPE path) {
    char* newPath = ASPL_MALLOC(ASPL_ACCESS(path).value.string->length + 1);
    strcpy(newPath, ASPL_ACCESS(path).value.string->str);
    for (int i = 0; i < ASPL_ACCESS(path).value.string->length; i++)
    {
        if (newPath[i] == '\\')
        {
            newPath[i] = '/';
        }
    }
    return newPath;
}

char* aspl_util_io$osify_path(ASPL_OBJECT_TYPE path) {
#ifdef _WIN32
    return aspl_util_io$windowsify_path(path);
#else
    return aspl_util_io$unixify_path(path);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$directory$exists(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(pathStr);
    return ASPL_BOOL_LITERAL(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat path_stat;
    if (stat(ASPL_ACCESS(*path).value.string->str, &path_stat) == -1) {
        return ASPL_BOOL_LITERAL(0);
    }
    return ASPL_BOOL_LITERAL(S_ISDIR(path_stat.st_mode));
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$directory$create(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
#ifdef _WIN32
    int result = mkdir(pathStr);
#else
    int result = mkdir(pathStr, 0777);
#endif
    if (result == 0)
    {
        return ASPL_TRUE();
    }
    else
    {
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$directory$delete(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    int result = rmdir(pathStr);
    if (result == 0)
    {
        return ASPL_TRUE();
    }
    else
    {
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$get_directory_path(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
#ifdef _WIN32
    char fullpath[PATH_MAX];
    DWORD len = GetFullPathNameA(pathStr, PATH_MAX, fullpath, NULL);
    if (len > 0) {
        if (GetFileAttributesA(fullpath) & FILE_ATTRIBUTE_DIRECTORY) {
            return ASPL_STRING_LITERAL(fullpath);
        }
        else {
            // If it's a file, remove the file name from the path
            char* lastSlash = strrchr(fullpath, '\\');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
            }
            return ASPL_STRING_LITERAL(fullpath);
        }
    }
    else {
        ASPL_PANIC("Path length <= 0");
    }
#else
    char* resolved_path = realpath(pathStr, NULL);
    if (resolved_path != NULL) {
        struct stat path_stat;
        stat(resolved_path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)) {
            return ASPL_STRING_LITERAL(resolved_path);
        }
        else {
            // If it's a file, remove the file name from the path
            char* lastSlash = strrchr(resolved_path, '/');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
            }
            return ASPL_STRING_LITERAL(resolved_path);
        }
    }
    else {
        ASPL_PANIC("Could not resolve path '%s'", pathStr);
    }
#endif
    // TODO: Resolve symlinks etc...
    // see: https://github.com/vlang/v/blob/master/vlib/os/os.c.v#L875
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$get_directory_name(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$unixify_path(*path); // unixify here because the code only works with `/`
    int pathLength = ASPL_ACCESS(*path).value.string->length;
    while (pathLength > 0 && pathStr[pathLength - 1] == '/') {
        pathStr[pathLength - 1] = '\0';
        pathLength--;
    }
    char* lastSlash = strrchr(pathStr, '/');
    if (lastSlash == NULL)
    {
        return ASPL_STRING_LITERAL(pathStr);
    }
    else
    {
        int lastSlashIndex = lastSlash - pathStr;
        char* directoryName = ASPL_MALLOC(ASPL_ACCESS(*path).value.string->length - lastSlashIndex);
        strcpy(directoryName, lastSlash + 1);
        return ASPL_STRING_LITERAL(directoryName);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$directory$list(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    DIR* dir = opendir(pathStr);
    if (dir == NULL)
    {
        return ASPL_LIST_LITERAL("list<string>", 12, 0, 0);
    }
    else
    {
        struct dirent* entry;
        int pathCount = 0;
        ASPL_OBJECT_TYPE* paths = NULL;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
#ifdef _WIN32
            char* fullPath = ASPL_MALLOC(strlen(pathStr) + strlen(entry->d_name) + 2);
            strcpy(fullPath, pathStr);
            strcat(fullPath, "/");
            strcat(fullPath, entry->d_name);
            DWORD dwAttrib = GetFileAttributes(fullPath);
            if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
                continue;
            }
#else
            if (entry->d_type != DT_DIR) {
                continue;
            }
#endif
            char* p = ASPL_MALLOC(ASPL_ACCESS(*path).value.string->length + strlen(entry->d_name) + 2);
            strcat(p, entry->d_name);
            paths = ASPL_REALLOC(paths, sizeof(ASPL_OBJECT_TYPE) * (pathCount + 1));
            paths[pathCount] = ASPL_STRING_LITERAL(p);
            pathCount++;
        }
        closedir(dir);
        return ASPL_LIST_LITERAL("list<string>", 12, paths, pathCount);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$directory$get_home_directory()
{
#ifdef _WIN32
    char* home = getenv("USERPROFILE");
    if (home == NULL)
    {
        ASPL_PANIC("Could not get home directory");
    }
    else
    {
        return ASPL_STRING_LITERAL(home);
    }
#else
    char* home = getenv("HOME");
    if (home == NULL)
    {
        ASPL_PANIC("Could not get home directory");
    }
    else
    {
        return ASPL_STRING_LITERAL(home);
    }
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$exists(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(pathStr);
    return ASPL_BOOL_LITERAL(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat path_stat;
    if (stat(pathStr, &path_stat) == 0 && S_ISREG(path_stat.st_mode)) {
        return ASPL_TRUE();
    }
    else {
        return ASPL_FALSE();
    }
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$read_string(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    FILE* file = fopen(pathStr, "rb"); // rb because the length is in bytes (which is not necessarily the same as characters)
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* buffer = ASPL_MALLOC(length + 1);
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
        fclose(file);
        return ASPL_STRING_LITERAL(buffer);
    }
    else
    {
        ASPL_PANIC("Could not open file '%s'", ASPL_ACCESS(*path).value.string->str);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$read_bytes(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    FILE* file = fopen(pathStr, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* buffer = ASPL_MALLOC(length);
        fread(buffer, 1, length, file);
        fclose(file);
        ASPL_OBJECT_TYPE* bytes = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * length);
        for (int i = 0; i < length; i++)
        {
            bytes[i] = ASPL_BYTE_LITERAL(buffer[i]);
        }
        return ASPL_LIST_LITERAL("list<byte>", 10, bytes, length);
    }
    else
    {
        ASPL_PANIC("Could not open file '%s'", ASPL_ACCESS(*path).value.string->str);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$write_string(ASPL_OBJECT_TYPE* path, ASPL_OBJECT_TYPE* string)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    FILE* file = fopen(pathStr, "wb"); // wb because the length is in bytes (which is not necessarily the same as characters)    if (file)
    if (file)
    {
        fwrite(ASPL_ACCESS(*string).value.string->str, 1, ASPL_ACCESS(*string).value.string->length, file);
        fclose(file);
        return ASPL_TRUE();
    }
    else
    {
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$write_bytes(ASPL_OBJECT_TYPE* path, ASPL_OBJECT_TYPE* bytes)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    FILE* file = fopen(pathStr, "wb");
    if (file)
    {
        for (int i = 0; i < ASPL_ACCESS(bytesObj).value.list->length; i++)
        {
            ASPL_OBJECT_TYPE byte = ASPL_ACCESS(bytesObj).value.list->value[i];
            fwrite(&ASPL_ACCESS(byte).value.integer8, 1, 1, file);
        }
        fclose(file);
        return ASPL_TRUE();
    }
    else
    {
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$get_file_name(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$unixify_path(*path); // unixify here because the code only works with `/`
    char* lastSlash = strrchr(pathStr, '/');
    if (lastSlash == NULL)
    {
        return ASPL_STRING_LITERAL(pathStr);
    }
    else
    {
        return ASPL_STRING_LITERAL(lastSlash + 1);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$delete(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    int result = remove(pathStr);
    if (result == 0)
    {
        return ASPL_TRUE();
    }
    else
    {
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$file$list(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
    DIR* dir = opendir(pathStr);
    if (dir == NULL)
    {
        return ASPL_LIST_LITERAL("list<string>", 12, 0, 0);
    }
    else
    {
        struct dirent* entry;
        int pathCount = 0;
        ASPL_OBJECT_TYPE* paths = NULL;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
#ifdef _WIN32
            char* fullPath = ASPL_MALLOC(strlen(pathStr) + strlen(entry->d_name) + 2);
            strcpy(fullPath, pathStr);
            strcat(fullPath, "/");
            strcat(fullPath, entry->d_name);
            DWORD dwAttrib = GetFileAttributes(fullPath);
            if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
#else
            if (entry->d_type == DT_DIR) {
                continue;
            }
#endif
            paths = ASPL_REALLOC(paths, sizeof(ASPL_OBJECT_TYPE) * (pathCount + 1));
            paths[pathCount] = ASPL_STRING_LITERAL(entry->d_name);
            pathCount++;
        }
        closedir(dir);
        return ASPL_LIST_LITERAL("list<string>", 12, paths, pathCount);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$glob$search(ASPL_OBJECT_TYPE* pattern)
{
    char* patternStr = aspl_util_io$osify_path(*pattern);
#ifdef _WIN32
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(patternStr, &data);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        ASPL_OBJECT_TYPE* paths = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
        int pathCount = 0;
        if (!strcmp(data.cFileName, ".") && !strcmp(data.cFileName, ".."))
        {
            int size = strlen(data.cFileName) + 1;
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                size++;
            }
            char* path = ASPL_MALLOC(size);
            strcpy(path, data.cFileName);
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                path[strlen(data.cFileName)] = '/';
            }
            path[size - 1] = '\0';
            paths[0] = ASPL_STRING_LITERAL(path);
            pathCount++;
        }
        while (FindNextFile(hFind, &data))
        {
            if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
            {
                continue;
            }
            paths = ASPL_REALLOC(paths, sizeof(ASPL_OBJECT_TYPE) * (pathCount + 1));
            int size = strlen(data.cFileName) + 1;
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                size++;
            }
            char* path = ASPL_MALLOC(size);
            strcpy(path, data.cFileName);
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                path[strlen(data.cFileName)] = '/';
            }
            path[size - 1] = '\0';
            paths[pathCount] = ASPL_STRING_LITERAL(path);
            pathCount++;
        }
        FindClose(hFind);
        return ASPL_LIST_LITERAL("list<string>", 12, paths, pathCount);
    }
    else
    {
        ASPL_OBJECT_TYPE* paths = ASPL_MALLOC(1);
        return ASPL_LIST_LITERAL("list<string>", 12, paths, 0);
    }
#else
#ifdef __ANDROID__
    // TODO: Implement glob for Android
    ASPL_OBJECT_TYPE* paths = ASPL_MALLOC(1);
    return ASPL_LIST_LITERAL("list<string>", 12, paths, 0);
#else
    glob_t glob_result;
    glob(patternStr, GLOB_TILDE, NULL, &glob_result);
    ASPL_OBJECT_TYPE* paths = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * glob_result.gl_pathc);
    for (int i = 0; i < glob_result.gl_pathc; i++)
    {
        char* path = glob_result.gl_pathv[i];
        struct stat path_stat;
        stat(path, &path_stat);
        if (S_ISDIR(path_stat.st_mode)) {
            int size = strlen(path) + 2;
            char* pathCopy = ASPL_MALLOC(size);
            strcpy(pathCopy, path);
            pathCopy[size - 2] = '/';
            pathCopy[size - 1] = '\0';
            paths[i] = ASPL_STRING_LITERAL(pathCopy);
        }
        else {
            paths[i] = ASPL_STRING_LITERAL(path);
        }
    }
    globfree(&glob_result);
    return ASPL_LIST_LITERAL("list<string>", 12, paths, glob_result.gl_pathc);
#endif
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$relative_to_absolute(ASPL_OBJECT_TYPE* path)
{
    char* pathStr = aspl_util_io$osify_path(*path);
#ifdef _WIN32
    char* absolutePath = ASPL_MALLOC(MAX_PATH);
    GetFullPathName(pathStr, MAX_PATH, absolutePath, NULL);
    return ASPL_STRING_LITERAL(absolutePath);
#else
    char* absolutePath = realpath(pathStr, NULL);
    if (absolutePath == NULL)
    {
        ASPL_PANIC("Could not resolve path '%s'", pathStr);
    }
    else
    {
        char* absPathCopy = ASPL_MALLOC(strlen(absolutePath) + 1);
        strcpy(absPathCopy, absolutePath);
        free(absolutePath);
        return ASPL_STRING_LITERAL(absPathCopy);
    }
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$get_current_executable_path()
{
#ifdef _WIN32
    char* path = ASPL_MALLOC(MAX_PATH);
    GetModuleFileName(NULL, path, MAX_PATH);
    return ASPL_STRING_LITERAL(path);
#elif __APPLE__
    char result[PATH_MAX];
    pid_t pid = getpid();
    int ret = proc_pidpath(pid, result, sizeof(result));
    if (ret <= 0) {
        ASPL_PANIC("os.executable() failed at calling proc_pidpath with pid %d: %d\n", pid, ret);
    }
    return ASPL_STRING_LITERAL(result);
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
    if (count == -1) {
        ASPL_PANIC("os.executable() failed at calling readlink: %s", strerror(errno));
    }
    path[count] = '\0';
    return ASPL_STRING_LITERAL(path);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$copy(ASPL_OBJECT_TYPE* source, ASPL_OBJECT_TYPE* destination) {
    char* sourceStr = aspl_util_io$osify_path(*source);
    char* destinationStr = aspl_util_io$osify_path(*destination);
#ifdef _WIN32
    CopyFile(sourceStr, destinationStr, FALSE);
#else
    FILE* sourceFile = fopen(sourceStr, "r");
    FILE* destinationFile = fopen(destinationStr, "w");
    if (sourceFile && destinationFile)
    {
        char buffer[4096];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), sourceFile)) != 0)
        {
            fwrite(buffer, 1, bytes, destinationFile);
        }
    }
    fclose(sourceFile);
    fclose(destinationFile);
#endif
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$path$move(ASPL_OBJECT_TYPE* source, ASPL_OBJECT_TYPE* destination) {
    char* sourceStr = aspl_util_io$osify_path(*source);
    char* destinationStr = aspl_util_io$osify_path(*destination);
#ifdef _WIN32
    MoveFile(sourceStr, destinationStr);
#else
    rename(sourceStr, destinationStr);
#endif
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_io$create_symlink(ASPL_OBJECT_TYPE* source, ASPL_OBJECT_TYPE* destination) {
    char* sourceStr = aspl_util_io$osify_path(*source);
    char* destinationStr = aspl_util_io$osify_path(*destination);
#ifdef _WIN32
    CreateSymbolicLink(sourceStr, destinationStr, 0);
#else
    symlink(sourceStr, destinationStr);
#endif
    return ASPL_UNINITIALIZED;
}