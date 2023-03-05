#pragma once

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #define SPDLOG_USE_STD_FORMAT
    #include <spdlog/logger.h>
#endif

    class Log
    {
    public:
        static void init();

    #ifdef SK_ENABLE_DEBUG_OUTPUT
        static shared<spdlog::logger> &getCoreLogger() { return core_logger; }
        static shared<spdlog::logger> &getClientLogger() { return client_logger; }
    
    private:
        static shared<spdlog::logger> core_logger;
        static shared<spdlog::logger> client_logger;
    #endif
    };

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #ifdef SK_CORE
        #define SK_LOGGER Log::getCoreLogger()
    #else
        #define SK_LOGGER Log::getClientLogger()
    #endif

    template<typename... Args>
    static std::string _sk_msg(const path& FILE, const int LINE, std::string_view msg, Args&&... args)
    {
        return std::format("[{}:{}]: {}", FILE.filename(), std::to_string(LINE), std::vformat(msg, std::make_format_args(args...)));
    }

    template<typename T>
    static std::string _sk_msg(const path& FILE, const int LINE, const T& t)
    {
        return _sk_msg(FILE, LINE, "{}", t);
    }

    #define _SK_MSG(...) _sk_msg(__FILE__, __LINE__, __VA_ARGS__)

    #define SK_TRACE(...) SK_LOGGER->trace(_SK_MSG(__VA_ARGS__))
    #define SK_INFO(...) SK_LOGGER->info(_SK_MSG(__VA_ARGS__))
    #define SK_WARN(...) SK_LOGGER->warn(_SK_MSG(__VA_ARGS__))
    #define SK_ERROR(...) SK_LOGGER->error(_SK_MSG(__VA_ARGS__));
    #define SK_CRITICAL(...) do { SK_LOGGER->critical(_SK_MSG(__VA_ARGS__)); std::abort(); } while(0)
    #define SK_ASSERT(x, ...) do { if (!(x)) { SK_LOGGER->critical(__VA_ARGS__); } } while (0)
    #define SK_VERIFY(x, ...) do { if (!(x)) { SK_LOGGER->error(__VA_ARGS__); } } while (0)
#else
    #define SK_TRACE(...)
    #define SK_INFO(...)
    #define SK_WARN(...)
    #define SK_ERROR(...)
    #define SK_CRITICAL(...)
    #define SK_ASSERT(x, ...)
    #define SK_VERIFY(x, ...)
#endif