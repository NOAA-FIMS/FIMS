/**
 * @file def.hpp
 * @brief Platform macros and the core FIMS logging system.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef DEF_HPP
#define DEF_HPP
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <cstdlib>
#include <chrono>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <csignal>
#include <cstring>

#include <stdexcept>

#if defined(linux) || defined(__linux) || defined(__linux__)
#define FIMS_LINUX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#define FIMS_BSD
#elif defined(sun) || defined(__sun)
#define FIMS_SOLARIS
#elif defined(__sgi)
#define FIMS_IRIX
#elif defined(__hpux)
#define FIMS_HPUX
#elif defined(__CYGWIN__)
#define FIMS_CYGWIN
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define FIMS_WIN32
#elif defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
#define FIMS_WIN64
#elif defined(__BEOS__)
#define FIMS_BEOS
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define FIMS_MACOS
#elif defined(__IBMCPP__) || defined(_AIX)
#define FIMS_AIX
#elif defined(__amigaos__)
#define FIMS_AMIGAOS
#elif defined(__QNXNTO__)
#define FIMS_QNXNTO
#endif

#if defined(FIMS_WIN32) || defined(FIMS_WIN64)
#define FIMS_WINDOWS
#endif

#ifdef FIMS_WINDOWS
#include <Windows.h>
#include <Lmcons.h>  // for UNLEN
#elif defined(FIMS_LINUX) || defined(FIMS_MACOS) || defined(FIMS_BSD)
#include <unistd.h>
#include <pwd.h>
#endif

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#ifdef FIMS_WINDOWS
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
#endif

// The following rows initialize default log files for outputting model progress
// comments used to assist in diagnosing model issues and tracking progress.

#ifdef TMB_MODEL
// simplify access to singletons
#define TMB_FIMS_REAL_TYPE double
#define TMBAD_FIMS_TYPE TMBad::ad_aug
#endif

namespace fims {

/**
 * @brief A data structure with defined fields for a single log record.
 *
 * @details At run time, each log entry is stored in this structure with
 * information on timestamp, severity level, message text, sequence id,
 * username, working directory, source file, routine, and source line.
 */
struct LogEntry {
  /**
   * @brief The date and time that the log entry was created.
   * @details Example: "Oct 28 09:18:51 2024". You can track how long it took
   * to work through each portion of the model by analyzing the progression of
   * the timestamp through the log file.
   */
  std::string timestamp;
  /**
   * @brief The description of the log entry.
   * @details Example messages include "Adding Selectivity object to TMB" or
   * "Mismatch dimension error", where descriptions are predefined in the C++
   * code.
   */
  std::string message;
  /**
   * @brief The logging level associated with the entry.
   * @details The level is determined by the macro used to generate the
   * message, for example FIMS_INFO_LOG(), FIMS_WARNING_LOG(), or
   * FIMS_ERROR_LOG(), which map to "info", "warning", and "error",
   * respectively.
   */
  std::string level;
  /**
   * @brief The message identifier corresponding to creation order.
   * @details Example: "1". This helps track operation ordering across model
   * runs.
   */
  size_t rank;
  /**
   * @brief The user name registered on the machine where the log was created.
   * @details Example: "John.Doe".
   */
  std::string user;
  /**
   * @brief The working directory for the environment that created the log.
   * @details Example on Windows: "C:/github/NOAA-FIMS/FIMS/vignettes".
   * Example on Linux: "/home/oppy/FIMS-Testing/dev/dev_logging/FIMS/vignettes".
   */
  std::string wd;
  /**
   * @brief The full file path of the file that triggered the log entry.
   * @details Example:
   * "C:/github/NOAA-FIMS/FIMS/inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp".
   */
  std::string file;
  /**
   * @brief The function or method that initiated the log entry.
   * @details Example: "virtual bool
   * LogisticSelectivityInterface::add_to_fims_tmb()". For templated functions,
   * type information is reported in square brackets, for example: "bool
   * fims_info::Information<Type>::CreateModel() [with Type = double]".
   */
  std::string routine;
  /**
   * @brief The line in file where the log entry was initiated.
   * @details Example: "219", which is a line inside the routine listed above.
   */
  int line;

