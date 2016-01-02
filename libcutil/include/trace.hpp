/**
 *  This file is part of LIBCUTIL.
 *
 *  LIBCUTIL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBCUTIL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */


#ifndef TRACE_HPP
#define TRACE_HPP


#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <assert.h>

namespace utils
{

/** @brief An object to write timestamped debug trace (logs) */
class Logable
{
public:
  Logable();
  Logable(const std::string &module);

  Logable(const std::string &root_module, const std::string &module);

  ~Logable();

  /** Setup the category of the log:
   *  @param module Typically project / component name.
   *  @param class  Typically class / function name */
  void setup(const std::string &module);
  void setup(const std::string &root_module, const std::string &module);

  inline void verbose(std::string s, ...) const
  {
#   ifdef DEBUG_MODE
    va_list ap;
    va_start(ap, s);
    verbose_internal(s, ap);
    va_end(ap);
#   endif
  }
  void trace  (std::string s, ...)       const;
  void trace_major(std::string s, ...)   const;
  void warning(std::string s, ...)       const;
  void anomaly(std::string s, ...)       const;

private:
  void verbose_internal(std::string s, va_list ap)   const;
  unsigned int module_id;
};

/** @cond not-documented */

/** @brief Abstract interface to an output receptacle for trace logs */
class Tracer
{
public:
  virtual ~Tracer(){}

  virtual void trace(int trace_level, std::string module, std::string message) = 0;
  virtual void flush() = 0;
};

/** @brief Trace to a log file */
class FileTracer: public Tracer
{
public:
  FileTracer();
  ~FileTracer();
  int set_log_file(const std::string &filename);
  void trace(int trace_level, std::string module, std::string message);
  void flush();
private:
  FILE *of;
  std::string filename;
};

/** @brief Trace to standard output */
class StdTracer: public Tracer
{
public:
  StdTracer();
  ~StdTracer();
  void trace(int trace_level, std::string module, std::string message);
  void flush();
private:
};

/** @endcond */


/** @brief The different trace levels (criticity) */
typedef enum
{
  /** @brief Verbose trace, for internal debugging only */
  AL_VERBOSE   = 0,

  /** @brief Normal trace level */
  AL_NORMAL    = 1,

  /** @brief Normal trace level */
  AL_MAJOR     = 2,

  /** @brief Non critical warning */
  AL_WARNING   = 3,

  /** @brief Critical */
  AL_ANOMALY   = 4,

  /** @brief To disable all trace levels */
  AL_NONE      = 5

} TraceLevel;


class TraceManager
{
public:

  /** @brief The different trace levels (criticity) */
  typedef enum trace_target_enum
  {
    /** Stdout / stderr */
    TRACE_TARGET_STD      = 0,

    /** Log file */
    TRACE_TARGET_FILE     = 1,

    TRACE_TARGET_RS232    = 2

  } TraceTarget;

  /** Enable / disable all traces */
  static void enable_all(bool enabled);

  /** Set global minimum trace level */
  static void set_global_min_level(TraceTarget target, TraceLevel min_level);

  static TraceLevel get_global_min_level(TraceTarget target);

  /** Setup trace modes */
  static void set_log_file(std::string filename);

  //static void enable_colors(bool enabled)

  static void trace(TraceLevel level, unsigned int module_id, std::string s, va_list ap);
  static void trace(TraceLevel level, std::string module, std::string s, va_list ap);
  static void trace(TraceLevel level, unsigned int module_id, std::string s, ...);
  static void trace(TraceLevel level, std::string module, std::string s, ...);
  // do nothing
  static void trace(TraceLevel level, unsigned int module_id) {}

  static uint32_t register_module(std::string module);
  static void     unregister_module(uint32_t module_id);


  static void set_abort_on_anomaly(bool abort);

  static uint32_t get_anomaly_count();
  static uint32_t get_warning_count();

  TraceManager();
  ~TraceManager();

  static TraceManager *get_instance();

private:
  friend class Tracer;
  friend class Logable;
  bool           global_enable;
  TraceLevel     global_min_levels[3];
  Tracer        *tracers[3];
  FileTracer     file_tracer;
  StdTracer      std_tracer;

  uint32_t       anomaly_count, warning_count;

  struct TraceModule
  {
    unsigned int id;
    std::string name;
  };

  std::vector<TraceModule> modules;

  static char *tmp_buffer;
};

extern unsigned int main_log;

#define log_verbose(HLOG, ...) utils::TraceManager::trace(utils::AL_VERBOSE, HLOG, ##__VA_ARGS__)
#define log_normal(HLOG, ...)  utils::TraceManager::trace(utils::AL_NORMAL, HLOG,  ##__VA_ARGS__)
#define log_major(HLOG, ...)   utils::TraceManager::trace(utils::AL_MAJOR, HLOG,   ##__VA_ARGS__)
#define log_warning(HLOG, ...) utils::TraceManager::trace(utils::AL_WARNING, HLOG, ##__VA_ARGS__)
#define log_anomaly(HLOG, ...) utils::TraceManager::trace(utils::AL_ANOMALY, HLOG, ##__VA_ARGS__)

/** @brief Display an error message in the log before asserting */
#define lassert(AA, ...) if(!(AA)){log_warning(0, ##__VA_ARGS__); log_anomaly(0,"Assertion failed: " #AA ".\nFile: %s, line: %d, func: %s", __FILE__, __LINE__, __func__); assert(AA);}

/** @brief Returns -1 if the condition is not true (and add an error message in the log) */
#define rassert(AA, ...) if(!(AA)){log_warning(0, ##__VA_ARGS__); log_anomaly(0,"Assertion failed: " #AA ".\nFile: %s, line: %d", __FILE__, __LINE__); return -1;}
//#define rassert(AA) if(!(AA)){log_anomaly(0,"Assertion failed: " #AA "."); return -1;}

#ifdef DEBUG_MODE
# define lassert_safe(...) lassert(__VA_ARGS__)
//# define lassert_safe(AA) if(!(AA)){log_anomaly(0,"Assertion failed: " #AA ".\nFile: %s, line: %d", __FILE__, __LINE__); assert(AA);}
#else
# define lassert_safe(...)
#endif

#ifdef DEBUG_MODE
# define rassert_safe(...) rassert(__VA_ARGS__)
#else
# define rassert_safe(...)
#endif

inline int applog_register(const std::string &name)
{
  return utils::TraceManager::register_module(name);
}

// TODO: inline function
#define applog(HLOG, LEVEL, ...) utils::TraceManager::trace(LEVEL, HLOG, __VA_ARGS__)

typedef int applog_t;

/*inline void syslog(int hlog, int level, ...)
{

}*/



} // namespace utils

#endif
