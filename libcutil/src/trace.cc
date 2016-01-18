#include "trace.hpp"
#include "cutil.hpp"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
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

namespace utils
{

using namespace std;

unsigned int main_log;

static bool abort_on_anomaly = false, throw_on_anomaly = false;
#ifndef DISPLAY_TIME
static bool date_base_ok  = false;
static uint64_t date_base = 0;
#endif

#ifdef LINUX
hal::Mutex tm_mutex_;
#define tm_mutex (&tm_mutex_)
#else
hal::Mutex *tm_mutex;
#endif

static TraceManager instance;

#define TRACE_BSIZE (1024*1024)


char *TraceManager::tmp_buffer = nullptr;

TraceManager::TraceManager()
{
  warning_count = 0;
  anomaly_count = 0;
  global_enable = true;
  global_min_levels[0] = AL_VERBOSE;
  global_min_levels[1] = AL_NONE;
  global_min_levels[2] = AL_NONE;
  tracers[0] = &std_tracer;
  tracers[1] = &file_tracer;
  tracers[2] = &std_tracer;
  tmp_buffer = (char *) malloc(TRACE_BSIZE);
  main_log = this->register_module("main");
  //dsp_log  = this->register_module("dsp");
}

uint32_t TraceManager::get_anomaly_count()
{
  return instance.anomaly_count;
}

uint32_t TraceManager::get_warning_count()
{
  return instance.warning_count;
}

void TraceManager::set_abort_on_anomaly(bool abort)
{
  abort_on_anomaly = abort;
}

TraceManager::~TraceManager()
{
  //printf("delete trace manager.\n");
}

TraceManager *TraceManager::get_instance()
{
# ifndef LINUX
  if(tm_mutex == nullptr)
  {
    //instance = new TraceManager();
    tm_mutex = new hal::Mutex();
  }
# endif
  return &instance;
}

void TraceManager::enable_all(bool enable)
{
  instance.global_enable = enable;
}

void TraceManager::set_log_file(std::string filename)
{
  instance.file_tracer.set_log_file(filename);
}

TraceLevel TraceManager::get_global_min_level(TraceTarget target)
{
  uint32_t tr = (uint32_t) target;

  if(tr < 3)
    return instance.global_min_levels[tr];

  return AL_NONE;
}

void TraceManager::set_global_min_level(TraceTarget target, TraceLevel min_level)
{
  uint32_t tr = (uint32_t) target;

  if(tr < 3)
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
  //set_log_file("./trace.log");
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

void FileTracer::trace(int trace_level, std::string module, std::string message)
{
  if(of != nullptr)
  {
#ifndef DISPLAY_TIME
    if(!date_base_ok)
    {
      date_base_ok = true;
      date_base = hal::get_tick_count_us();
    }
#endif
    {
#ifndef DISPLAY_TIME
      uint64_t ticks = hal::get_tick_count_us() - date_base;
      uint32_t seconds = ticks / (1000 * 1000);
      ticks -= seconds * 1000 * 1000;
      uint32_t ms      = ticks / 1000;
      ticks -= ms * 1000;
      uint32_t us = (uint32_t) ticks;
#endif
      if(trace_level >= AL_WARNING)
        fprintf(of, "----------------------------------------\n");
      if(trace_level == AL_WARNING)
        fprintf(of, "---------- WARNING\n");
      else if(trace_level == AL_ANOMALY)
        fprintf(of, "---------- ANOMALY\n");
#ifdef DISPLAY_TIME
//       boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::universal_time();
      boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
      std::string time_str = boost::posix_time::to_iso_extended_string(time_now);
      fprintf(of, "%s: ", time_str.c_str());
#else
      fprintf(of, "%4u,%03d,%03d: ", seconds, ms, us);
#endif

      fprintf(of, "[%s] %s\n", module.c_str(), message.c_str());

      if(trace_level >= AL_WARNING)
      {
        fprintf(of, "----------------------------------------\n");
      }

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

void StdTracer::trace(int trace_level, std::string module, std::string message)
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
  if(!date_base_ok)
  {
    date_base_ok = true;
    date_base = hal::get_tick_count_us();
  }
#endif
  {
#ifdef DISPLAY_TIME
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
    std::string time_str = boost::posix_time::to_iso_extended_string(time_now);
    fprintf(output, "%s: ", time_str.c_str());
#else
    uint64_t new_ticks = hal::get_tick_count_us();
    uint64_t ticks =  new_ticks - date_base;
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

    *out << " \033[" << color << "m[" << module << "] " << message << "\033[0m" << std::endl;

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

void TraceManager::trace(TraceLevel level,
                         std::string module,
                         std::string s,
                         va_list ap)
{
  bool somewhere_to_trace = false;

  if(!instance.global_enable)
    return;

  for(uint32_t i = 0; i < 3; i++)
  {
    if(level >= instance.global_min_levels[i])
    {
      somewhere_to_trace = true;
      break;
    }
  }
  
  if(!somewhere_to_trace)
    return;

  if(tmp_buffer != nullptr)
  {
    tm_mutex->lock();
    if(vsnprintf(tmp_buffer, TRACE_BSIZE, s.c_str(), ap) > 0)
    {
      for(uint32_t i = 0; i < 3; i++)
      {
        if(level >= instance.global_min_levels[i])
        {
          Tracer *tracer = instance.tracers[i];
          tracer->trace(level, module, tmp_buffer);
  # ifdef DEBUG_MODE
           tracer->flush();
  # else
          if(i == 1)
            tracer->flush(); // always flush file trace.
  # endif
        }
      }
    }
    tm_mutex->unlock();
  }



  if((abort_on_anomaly || throw_on_anomaly) && (level == TraceLevel::AL_ANOMALY))
  {
    for(uint32_t i = 0; i < 3; i++)
      if(level >= instance.global_min_levels[i])
        instance.tracers[i]->flush();
  }

# ifdef DEBUG_MODE
  if(abort_on_anomaly && (level == TraceLevel::AL_ANOMALY))
    *((char *) 0) = 5;

    //if (ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1)
    //{
      //raise(SIGABRT);
    //}
# endif

# ifdef RELEASE_MODE
  if(abort_on_anomaly && (level == TraceLevel::AL_ANOMALY))
  {
    raise(SIGABRT);
  }
# endif

  if(throw_on_anomaly && (level == TraceLevel::AL_ANOMALY))
   throw std::runtime_error("Runtime anomaly detected. See log file for more informations.");
}

void TraceManager::trace(TraceLevel level,
                         unsigned int module_id,
                         std::string s,
                         va_list ap)
{
  if(module_id < instance.modules.size())
    trace(level, instance.modules[module_id].name, s, ap);
  else
    trace(level, "", s, ap);
}

void TraceManager::trace(TraceLevel level, std::string module, std::string s, ...)
{
  va_list ap;
  va_start(ap, s);
  trace(level, module, s, ap);
  va_end(ap);
}

void TraceManager::trace(TraceLevel level, unsigned int module_id, std::string s, ...)
{
  va_list ap;
  va_start(ap, s);
  if(module_id < instance.modules.size())
    trace(level, instance.modules[module_id].name, s, ap);
  else
    trace(level, "", s, ap);
  va_end(ap);
}

uint32_t TraceManager::register_module(std::string module)
{
  TraceModule tm;
  tm.name = module;
  tm.id   = instance.modules.size();
  instance.modules.push_back(tm);
  //cout << "TM: registered module " << module << endl;
  return tm.id;
}

void TraceManager::unregister_module(uint32_t module_id)
{
  // TODO
}


Logable::Logable()
{
  module_id = 0;
}

Logable::Logable(const std::string &module)
{
  module_id = TraceManager::register_module(module);
}

Logable::Logable(const std::string &root_module, const std::string &module)
{
  module_id = TraceManager::register_module(root_module + "/" + module);
}

Logable::~Logable()
{
  TraceManager::unregister_module(module_id);
}

void Logable::setup(const std::string &module)
{
  module_id = TraceManager::register_module(module);
}

void Logable::setup(const std::string &root_module, const std::string &module)
{
  module_id = TraceManager::register_module(root_module + "/" + module);
}

void Logable::trace(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  TraceManager::trace(AL_NORMAL, module_id, s, ap);
  va_end(ap);
}

void Logable::trace_major(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  TraceManager::trace(AL_MAJOR, module_id, s, ap);
  va_end(ap);
}

void Logable::anomaly(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  TraceManager::trace(AL_ANOMALY, module_id, s, ap);
  va_end(ap);

  instance.anomaly_count++;
}

void Logable::warning(std::string s, ...) const
{
  va_list ap;
  va_start(ap, s);
  TraceManager::trace(AL_WARNING, module_id, s, ap);
  va_end(ap);

  instance.warning_count++;
}

void Logable::verbose_internal(std::string s, va_list ap) const
{
  TraceManager::trace(AL_VERBOSE, module_id, s, ap);
}
}

