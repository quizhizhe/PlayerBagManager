#pragma once
extern class Logger logger;
#ifdef PLUGIN_DEV_MODE

inline double ns_time(LARGE_INTEGER begin_time, LARGE_INTEGER end_time, LARGE_INTEGER freq_) {
    return (end_time.QuadPart - begin_time.QuadPart) * 1000000.0 / freq_.QuadPart;
}
#define TestFuncTime(func, ...)\
{\
LARGE_INTEGER freq_;\
LARGE_INTEGER begin_time;\
LARGE_INTEGER end_time;\
QueryPerformanceFrequency(&freq_); \
QueryPerformanceCounter(&begin_time); \
func(__VA_ARGS__); \
QueryPerformanceCounter(&end_time); \
logger.warn("{}\t time: {}ns", #func, ns_time(begin_time, end_time, freq_));\
}

inline void _WASSERT(
    _In_z_ char const* _Message,
    _In_z_ char const* _File,
    _In_   unsigned    _Line
) {
    logger.error("断言失败，表达式：{}", _Message);
    logger.error("位于文件：{}", _File);
    logger.error("第 {} 行", _Line);
};

#define ASSERT(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_WASSERT(#expression, __FILE__, (unsigned)(__LINE__)), __debugbreak(), 0) \
        )
#else
#define TestLogTime(func, ...) ((void)0)
#define ASSERT(var) ((void)0)
#endif // PLUGIN_DEV_MODE