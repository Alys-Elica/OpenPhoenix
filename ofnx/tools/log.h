#ifndef TOOLS_LOG_H
#define TOOLS_LOG_H

#include <format>
#include <iostream>

#if defined(_MSC_VER)
#define FUNC_SIG __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#define FUNC_SIG __PRETTY_FUNCTION__
#else
#define FUNC_SIG __func__
#endif

#define LOG(level, fmt, ...)                                       \
    do {                                                           \
        std::cout << "[" << level << "][" << __func__ << "] "      \
                  << std::format(fmt, ##__VA_ARGS__) << std::endl; \
    } while (0)
#define LOG_DEBUG(fmt, ...) LOG("DEBUG", fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG("INFO", fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) LOG("WARNING", fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG("CRITICAL", fmt, ##__VA_ARGS__)

#endif // TOOLS_LOG_H
