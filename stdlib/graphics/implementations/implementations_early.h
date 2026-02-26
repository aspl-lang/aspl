#ifdef _WIN32
#if !defined(WINVER) || WINVER < 0x0602
#undef WINVER
#define WINVER 0x0602
#endif
#endif