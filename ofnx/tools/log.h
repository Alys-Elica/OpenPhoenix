#ifndef TOOLS_LOG_H
#define TOOLS_LOG_H

#include <format>
#include <iostream>
#include <print>

#ifdef NDEBUG

// Release build
template <typename... T>
void logWrite(const std::string_view lvl, std::format_string<T...> fmt, T... args)
{
    auto msg = std::format(fmt, std::forward<T>(args)...);
    std::print(stderr, "[{}] {}\n", lvl, msg);
}

#define LOG_DEBUG(msg, ...)
#define LOG_INFO(msg, ...) logWrite("INFO", msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) logWrite("WARN", msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) logWrite("ERROR", msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) logWrite("FATAL", msg, ##__VA_ARGS__)

#else

// Debug build
template <typename... T>
void logWrite(const std::string_view lvl, const int line, const std::string_view func, std::format_string<T...> fmt, T... args)
{
    auto msg = std::format(fmt, std::forward<T>(args)...);
    std::print(stderr, "[{}][{}:{}] {}\n", lvl, func, line, msg);
}

#define LOG_DEBUG(msg, ...) logWrite("DEBUG", __LINE__, __FILE__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) logWrite("INFO", __LINE__, __FILE__, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) logWrite("WARN", __LINE__, __FILE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) logWrite("ERROR", __LINE__, __FILE__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) logWrite("FATAL", __LINE__, __FILE__, msg, ##__VA_ARGS__)

#endif

#endif // TOOLS_LOG_H
