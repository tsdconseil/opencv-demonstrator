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



/** Debug printf (disabled at global niveau by default, to enable,
 *  use, "set_debug_options" function below). */
//extern void verbose(const char *s, ...);
//extern void infos(const char *s, ...);
//extern void trace_majeure(const char *s, ...);
//extern void avertissement(const char *s, ...);
//extern void erreur(const char *s, ...);




namespace journal
{

  /** @brief The different infos levels (criticity) */
  typedef enum
  {
    /** @brief Verbose infos, for internal debugging only */
    AL_VERBOSE   = 1,

    /** @brief Normal infos niveau */
    AL_NORMAL    = 2,

    /** @brief Normal infos niveau */
    AL_MAJOR     = 4,

    /** @brief Non critical warning */
    AL_WARNING   = 8,

    /** @brief Critical */
    AL_ANOMALY   = 16,

    /** @brief To disable all infos levels */
    AL_NONE      = 0

  } TraceLevel;


  /** @brief Destination des traces */
  typedef enum trace_target_enum
  {
    /** Stdout / stderr */
    TRACE_TARGET_STD      = 0,

    /** Log file */
    TRACE_TARGET_FILE     = 1,

    TRACE_TARGET_NONE     = 2
  } TraceTarget;

  /** @brief An object to write timestamped debug infos (logs) */
  class Logable
  {
  public:
    Logable(const std::string &module = "");

    /** Setup the category of the log:
     *  @param module Typically project / component name. */
    void setup(const std::string &module);
  //private:
    std::string module_id;
    void set_min_level(TraceTarget target, TraceLevel min_level);

    TraceLevel     min_levels[2];
  };


  extern void gen_trace(TraceLevel niveau, const std::string &module, const Logable &log, const std::string &s, va_list ap);
  extern void gen_trace(TraceLevel niveau, const std::string &module, const Logable &log, const std::string &s, ...);
  extern void gen_trace(TraceLevel niveau, const std::string &module, const std::string &s, ...);

  /** Setup infos modes */
  extern void set_log_file(std::string filename);


  /** Enable / disable all traces */
  extern void enable_all(bool enabled);

  /** Set global minimum infos niveau */
  extern void set_global_min_level(TraceTarget target, TraceLevel min_level);

  extern TraceLevel get_global_min_level(TraceTarget target);

  extern void set_abort_on_anomaly(bool abort);

  extern uint32_t get_anomaly_count();
  extern uint32_t get_warning_count();


  /** @brief Abstract interface to an output receptacle for infos logs */
  class Tracer
  {
  public:
    virtual ~Tracer(){}
    virtual void gen_trace(int trace_level, const std::string &module, const std::string &message) = 0;
    virtual void flush() = 0;
  };


  extern Logable journal_principal;

//#ifdef DEBUG_MODE
# define trace_verbeuse(...) utils::journal::gen_trace(utils::journal::AL_VERBOSE, __func__, __VA_ARGS__)
//#else
//# define trace_verbeuse(...)
//#endif
#define infos(...)          utils::journal::gen_trace(utils::journal::AL_NORMAL,  __func__, __VA_ARGS__)
#define trace_majeure(...)  utils::journal::gen_trace(utils::journal::AL_MAJOR,   __func__, __VA_ARGS__)
#define avertissement(...)  utils::journal::gen_trace(utils::journal::AL_WARNING, __func__, __VA_ARGS__)
#define erreur(...)         utils::journal::gen_trace(utils::journal::AL_ANOMALY, __func__, __VA_ARGS__)


/** @brief Display an error message in the log before asserting */
#define lassert(AA, ...) if(!(AA)){utils::journal::gen_trace(utils::journal::AL_WARNING, __func__, ##__VA_ARGS__); utils::journal::gen_trace(utils::journal::AL_ANOMALY, __func__, "Assertion failed: " #AA ".\nFile: %s, line: %d, func: %s", __FILE__, __LINE__, __func__); assert(AA);}

/** @brief Returns -1 if the condition is not true (and add an error message in the log) */
#define rassert(AA, ...) if(!(AA)){utils::journal::gen_trace(utils::journal::AL_WARNING, __func__, ##__VA_ARGS__); utils::journal::gen_trace(utils::journal::AL_ANOMALY, __func__, "Assertion failed: " #AA ".\nFile: %s, line: %d", __FILE__, __LINE__); return -1;}


#ifdef DEBUG_MODE
# define lassert_safe(...) lassert(__VA_ARGS__)
#else
# define lassert_safe(...)
#endif

#ifdef DEBUG_MODE
# define rassert_safe(...) rassert(__VA_ARGS__)
#else
# define rassert_safe(...)
#endif


  // infos("bla bla %d", 54);
  // infos(log, "bla bla %d", 54);

}

} // namespace utils

#endif