  /**
   * @brief Serialize this entry to a JSON object string.
   * @return A JSON object represented as a string (without trailing comma).
   */
  std::string to_string() {
    std::stringstream ss;
    ss << "\"timestamp\": " << "\"" << this->timestamp << "\"" << ",\n";
    ss << "\"level\": " << "\"" << this->level << "\",\n";
    ss << "\"message\": " << "\"" << this->message << "\",\n";
    ss << "\"id\": " << "\"" << this->rank << "\",\n";
    ss << "\"user\": " << "\"" << this->user << "\",\n";
    ss << "\"wd\": " << "\"" << this->wd << "\",\n";
    ss << "\"file\": " << "\"" << this->file << "\",\n";
    ss << "\"routine\": " << "\"" << this->routine << "\",\n";
    ss << "\"line\": " << "\"" << this->line << "\"\n";
    return ss.str();
  }
};

/**
 * @brief Singleton logger for FIMS.
 *
 * @details `FIMSLog` accumulates log entries in memory and provides
 * JSON-formatted accessors for all entries and severity-specific subsets.
 *
 * Runtime behavior:
 * - `write_on_exit = true` writes the current log buffer to disk in the
 *   destructor.
 * - `throw_on_error = true` throws a `std::runtime_error` after recording an
 *   error-level entry.
 */
class FIMSLog {
  std::vector<std::string> entries;
  std::vector<LogEntry> log_entries;
  size_t entry_number = 0;
  std::string path = "fims.log";
  size_t warning_count = 0;
  size_t error_count = 0;

  /**
   * Get username.
   *
   * @return username.
   */
  std::string get_user() {
#ifdef FIMS_WINDOWS
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
      return std::string(username);
    } else {
      return "[unknown user]";
    }

#elif defined(FIMS_LINUX) || defined(FIMS_MACOS) || defined(FIMS_BSD)
    const char* user_env = getenv("USER");
    if (user_env) return std::string(user_env);

    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) {
      return std::string(pw->pw_name);
    } else {
      return "[unknown user]";
    }

#else
    return "[unsupported platform]";
