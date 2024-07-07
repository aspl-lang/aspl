// NOTE: This implementation only works with the AIL backend, not the C backend!

unsigned int aspl_util_breakpoints$log(char* msg, ...) {
    va_list args;
    va_start(args, msg);
    printf("\033[90m");
    vprintf(msg, args);
    printf("\033[0m\n");
    va_end(args);
    return strlen(msg);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_breakpoints$break(ASPL_OBJECT_TYPE* message) {
    if (ASPL_ACCESS(*message).kind != ASPL_OBJECT_KIND_NULL) {
        aspl_util_breakpoints$log("Breakpoint: %s", ASPL_ACCESS(*message).value.string->str);
    }
    int n = 0;
    while (1) {
        char c[1024];
        fgets(c, 1024, stdin);
        c[strlen(c) - 1] = '\0';
        if (strcmp(c, "c") == 0 || strcmp(c, "continue") == 0) {
            // TODO: Fill in the correct number of backspaces to clear the breakpoint messages
            (void)n; // prevent unused variable warning
            return ASPL_UNINITIALIZED;
        } else if (strcmp(c, "st") == 0 || strcmp(c, "stack") == 0 || strcmp(c, "stacktrace") == 0 || strcmp(c, "stack-trace") == 0 || strcmp(c, "stack_trace") == 0) {
            aspl_ailinterpreter_display_stack_trace(aspl_ailinterpreter_current_thread_context);
        } else if (strcmp(c, "q") == 0 || strcmp(c, "quit") == 0 || strcmp(c, "exit") == 0) {
            exit(0);
        } else if (strcmp(c, "sc") == 0 || strcmp(c, "scope") == 0) {
            // TODO
        } else if (strcmp(c, "h") == 0 || strcmp(c, "help") == 0) {
            n += aspl_util_breakpoints$log("Commands:");
            n += aspl_util_breakpoints$log("  c, continue: Continue execution");
            n += aspl_util_breakpoints$log("  st, stack, stacktrace, stack-trace, stack_trace: Display stack trace");
            n += aspl_util_breakpoints$log("  q, quit, exit: Quit the program");
            n += aspl_util_breakpoints$log("  sc, scope: Display the current scope");
            n += aspl_util_breakpoints$log("  h, help: Display this help message");
        } else {
            n += aspl_util_breakpoints$log("Unknown command: %s", c);
        }
    }
}