#ifdef _WIN32
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <stdlib.h>
#endif

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$get_current_program_arguments()
{
    int argc = aspl_argc;
    char** argv = aspl_argv;
    ASPL_OBJECT_TYPE* args = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * argc);
    for (int i = 0; i < argc; i++)
    {
        args[i] = ASPL_STRING_LITERAL(argv[i]);
    }
    return ASPL_LIST_LITERAL("list<string>", 11, args, argc);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$command$execute(ASPL_OBJECT_TYPE* command)
{
    ASPL_OBJECT_TYPE commandObj = (ASPL_OBJECT_TYPE)*command;
#ifdef _WIN32
    FILE* pipe = _popen(ASPL_ACCESS(commandObj).value.string->str, "r");
#else
    FILE* pipe = popen(ASPL_ACCESS(commandObj).value.string->str, "r");
#endif
    if (pipe == NULL)
    {
        ASPL_PANIC("Failed to execute command '%s'", ASPL_ACCESS(commandObj).value.string->str);
    }
    else
    {
        char* output = ASPL_MALLOC(1);
        int outputLength = 0;
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            output = ASPL_REALLOC(output, outputLength + strlen(buffer) + 1);
            strcpy(output + outputLength, buffer);
            outputLength += strlen(buffer);
        }
#if defined(_WIN32)
        int exitCode = _pclose(pipe);
#else
        int exitCode = pclose(pipe);
#endif
        ASPL_OBJECT_TYPE* result = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 2);
        result[0] = ASPL_INT_LITERAL(exitCode);
        result[1] = ASPL_STRING_LITERAL(output);
        return ASPL_LIST_LITERAL("list<integer|string>", 10, result, 2);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$command$system(ASPL_OBJECT_TYPE* command)
{
    ASPL_OBJECT_TYPE commandObj = (ASPL_OBJECT_TYPE)*command;
    int exitCode = system(ASPL_ACCESS(commandObj).value.string->str);
    return ASPL_INT_LITERAL(exitCode);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$command$execvp(ASPL_OBJECT_TYPE* command, ASPL_OBJECT_TYPE* args)
{
    ASPL_OBJECT_TYPE commandObj = (ASPL_OBJECT_TYPE)*command;
    ASPL_OBJECT_TYPE argsObj = (ASPL_OBJECT_TYPE)*args;
    const char* commandStr = ASPL_ACCESS(commandObj).value.string->str;
    int numArgs = ASPL_ACCESS(argsObj).value.list->length - 1;
    const char** argsv = (const char**)ASPL_MALLOC(sizeof(const char*) * (numArgs + 2));

    argsv[0] = commandStr;

    for (int i = 1; i <= numArgs; i++)
    {
        argsv[i] = ASPL_ACCESS(ASPL_ACCESS(argsObj).value.list->value[i]).value.string->str;
    }

    argsv[numArgs + 1] = NULL;  // The last element of argsv should be NULL.

#ifdef _WIN32
    _execvp(commandStr, argsv);
#else
    execvp(commandStr, (char* const*)argsv);
#endif
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$get_working_directory()
{
#ifdef _WIN32
    char* path = ASPL_MALLOC(MAX_PATH);
    GetCurrentDirectory(MAX_PATH, path);
    return ASPL_STRING_LITERAL(path);
#else
    char* path = ASPL_MALLOC(PATH_MAX);
    getcwd(path, PATH_MAX);
    return ASPL_STRING_LITERAL(path);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$change_working_directory(ASPL_OBJECT_TYPE* path)
{
    ASPL_OBJECT_TYPE pathObj = (ASPL_OBJECT_TYPE)*path;
#ifdef _WIN32
    SetCurrentDirectory(ASPL_ACCESS(pathObj).value.string->str);
#else
    chdir(ASPL_ACCESS(pathObj).value.string->str);
#endif
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$change_mode(ASPL_OBJECT_TYPE* path, ASPL_OBJECT_TYPE* mode)
{
    ASPL_OBJECT_TYPE pathObj = (ASPL_OBJECT_TYPE)*path;
    ASPL_OBJECT_TYPE modeObj = (ASPL_OBJECT_TYPE)*mode;
#ifdef _WIN32
    // no equivalent on Windows
#else
    chmod(ASPL_ACCESS(pathObj).value.string->str, ASPL_ACCESS(modeObj).value.integer32);
#endif
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$create_temporary_directory()
{
    char* tempDirPath;
#ifdef _WIN32
    char tempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, tempPath) == 0)
    {
        ASPL_PANIC("Failed to retrieve temporary path.");
    }

    tempDirPath = ASPL_MALLOC(MAX_PATH);
    if (GetTempFileName(tempPath, "", 0, tempDirPath) == 0)
    {
        ASPL_PANIC("Failed to create temporary directory name.");
    }

    DeleteFile(tempDirPath);
    if (!CreateDirectory(tempDirPath, NULL))
    {
        ASPL_PANIC("Failed to create temporary directory.");
    }
#else
    tempDirPath = ASPL_MALLOC(PATH_MAX);
    snprintf(tempDirPath, PATH_MAX, "/tmp/XXXXXX");

    if (mkdtemp(tempDirPath) == NULL)
    {
        ASPL_PANIC("Failed to create temporary directory.");
    }
#endif
    return ASPL_STRING_LITERAL_NO_COPY(tempDirPath);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$get_current_runtime_os_name()
{
    // TODO
#ifdef _WIN32
    return ASPL_STRING_LITERAL("windows");
#elif __APPLE__
    return ASPL_STRING_LITERAL("macos");
#elif __linux__
    return ASPL_STRING_LITERAL("linux");
#elif __unix__
    return ASPL_STRING_LITERAL("unix");
#else
    return ASPL_STRING_LITERAL("unknown");
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$get_current_runtime_architecture_name()
{
#ifdef _WIN32
    return ASPL_STRING_LITERAL(getenv("PROCESSOR_ARCHITECTURE"));
#else
    struct utsname* d = ASPL_MALLOC(sizeof(struct utsname));
    uname(d);
    return ASPL_STRING_LITERAL(d->machine);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_os$get_current_runtime_architecture_enum()
{
    // TODO
#if defined(__x86_64__) || defined(_M_X64)
    return ASPL_INT_LITERAL(0);
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    return ASPL_INT_LITERAL(5);
#elif arm64
    return ASPL_INT_LITERAL(2);
#elif arm32
    return ASPL_INT_LITERAL(3);
#elif rv64
    return ASPL_INT_LITERAL(4);
#elif rv32
    return ASPL_INT_LITERAL(5);
#elif i386
    return ASPL_INT_LITERAL(6);
#else
    return ASPL_INT_LITERAL(-1);
#endif
}