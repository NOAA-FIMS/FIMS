/**
 * @file def.hpp
 * @brief Creates pre-processing macros such as what type of machine you are on
 * and creates the log information.
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
// These files will only be created if a logs folder is added to the root model
// directory.

#ifdef TMB_MODEL
// simplify access to singletons
#define TMB_FIMS_REAL_TYPE double
#ifdef TMBAD_FRAMEWORK
#define TMBAD_FIMS_TYPE TMBad::ad_aug
#else
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE>
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER>
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER>
#endif
#endif

namespace fims {

/**
 * Log entry.
 */
struct LogEntry {
  /** The date/time that the log entry was created, e.g., "Oct 28 09:18:51
   * 2024". You can track how long it took to work through each portion of the
   * model by analyzing the progression of the timestamp through the log file.*/
  std::string timestamp;
  /** The description of the log entry, e.g., "Adding Selectivity object to TMB"
   * or "Mismatch dimension error", where the descriptions are predefined in the
   * C++ code. Please make a GitHub issue or contact a developer if you have
   * ideas for a more informative description.*/
  std::string message;
  /** The logging level, which is a result of which macro was used to generate
   * the message, e.g., FIMS_INFO_LOG(), FIMS_WARNING_LOG(), or FIMS_ERROR_LOG()
   * results in "info", "warning", or "error", respectively, in the log file. An
   * additional level is available to developers from FIMS_DEBUG_LOG(),
   * resulting in a level of "debug", but this macro is only available in
   * branches other than main.*/
  std::string level;
  /** The message id, directly corresponds to the order in which the entries
   * were created, e.g., "1", which is helpful for knowing the order of
   * operations within the code base and comparing log files across model
   * runs.*/
  size_t rank;
  /** The user name registered to the computer where the log file was created,
   * e.g., "John.Doe".*/
  std::string user;
  /** The working directory for the R environment that created the log file,
   * e.g., "C:/github/NOAA-FIMS/FIMS/vignettes" if you are on a Windows machine
   * or "/home/oppy/FIMS-Testing/dev/dev_logging/FIMS/vignettes" if you are on a
   * linux machine.*/
  std::string wd;
  /** The full file path of the file that triggered the log entry, e.g.,
   * "C:/github/NOAA-FIMS/FIMS/inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp".*/
  std::string file;
  /** The function or method that led to the initialization the log entry, e.g.,
   * "virtual bool LogisticSelectivityInterface::add_to_fims_tmb()". If the
   * function is templated, then the function type will be reported here in
   * square brackets after the function name, e.g., "bool
   * fims_info::Information<Type>::CreateModel() [with Type = double]".*/
  std::string routine;
  /** The line in `file` where the log entry was initiated, e.g., "219", which
   * will be a line inside of the `routine` listed above.*/
  int line;

  /**
   * Convert this object to a string.
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
 * FIMS logging class.
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
   * @brief Get the Absolute Path Without Dot Dot object
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
   * Set a path for the log file.
   *
   * @param path
   */
  void set_path(std::string path) { this->path = path; }

  /**
   * Get the path for the log file.
   *
   * @return
   */
  std::string get_path() { return this->path; }

  /**
   * Add a "info" level message to the log.
   *
   * @param str
   * @param line
   * @param file
   * @param func
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
   * Add a "debug" level message to the log.
   *
   * @param str
   * @param line
   * @param file
   * @param func
   */
  void debug_message(std::string str, int line, const char* file,
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
    l.level = "debug";
    l.rank = this->log_entries.size();
    l.user = this->get_user();
    l.wd = cwd.generic_string();
    l.file = absolutePath.string();
    l.line = line;
    l.routine = func;
    this->log_entries.push_back(l);
  }

  /**
   * Add a "error" level message to the log.
   *
   * @param str
   * @param line
   * @param file
   * @param func
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
   * Add a "warning" level message to the log.
   *
   * @param str
   * @param line
   * @param file
   * @param func
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
   * Get the log as a string object.
   *
   * @return
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
   * Return only error entries from the log.
   *
   * @return
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
   * Return only warning entries from the log.
   *
   * @return
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
   * Return only info entries from the log.
   *
   * @return
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
   * Query the log by module.
   *
   * @param module
   * @return
   */
  std::string get_module(const std::string& module) {
    std::stringstream ss;
    std::vector<LogEntry> info;
    for (size_t i = 0; i < log_entries.size(); i++) {
      if (log_entries[i].file.find(module) != std::string::npos) {
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
   * @brief Get the counts of the number of errors
   */
  size_t get_error_count() const { return error_count; }

  /**
   * @brief Get the counts of the number of warnings
   */
  size_t get_warning_count() const { return warning_count; }

  /**
   * @brief Clears all pointers/references of a FIMS model.
   *
   */
  void clear() {
    this->entries.clear();
    this->log_entries.clear();
    this->warning_count = 0;
    this->entry_number = 0;
  }
};

inline std::shared_ptr<FIMSLog> FIMSLog::fims_log = std::make_shared<FIMSLog>();

}  // namespace fims

#ifdef FIMS_DEBUG

#define FIMS_DEBUG_LOG(MESSAGE)                                 \
  FIMSLog::fims_log->debug_message(MESSAGE, __LINE__, __FILE__, \
                                   __PRETTY_FUNCTION__);

#else

#define FIMS_DEBUG_LOG(MESSAGE) /**< Print MESSAGE to debug log */

#endif

#define FIMS_INFO_LOG(MESSAGE)           \
  fims::FIMSLog::fims_log->info_message( \
      MESSAGE, __LINE__, __FILE__,       \
      __PRETTY_FUNCTION__); /**< Print MESSAGE to info log */

#define FIMS_WARNING_LOG(MESSAGE)           \
  fims::FIMSLog::fims_log->warning_message( \
      MESSAGE, __LINE__, __FILE__,          \
      __PRETTY_FUNCTION__); /**< Print MESSAGE to warning log */

#define FIMS_ERROR_LOG(MESSAGE)           \
  fims::FIMSLog::fims_log->error_message( \
      MESSAGE, __LINE__, __FILE__,        \
      __PRETTY_FUNCTION__); /**< Print MESSAGE to error log */

#define FIMS_STR(s) #s /**< String of s */

namespace fims {

/**
 * Signal intercept function. Writes the log to the disk before
 * a crash occurs.
 *
 * @param sig
*/
inline void WriteAtExit(int sig) {
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
 * @param v
 * @return
 */
template <typename T>
std::string to_string(T v) {
  std::stringstream ss;
  ss << v;
  return ss.str();
}

}  // namespace fims

#endif /* TRAITS_HPP */
