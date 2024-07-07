#ifdef _WIN32
#include <windows.h>
#include <time.h>
#else
#include <time.h>
#endif

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_time$nanosleep(ASPL_OBJECT_TYPE* nanoseconds) {
    ASPL_OBJECT_TYPE nanosecondsObj = (ASPL_OBJECT_TYPE)*nanoseconds;
#ifdef _WIN32
    Sleep(ASPL_ACCESS(nanosecondsObj).value.integer64 / 1000000);
#else
    struct timespec ts;
    ts.tv_sec = ASPL_ACCESS(nanosecondsObj).value.integer64 / 1000000000;
    ts.tv_nsec = ASPL_ACCESS(nanosecondsObj).value.integer64 % 1000000000;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
        // Retry if interrupted by a signal
        continue;
    }
#endif
    return ASPL_UNINITIALIZED;
}

// taken from: https://github.com/vlang/v/blob/master/vlib/time/chrono.v#L7
long aspl_util_days_from_unix_epoch(long long year, long long month, long long day) {
    long long y = month <= 2 ? year - 1 : year;
    long long era = y / 400;
    long long year_of_the_era = y - era * 400;
    long long day_of_year = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    long long day_of_the_era = year_of_the_era * 365 + year_of_the_era / 4 - year_of_the_era / 100 + day_of_year;
    return era * 146097 + day_of_the_era - 719468;
}

// taken from: https://github.com/vlang/v/blob/master/vlib/time/chrono.c.v#L5
long long aspl_util_portable_timegm(struct tm* t) {
    long long year = t->tm_year + 1900;
    long long month = t->tm_mon;
    if (month > 11) {
        year += month / 12;
        month %= 12;
    }
    else if (month < 0) {
        long long years_diff = (11 - month) / 12;
        year -= years_diff;
        month += 12 * years_diff;
    }
    long long days_since_1970 = (long long)aspl_util_days_from_unix_epoch(year, month + 1, t->tm_mday);
    return 60 * (60 * (24 * days_since_1970 + t->tm_hour) + t->tm_min) + t->tm_sec;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_time$nanotime_utc() {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    struct tm t = {
        .tm_year = st.wYear - 1900,
        .tm_mon = st.wMonth - 1,
        .tm_mday = st.wDay,
        .tm_hour = st.wHour,
        .tm_min = st.wMinute,
        .tm_sec = st.wSecond
    };
    return ASPL_LONG_LITERAL(aspl_util_portable_timegm(&t) * 1000000000 + st.wMilliseconds * 1000000);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ASPL_LONG_LITERAL(ts.tv_sec * 1000000000 + ts.tv_nsec);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_time$nanotime_local() {
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return ASPL_LONG_LITERAL(uli.QuadPart * 100);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ASPL_LONG_LITERAL(ts.tv_sec * 1000000000 + ts.tv_nsec);
#endif
}