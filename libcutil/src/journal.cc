#include "cutil.hpp"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>

#include "../include/journal.hpp"
#ifdef LINUX
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <linux/unistd.h>
#endif
#include <iostream>
#include <csignal>

//#define DISPLAY_TIME
#ifdef DISPLAY_TIME
#include <boost/date_time/posix_time/posix_time.hpp>
#endif

#include <stdexcept>

using namespace std;

namespace utils
{


namespace journal
{


Logable journal_principal;

/** @brief Trace to a log file */
class FileTracer: public Tracer
{
public:
  FileTracer();
  ~FileTracer();
  int set_log_file(const std::string &filename);
  void gen_trace(int trace_level, const std::string &module, const std::string &message);
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
  void gen_trace(int trace_level, const std::string &module, const std::string &message);
  void flush();
private:
};

struct TraceData
{
  TraceData();
  friend class Tracer;
  friend class Logable;
  bool           global_enable;
  TraceLevel     global_min_levels[2];
  Tracer        *tracers[2];
  FileTracer     file_tracer;
  StdTracer      std_tracer;
  uint32_t       anomaly_count, warning_count;
  char *tmp_buffer;
  bool abort_on_anomaly, throw_on_anomaly;
  #ifndef DISPLAY_TIME
  bool date_base_ok;
  uint64_t date_base;
  #endif
};

static TraceData instance;



/*
#ifdef LINUX
hal::Mutex tm_mutex_;
#define tm_mutex (&tm_mutex_)
#else
hal::Mutex *tm_mutex;
#endif
*/

hal::Mutex tm_mutex_;
#define tm_mutex (&tm_mutex_)
#define TRACE_BSIZE (10*1024)

TraceData::TraceData()
{
  abort_on_anomaly = throw_on_anomaly = false;
# ifndef DISPLAY_TIME
  date_base_ok  = false;
  date_base = 0;
# endif
  warning_count = 0;
  anomaly_count = 0;
  global_enable = true;
  global_min_levels[0] = AL_VERBOSE;
  global_min_levels[1] = AL_NONE;
  tracers[0] = &std_tracer;
  tracers[1] = &file_tracer;
  tmp_buffer = (char *) malloc(TRACE_BSIZE);
}



uint32_t get_anomaly_count()
{
  return instance.anomaly_count;
}

uint32_t get_warning_count()
{
  return instance.warning_count;
}

void set_abort_on_anomaly(bool abort)
{
  instance.abort_on_anomaly = abort;
}




void enable_all(bool enable)
{
  instance.global_enable = enable;
}

void set_log_file(std::string filename)
{
  instance.file_tracer.set_log_file(filename);
}

TraceLevel get_global_min_level(TraceTarget target)
{
  uint32_t tr = (uint32_t) target;

  if(tr < 2)
    return instance.global_min_levels[tr];

  return AL_NONE;
}

void Logable::set_min_level(TraceTarget target, TraceLevel min_level)
{
  uint32_t tr = (uint32_t) target;

  if(tr < 2)
  {
    min_levels[tr] = min_level;
  }
}

void set_global_min_level(TraceTarget target, TraceLevel min_level)
{
  uint32_t tr = (uint32_t) target;

  if(tr < 2)
  {
    instance.global_min_levels[tr] = min_level;
  }
}




int FileTracer::set_log_file(const std::string &filename)
{
  if(utils::files::file_exists(filename))
    utils::files::copy_file(filename + ".old", filename);


  this->filename = filename;
  if(of != nullptr)
    fclose(of);
  of = fopen(filename.c_str(), "wt");
  return (of == nullptr) ? -1 : 0;
}

FileTracer::FileTracer()
{
  of = nullptr;
  //set_log_file("./infos.log");
}

FileTracer::~FileTracer()
{
  if(of != nullptr)
    fclose(of);
}

void FileTracer::flush()
{
  fflush(of);
}

void FileTracer::gen_trace(int trace_level, const std::string &module, const std::string &message)
{
  if(of != nullptr)
  {
#ifndef DISPLAY_TIME
    if(!instance.date_base_ok)
    {
      instance.date_base_ok = true;
      instance.date_base = hal::get_tick_count_us();
    }
#endif
    {
#ifndef DISPLAY_TIME
      uint64_t ticks = hal::get_tick_count_us() - instance.date_base;
      uint32_t seconds = ticks / (1000 * 1000);
      ticks -= seconds * 1000 * 1000;
      uint32_t ms      = ticks / 1000;
      ticks -= ms * 1000;
      uint32_t us = (uint32_t) ticks;
#endif
      if(trace_level == AL_WARNING)
        fprintf(of, "-- AVERTISSEMENT : ");
      else if(trace_level == AL_ANOMALY)
        fprintf(of, "-- ANOMALIE : ");
#ifdef DISPLAY_TIME
//       boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::universal_time();
      boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
      std::string time_str = boost::posix_time::to_iso_extended_string(time_now);
      fprintf(of, "%s: ", time_str.c_str());
#else
      fprintf(of, "%4u,%03d,%03d: ", seconds, ms, us);
#endif

      fprintf(of, "[%s] %s\n", module.c_str(), message.c_str());

      //if(trace_level >= AL_NORMAL)
      {
        fflush(of);
      }


      if(ferror(of))
      {
        cerr << "Error while writing in the log file: closing & restart." << endl;
        clearerr(of);
        fclose(of);
        of = fopen(filename.c_str(), "wt");
      }
    }
  }
}

StdTracer::StdTracer()
{
}

StdTracer::~StdTracer()
{
}

void StdTracer::gen_trace(int trace_level, const std::string &module, const std::string &message)
{
  FILE *fout = (trace_level < AL_WARNING) ? stdout : stderr;

  ostream *out = &(std::cout);
  //FILE *output = stdout;
  char color[30];

  if(trace_level >= AL_WARNING)
    out = &(std::cerr); //output = stderr;

  switch(trace_level)
  {
    case AL_VERBOSE:
    {
      sprintf(color, "34");
      break;
    }
    case AL_NORMAL:
    {
      sprintf(color, "30");
      break;
    }
    case AL_MAJOR:
    {
      sprintf(color, "1");
      break;
    }
    case AL_WARNING:
    {
      sprintf(color, "31");
      break;
    }
    case AL_ANOMALY:
    {
      //sprintf(color, "31");
      sprintf(color, "1;37;41");
      break;
    }
  }

#ifndef DISPLAY_TIME
  if(!instance.date_base_ok)
  {
    instance.date_base_ok = true;
    instance.date_base = hal::get_tick_count_us();
  }
#endif
  {
#ifdef DISPLAY_TIME
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
    std::string time_str = boost::posix_time::to_iso_extended_string(time_now);
    fprintf(output, "%s: ", time_str.c_str());
#else
    uint64_t new_ticks = hal::get_tick_count_us();
    uint64_t ticks =  new_ticks - instance.date_base;
    uint32_t seconds = ticks / (1000 * 1000);
    ticks -= seconds * 1000 * 1000;
    uint32_t ms      = ticks / 1000;
    ticks -= ms * 1000;
    uint32_t us = (uint32_t) ticks;

    fprintf(fout, "%4u,%03u,%03u ", seconds, ms, us);
    //*out << seconds << "," << ms << "," << us;
#endif

    /*#ifdef LINUX
    fprintf(output, " %x ", (unsigned int) pthread_self());///gettid());
    #endif*/


    *out << " \033[" << color << "m";

    if(module.size() > 0)
      *out << "[" << module << "] ";

    *out << message << "\033[0m" << std::endl;

    //fprintf(output, "\033[%sm[%s] %s\033[0m\n", color, module.c_str(), message.c_str());

    //if(trace_level >= TraceManager::TRACE_LEVEL_WARNING)
//      fflush(output);

    /*if(seconds > 1000000)
    {
      fprintf(output, "Counter overflow: new ticks = %llu, base = %llu, diff = %llu, secs = %u.\n",
              (long long unsigned int) new_ticks, 
              (long long unsigned int) date_base, 
              (long long unsigned int) ticks, seconds);
    }*/

  }
}

void StdTracer::flush()
{
  std::cerr.flush();
  std::cout.flush();
}


void gen_trace(TraceLevel niveau,
               const std::string &fonction,
               const Logable &log,
               const std::string &s,
               va_list ap)
{
  bool somewhere_to_trace = false;

  instance.anomaly_count += (niveau == AL_ANOMALY) ? 1 : 0;
  instance.warning_count += (niveau == AL_WARNING) ? 1 : 0;

  if(!instance.global_enable)
    return;

  for(uint32_t i = 0; i < 2; i++)
  {
    if((niveau >= instance.global_min_levels[i]) && (niveau >= log.min_levels[i]))
    {
      somewhere_to_trace = true;
      break;
    }
  }
  
  if(!somewhere_to_trace)
    return;

  if(instance.tmp_buffer != nullptr)
  {
    tm_mutex->lock();
    if(vsnprintf(instance.tmp_buffer, TRACE_BSIZE, s.c_str(), ap) > 0)
    {
      for(uint32_t i = 0; i < 2; i++)
      {
        if((niveau >= instance.global_min_levels[i]) && (niveau >= log.min_levels[i]))
        {
          Tracer *tracer = instance.tracers[i];
          tracer->gen_trace(niveau, fonction, instance.tmp_buffer);
          /*
  # ifdef DEBUG_MODE
           tracer->flush();
  # else
          if(i == 1)
            tracer->flush(); // always flush file infos.
  # endif
  */
        }
      }
    }
    tm_mutex->unlock();
  }

  if(niveau == TraceLevel::AL_ANOMALY)
  {
    /*if(instance.abort_on_anomaly || instance.throw_on_anomaly)
    {
      for(uint32_t i = 0; i < 2; i++)
        if(niveau >= instance.global_min_levels[i])
          instance.tracers[i]->flush();
    }*/

  # ifdef DEBUG_MODE
    if(instance.abort_on_anomaly)
      *((char *) 0) = 5;

      //if (ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1)
      //{
        //raise(SIGABRT);
      //}
  # endif

  # ifdef RELEASE_MODE
    if(instance.abort_on_anomaly)
    {
      raise(SIGABRT);
    }
  # endif

    if(instance.throw_on_anomaly)
     throw std::runtime_error("Runtime anomaly detected. See log file for more informations.");
  }
}

void gen_trace(TraceLevel level, const std::string &fonction, const Logable &log, const std::string &s, ...)
{
  va_list ap;
  va_start(ap, s);
  gen_trace(level, fonction, log, s, ap);
  va_end(ap);
}

void gen_trace(TraceLevel level, const std::string &fonction, const std::string &s, ...)
{
  va_list ap;
  va_start(ap, s);
  gen_trace(level, fonction, journal_principal, s, ap);
  va_end(ap);
}




Logable::Logable(const std::string &module)
{
  module_id = module;
  min_levels[0] = min_levels[1] = AL_VERBOSE;
}


void Logable::setup(const std::string &module)
{
  module_id = module;
}

#if 0
void Logable::trace_normale(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(AL_NORMAL, *this, module_id, s, ap);
  va_end(ap);
}

void Logable::trace_major(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(AL_MAJOR, *this, module_id, s, ap);
  va_end(ap);
}

void Logable::anomaly(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(AL_ANOMALY, *this, module_id, s, ap);
  va_end(ap);
}

void Logable::warning(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(AL_WARNING, *this, module_id, s, ap);
  va_end(ap);
  instance.warning_count++;
}

void Logable::verbose_internal(std::string s, va_list ap) const
{
  if(actif)
    journal::gen_trace(journal::AL_VERBOSE, journal_principal, module_id, s, ap);
}
#endif

}

#if 0
void verbose(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(journal::AL_VERBOSE, "", s, ap);
  va_end(ap);
}

void infos(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(journal::AL_NORMAL, "", s, ap);
  va_end(ap);
}

void trace_majeure(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(journal::AL_MAJOR, "", s, ap);
  va_end(ap);
}


void erreur(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(journal::AL_ANOMALY, "", s, ap);
  va_end(ap);
}

void avertissement(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  journal::gen_trace(journal::AL_WARNING, "", s, ap);
  va_end(ap);
}
#endif


}