#endif
  }

 public:
  /**
   * @brief A boolean specifying if the log file is written when the session is
   * terminated. The default is TRUE.
   *
   */
  bool write_on_exit = true;
  /**
   * @brief A boolean specifying if the program is stopped upon the first
   * error, where the default is FALSE. This allows you go through an entire
   * program to collect all error messages.
   *
   */
  bool throw_on_error = false;
  /**
   * @brief A singleton instance of the log, i.e., where there is only one
   * log. The object is created when the .dll is loaded and it will never
   * be recreated while the .dll is loaded.
   *
   */
  static std::shared_ptr<FIMSLog> fims_log;

  /**
   * Default constructor for FIMSLog.
   */
  FIMSLog() {}

  /**
   * Destructor. If write_on_exit is set to true,
   * the log will be written to the disk in JSON format.
   */
  ~FIMSLog() {
    if (this->write_on_exit) {
      std::ofstream log(this->path);
      log << this->get_log();
      log.close();
    }
  }

  /**
   * @brief Get the absolute path without dot dot notation.
   *
   * Dot dot notation is for relative paths, where this function replaces
   * all dot dots with the actual full path.
   *
   * @param relativePath A path in your file system.
   * @return std::filesystem::path
   */
  std::filesystem::path getAbsolutePathWithoutDotDot(
      const std::filesystem::path& relativePath) {
    std::filesystem::path absolutePath =
        std::filesystem::absolute(relativePath);

    std::filesystem::path result;
    for (const auto& part : absolutePath) {
      if (part == "..") {
        if (!result.empty()) {
          result = result.parent_path();
        }
      } else {
        result /= part;
      }
    }

    return result.generic_string();
  }

  /**
   * @brief Set the destination file path for writing logs to disk.
   *
   * @details The configured path is used whenever the logger writes to disk,
   * including destructor-time writes (when `write_on_exit` is `true`) and
   * signal-triggered writes in `WriteAtExit()`.
   *
   * This method updates only the output location and does not clear or modify
   * the in-memory log buffer.
   *
   * @param path Relative or absolute path to the output log file (for example,
   * `"fims.log"` or `"logs/fims_run_01.json"`).
   * @see get_path()
   * @see write_on_exit
   */
  void set_path(std::string path) { this->path = path; }

  /**
   * @brief Get the current output path for on-disk logs.
   *
   * @return Output file path.
   */
  std::string get_path() { return this->path; }

  /**
   * Add a "info", "error", or "warning" level message to the log.
   *
   * @param str Log message text.
   * @param line Source line number where the message originated.
   * @param file Source file where the message originated.
   * @param func Function or method name where the message originated.
   */
  void info_message(std::string str, int line, const char* file,
                    const char* func) {
    std::filesystem::path relativePath = file;
    std::filesystem::path absolutePath =
        getAbsolutePathWithoutDotDot(relativePath);
    std::filesystem::path cwd = std::filesystem::current_path();
    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string ctime_no_newline = strtok(ctime(&now_time), "\n");

    LogEntry l;
    l.timestamp = ctime_no_newline;
    l.message = str;
    l.level = "info";
    l.rank = this->log_entries.size();
    l.user = this->get_user();
    l.wd = cwd.generic_string();
    l.file = absolutePath.string();
    l.line = line;
    l.routine = func;
    this->log_entries.push_back(l);
  }

  /**
   * @copydoc info_message
   */
  void error_message(std::string str, int line, const char* file,
                     const char* func) {
    this->error_count++;
    std::filesystem::path relativePath = file;
    std::filesystem::path absolutePath =
        getAbsolutePathWithoutDotDot(relativePath);
    std::filesystem::path cwd = std::filesystem::current_path();

    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string ctime_no_newline = strtok(ctime(&now_time), "\n");

    LogEntry l;
    l.timestamp = ctime_no_newline;
    l.message = str;
    l.level = "error";
    l.rank = this->log_entries.size();
    l.user = this->get_user();
    l.wd = cwd.generic_string();
    l.file = absolutePath.string();
    l.line = line;
    l.routine = func;
    this->log_entries.push_back(l);

    if (this->throw_on_error) {
      std::stringstream ss;
      ss << "\n\n" << l.to_string() << "\n\n";
      throw std::runtime_error(ss.str().c_str());
    }
  }

  /**
   * @copydoc info_message
   */
  void warning_message(std::string str, int line, const char* file,
                       const char* func) {
    this->warning_count++;
    std::filesystem::path relativePath = file;
    std::filesystem::path absolutePath =
        getAbsolutePathWithoutDotDot(relativePath);
    std::filesystem::path cwd = std::filesystem::current_path();

    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string ctime_no_newline = strtok(ctime(&now_time), "\n");

    LogEntry l;
    l.timestamp = ctime_no_newline;
    l.message = str;
    l.level = "warning";
    l.rank = this->log_entries.size();
    l.user = this->get_user();
    l.wd = cwd.generic_string();
    l.file = absolutePath.string();
    l.line = line;
    l.routine = func;
    this->log_entries.push_back(l);
  }

  /**
   * @brief Return all stored log entries as a JSON array string.
   *
   * @details
   * The returned value is a JSON array of serialized `LogEntry` objects in the
   * same order they were recorded.
   *
   * If no entries are stored, this method returns an empty JSON array (`[]`).
   *
   * @see get_errors()
   * @see get_warnings()
   * @see get_info()
   * @return JSON array string containing every stored `LogEntry`.
   */
  std::string get_log() {
    std::stringstream ss;
    if (log_entries.size() == 0) {
      ss << "[\n]";
    } else {
      ss << "[\n";
      for (size_t i = 0; i < log_entries.size() - 1; i++) {
        ss << "{\n" << this->log_entries[i].to_string() << "},\n";
      }
      ss << "{\n"
         << this->log_entries[log_entries.size() - 1].to_string() << "}\n]";
    }
    return ss.str();
  }

  /**
   * @brief Return only error-level log entries as a JSON array string.
   *
   * @details
   * This method filters the in-memory log buffer and includes only entries
   * where `level == "error"`.
   *
   * If no error entries exist, this method returns an empty JSON array (`[]`).
   *
   * @see get_log()
   * @see get_warnings()
   * @see get_info()
   * @return JSON array string containing entries with `level == "error"`.
   */
  std::string get_errors() {
    std::stringstream ss;
    std::vector<LogEntry> errors;
    for (size_t i = 0; i < log_entries.size(); i++) {
      if (log_entries[i].level == "error") {
        errors.push_back(this->log_entries[i]);
      }
    }

    if (errors.size() == 0) {
      ss << "[\n]";
    } else {
      ss << "[\n";
      for (size_t i = 0; i < errors.size() - 1; i++) {
        ss << "{\n" << errors[i].to_string() << "},\n";
      }

      ss << "{\n" << errors[errors.size() - 1].to_string() << "}\n]";
    }
    return ss.str();
  }

  /**
   * @brief Return only warning-level log entries as a JSON array string.
   *
   * @details
   * This method filters the in-memory log buffer and includes only entries
   * where `level == "warning"`.
   *
   * If no warning entries exist, this method returns an empty JSON array
   * (`[]`).
   *
   * @see get_log()
   * @see get_errors()
   * @see get_info()
   * @return JSON array string containing entries with `level == "warning"`.
   */
  std::string get_warnings() {
    std::stringstream ss;
    std::vector<LogEntry> warnings;
    for (size_t i = 0; i < log_entries.size(); i++) {
      if (log_entries[i].level == "warning") {
        warnings.push_back(this->log_entries[i]);
      }
    }

    if (warnings.size() == 0) {
      ss << "[\n]";
    } else {
      ss << "[\n";
      for (size_t i = 0; i < warnings.size() - 1; i++) {
        ss << "{\n" << warnings[i].to_string() << "},\n";
      }

      ss << "{\n" << warnings[warnings.size() - 1].to_string() << "}\n]";
    }
    return ss.str();
  }

  /**
   * @brief Return only info-level log entries as a JSON array string.
   *
   * @details
   * This method filters the in-memory log buffer and includes only entries
   * where `level == "info"`.
   *
   * If no info entries exist, this method returns an empty JSON array (`[]`).
   *
   * @see get_log()
   * @see get_errors()
   * @see get_warnings()
   * @return JSON array string containing entries with `level == "info"`.
   */
  std::string get_info() {
    std::stringstream ss;
    std::vector<LogEntry> info;
    for (size_t i = 0; i < log_entries.size(); i++) {
      if (log_entries[i].level == "info") {
        info.push_back(this->log_entries[i]);
      }
    }

    if (info.size() == 0) {
      ss << "[\n]";
    } else {
      ss << "[\n";
      for (size_t i = 0; i < info.size() - 1; i++) {
        ss << "{\n" << info[i].to_string() << "},\n";
      }

      ss << "{\n" << info[info.size() - 1].to_string() << "}\n]";
    }
    return ss.str();
  }

  /**
   * @brief Return the number of error or warning log entries currently stored.
   *
   * @details This value is reset to zero when `clear()` is called.
   * @see clear()
   * @return Count of error-level entries.
   */
  size_t get_error_count() const { return error_count; }

  /**
   * @copydoc get_error_count
   */
  size_t get_warning_count() const { return warning_count; }

  /**
   * @brief Clear in-memory logging state.
   *
   * @details Clears the raw entry cache and structured entries, resets warning
   * and entry counters, and preserves configured output path and
   * `throw_on_error` behavior.
   */
  void clear() {
    this->entries.clear();
    this->log_entries.clear();
    this->warning_count = 0;
    this->entry_number = 0;
  }
};

