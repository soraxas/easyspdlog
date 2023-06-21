#pragma once

#include <soraxas_toolbox/string_from_stuff.h>
#include <spdlog/common.h>
#include <spdlog/fmt/ostr.h>  // Enables logging any object with << operator
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief easyspdlog is a small libarary that uses spdlog for normal consistent logging across
 * mission systems projects
 *
 * ATTENTION: To use easyspdlog in your project and start logging, you must do following:
 *  - include the installed header file with "#include <easyspdlog.h>"
 *  - run the default setup at the start of main() with "easyspdlog::default_setup()"
 *  - add "easyspdlog" to your target_link_libararies() in CMAKE
 *  - have the SPDLOG_ACTIVE_LEVEL set in the CMAKE of your project (see below)
 *
 * CMAKE: For configurable logging behaviour from cmake, add the following lines to your
 * CMakeLists.txt: set(SPDLOG_ACTIVE_LEVEL "SPDLOG_LEVEL_OFF" CACHE STRING "SPDLOG_ACTIVE_LEVEL for
 * compile time logging") set_property(CACHE SPDLOG_ACTIVE_LEVEL PROPERTY STRINGS SPDLOG_LEVEL_OFF
 * SPDLOG_LEVEL_TRACE SPDLOG_LEVEL_DEBUG SPDLOG_LEVEL_INFO SPDLOG_LEVEL_WARN SPDLOG_LEVEL_ERROR
 * SPDLOG_LEVEL_CRITICAL) add_compile_definitions(SPDLOG_ACTIVE_LEVEL=${SPDLOG_ACTIVE_LEVEL})
 *
 *  Also add easyspdlog to your target_link_libararies(). Example:
 *      target_link_libararies(<target> PUBLIC easyspdlog)
 *
 * DETAILS:
 *  If you need specific formats, logging levels, file-logging, etc, then see the documentation
 * below.
 *
 */
namespace easyspdlog
{
    using sxs::string::simplify_type_name;
    using sxs::string::get_type_name;

#define EASYSPDLOG_TYPE_NAME(QUERY_TYPE) ::easyspdlog::get_type_name(typeid(QUERY_TYPE))

    namespace format
    {
        constexpr const char *format_minimal = "[%^%L%$] %v";
        constexpr const char *format_datetime =
            "[%Y%m%dT%H%M%S.%f] [%^%l%$] %v";  // e.g. [2021-02-26 10:59:59.669] [W]
        constexpr const char *format_time = "[%H:%M:%S.%e] [%^%L%$] %v";
        constexpr const char *format_time_code_linux_epoch = "[%E.%e] [%^%L:%s:%#%$] %v";
        constexpr const char *format_time_code =
            "[%H:%M:%S.%e] [%^%L:%s:%#%$] %v";  // e.g. [10:59:59.669] [W:main.cpp:43]
        constexpr const char *format_spdlog_default = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v";
    };  // namespace format

    /**
     * @brief By default, spdlog sends logs to stdout with colour and is multithreaded. This
     * function changes the default logger to stderr, colour and is multithreaded.
     */
    inline void set_console_to_stderr()
    {
        // The following comes from spdlog's own documentation, which is slightly ugly.
        // Replace the default logger with a (color, single-threaded) stderr logger
        // (but first replace it with an arbitrarily-named logger to prevent a name clash)
        spdlog::set_default_logger(spdlog::stderr_color_mt("temp"));
        spdlog::set_default_logger(spdlog::stderr_color_mt(""));
    }

    /**
     * @brief Set the console logging format. Some example formats exist under the easyspdlog
     * namespace, with format_time_code set by default.
     *
     * @param format
     */
    inline void set_console_format(const std::string &format = format::format_time_code)
    {
        spdlog::default_logger()->sinks()[0]->set_pattern(format);
    }

    /**
     * @brief Add a file sink to the default logger. Logs will now be sent to this file however it
     * is important to note that log messages do not flush by default, so if your program crashes,
     * the log file may not contain all logs. Two ways to combat this would be to call
     * set_flush_severity() to force flushing, or to call the flush() function manually in your
     * program.
     *
     * @param filepath
     * @param log_level
     * @param format
     * @param truncate
     */
    inline void add_file_sink(
        const std::string &filename,
        const spdlog::level::level_enum log_level = spdlog::level::level_enum::trace,
        const std::string &format = format::format_time_code, const bool truncate = true
    )
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
        file_sink->set_level(log_level);
        file_sink->set_pattern(format);
        spdlog::default_logger()->sinks().emplace_back(file_sink);
    }

    /**
     * @brief Set the flush severity of the logger (all logs will be flushed if a log of this
     * severity or higher is encountered). This may be useful if you program can crash and you are
     * logging to files, or in some multithreaded scenarios.
     *
     * @param log_level
     */
    inline void set_flush_severity(const spdlog::level::level_enum log_level)
    {
        spdlog::default_logger()->flush_on(log_level);
    }

    /**
     * @brief Force the logger to flush current logs to all sinks.
     */
    inline void flush()
    {
        spdlog::default_logger()->flush();
    }

    /**
     * @brief Default setup of logging behaviour. This is most likely the only function that needs
     * to be called. It has the following features:
     *  - Logging to stderr
     *  - Log level to be the same as compile level
     *  - Format as per format argument
     *  - Logs flushed to all sinks if any log of level error or higher is encountered
     *  - No file logs
     *
     * @param format
     */
    inline void default_setup(const std::string &format = format::format_time_code)
    {
        set_console_to_stderr();
// Logger is info by default, set it to be same as compile level
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_OFF
        spdlog::set_level(spdlog::level::off);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
        spdlog::set_level(spdlog::level::trace);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
        spdlog::set_level(spdlog::level::debug);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
        spdlog::set_level(spdlog::level::info);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_WARN
        spdlog::set_level(spdlog::level::warn);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_ERROR
        spdlog::set_level(spdlog::level::err);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_CRITICAL
        spdlog::set_level(spdlog::level::critical);
#endif
        set_console_format(format);
        set_flush_severity(spdlog::level::err);
        // spdlog::flush_every(std::chrono::seconds(5));
    }

#ifdef EASYSPDLOG_AUTO_SETUP
    namespace
    {
        static bool ok()
        {
            volatile bool _ = true;  // avoid being optimised out
            if (_)
                easyspdlog::default_setup();
            return _;
        }

        static bool initialised = ok();
    }  // namespace
#endif

}  // namespace easyspdlog