std::shared_ptr<FIMSLog> FIMSLog::fims_log = std::make_shared<FIMSLog>();

}  // namespace fims

/**
 * @def FIMS_INFO_LOG(MESSAGE)
 * @brief Record an info, warning, or error log entry with metadata.
 *
 * @details The logging macros capture `MESSAGE` plus the call-site metadata
 * (`__LINE__`, `__FILE__`, and `__PRETTY_FUNCTION__`) and forward those values
 * to the appropriate logger function. Each function type, i.e., `INFO`,
 * `WARNING`, and `ERROR` lead to different "level" entries within the log
 * entry, where `level = "warning"` does not increment a counter whereas the
 * other two options lead to increased warning or error counts.
 *
 * @param MESSAGE Human-readable log message describing what happened and why.
 */
#define FIMS_INFO_LOG(MESSAGE)                                       \
  fims::FIMSLog::fims_log->info_message(MESSAGE, __LINE__, __FILE__, \
                                        __PRETTY_FUNCTION__);

/**
 * @def FIMS_WARNING_LOG(MESSAGE)
 * @copydoc FIMS_INFO_LOG(MESSAGE)
 */
#define FIMS_WARNING_LOG(MESSAGE)                                       \
  fims::FIMSLog::fims_log->warning_message(MESSAGE, __LINE__, __FILE__, \
                                           __PRETTY_FUNCTION__);

/**
 * @def FIMS_ERROR_LOG(MESSAGE)
 * @copydoc FIMS_INFO_LOG(MESSAGE)
 */
#define FIMS_ERROR_LOG(MESSAGE)                                       \
  fims::FIMSLog::fims_log->error_message(MESSAGE, __LINE__, __FILE__, \
                                         __PRETTY_FUNCTION__);

/**
 * @def FIMS_STR(s)
 * @brief Convert a preprocessor token to a string literal.
 * @param s Input text from the macro that will be converted to a string.
 */
#define FIMS_STR(s) #s

namespace fims {

/**
 * @brief Signal handler that records a terminal error and flushes log entries.
 *
 * @details On receipt of a supported signal, this function appends an
 * error-level entry, writes the full log if `write_on_exit` is enabled,
 * restores the default signal handler, and re-raises the signal.
 *
 * @param sig Integer signal identifier provided by the operating system when
 * this handler is called (for example, SIGSEGV for invalid memory access,
 * SIGINT for an interrupt such as Ctrl+C, or SIGTERM for a termination
 * request).
 */
void WriteAtExit(int sig) {
  std::string signal_error = "NA";
  switch (sig) {
    case SIGSEGV:
      signal_error = "Invalid memory access (segmentation fault)";
      break;
    case SIGINT:
      signal_error = "External interrupt, possibly initiated by the user.";
      break;
    case SIGABRT:
      signal_error =
          "Abnormal termination condition, possible call to std::abort.";
      break;
    case SIGFPE:
      signal_error = "Erroneous arithmetic operation.";
      break;
    case SIGILL:
      signal_error = "Invalid program image or invalid instruction";
      break;
    case SIGTERM:
      signal_error = "Termination request, sent to the program.";
      break;
    default:
      signal_error = "Unknown signal thrown";
  }

  FIMSLog::fims_log->error_message(signal_error, -999, "?", "?");

  if (FIMSLog::fims_log->write_on_exit) {
    std::ofstream log(FIMSLog::fims_log->get_path());
    log << FIMSLog::fims_log->get_log();
    log.close();
  }
  std::signal(sig, SIG_DFL);
  raise(sig);
}

/**
 * Converts an object T to a string.
 *
 * @details The object `v` of type `T` must be able to be written to a
 * `std::ostream` with the `<<` operator.
 * @param v Value to convert to text using a string stream.
 * @return String representation of `v`.
 */
template <typename T>
std::string to_string(T v) {
  std::stringstream ss;
  ss << v;
  return ss.str();
}

}  // namespace fims

#endif /* TRAITS_HPP */
