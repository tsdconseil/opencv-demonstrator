#include "hal.hpp"
#include "trace.hpp"

#include <stdio.h>
#include <string.h> // for memcpy
#include <unistd.h> // for usleep

#if 0



typedef void (*WAITORTIMERCALLBACK)(void *prm, bool TimerOrWaitFired);

WINBASEAPI HANDLE WINAPI CreateTimerQueue(void);
WINBASEAPI void WINAPI DeleteTimerQueue(HANDLE);
WINBASEAPI BOOL WINAPI CreateTimerQueueTimer(PHANDLE,HANDLE,WAITORTIMERCALLBACK,PVOID,DWORD,DWORD,ULONG);

#endif


namespace utils { namespace hal{


void os_thread_start(void *prm)
{
  ((utils::hal::ThreadFunctor *) prm)->call();
}


uint64_t get_native_tick_counter()
  {
#   ifdef SDPOS
    return arch_counter_get();
#   elif defined(WIN)
    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);
    return tick.QuadPart;
#   else
  struct timespec ts;
  if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
  {
    perror("clock_gettime().");
    return 0;
  }
  return (uint64_t) (ts.tv_nsec / 1000) + (((uint64_t) ts.tv_sec) * 1000 * 1000);
#   endif
  }

uint32_t ticks_to_ms(uint64_t ticks)
  {
#   ifdef SDPOS
    return arch_counter_to_ms(ticks);
#   elif defined(WIN)
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    uint32_t result = (uint32_t) (ticks / (frequency.QuadPart / 1000));
    return result;
#   else
    return (uint32_t) ticks / 1000;
#   endif
  }

uint64_t ticks_to_us(uint64_t ticks)
  {
#   ifdef SDPOS
    return arch_counter_to_us(ticks);
#   elif defined(WIN)

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    uint64_t result = ticks / (frequency.QuadPart / (1000 * 1000));
    return result;
#   else
    return ticks;
#   endif
  }


#if 0
Timer::Timer()
{
  functor = nullptr;
# ifdef WIN
  this->h_timer_queue = CreateTimerQueue();
# endif
}

Timer::~Timer()
{
# ifdef WIN
  ::DeleteTimerQueue(h_timer_queue);
# endif
}


static void timer_routine(void *prm, bool TimerOrWaitFired)
{
  if(prm == nullptr)
  {
    log_anomaly(0, "TimerRoutine lpParam is NULL.");
  }
  else
  {
    Timer *tim = (Timer *) prm;
    tim->signal.raise();
    if(tim->functor != nullptr)
      tim->functor->call();
  }
}

void Timer::start(uint32_t period)
{
  if(!::CreateTimerQueueTimer(&h_timer, h_timer_queue,
              (WAITORTIMERCALLBACK) timer_routine, &signal, period, 0, 0))
  {
    log_anomaly(0, "CreateTimerQueueTimer failed");
  }
}
#endif

/////////////////////////////////////////////////////
/// Mutex implementation
/////////////////////////////////////////////////////
Mutex::Mutex()
{
# ifdef SDPOS
  mutex = new_mutex();
# elif defined(LINUX)
  if(pthread_mutex_init(&mutex, nullptr))
  {
    perror("pthread_mutex_init");
  }
# else
  ::InitializeCriticalSectionAndSpinCount(&critical, 0);
# endif
}

Mutex::~Mutex()
{
# ifdef SDPOS
  // TODO
# elif defined(LINUX)
  pthread_mutex_destroy(&mutex);
# else
  ::DeleteCriticalSection(&critical);
# endif
}

void Mutex::lock()
{
# ifdef SDPOS
  require(mutex);
# elif defined(LINUX)
  pthread_mutex_lock(&mutex);
# else
  ::EnterCriticalSection(&critical);
# endif
}

void Mutex::unlock()
{
# ifdef SDPOS
  release(mutex);
# elif defined(LINUX)
  pthread_mutex_unlock(&mutex);
# else
  ::LeaveCriticalSection(&critical);
# endif
}


/////////////////////////////////////////////////////
/// Signal implementation
/////////////////////////////////////////////////////
Signal::Signal()
{
# ifdef SDPOS
  handle = new_signal();
# elif defined(LINUX)
  if(pthread_cond_init(&cvar, nullptr))
  {
    perror("pthread_cond_init");
  }
  if(pthread_mutex_init(&mutex, nullptr))
  {
    perror("pthread_mutex_init");
  }
  cnt = 0;
# else
  handle = ::CreateEvent(0,true,false,0);
# endif
}

Signal::~Signal()
{
# ifdef SDPOS
  // TODO
# elif defined(LINUX)
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cvar);
# else
  ::CloseHandle(handle);
# endif
}

#ifdef WIN
HANDLE Signal::get_handle()
{
  return handle;
}
#endif

void Signal::raise()
{
# ifdef SDPOS
  signal(handle);
# elif defined(LINUX)
  pthread_mutex_lock(&mutex);
  cnt++;
  pthread_cond_signal(&cvar);
  pthread_mutex_unlock(&mutex);
# else
  ::SetEvent(handle);
# endif
}

void Signal::clear()
{
# ifdef SDPOS
  signal_clear(handle);
# elif defined(LINUX)
  cnt = 0;
# else
  ::ResetEvent(handle);
# endif
}

void Signal::wait()
{
# ifdef SDPOS
  wait(handle);
# elif defined(LINUX)
  for(;;)
  {
    pthread_mutex_lock(&mutex);
    if(cnt > 0)
    {
      cnt--;
      pthread_mutex_unlock(&mutex);
      return;
    }
    pthread_cond_wait(&cvar, &mutex);
    pthread_mutex_unlock(&mutex);
  }
# else
  ::WaitForSingleObject(handle, INFINITE);
  ::ResetEvent(handle);
# endif
}

int Signal::wait(unsigned int timeout)
{
  if(timeout == 0)
  {
    wait();
    return 0;
  }

# ifdef SDPOS
  return wait_with_timeout(handle, timeout);
# elif defined(LINUX)



  struct timespec ts;

  clock_gettime(CLOCK_REALTIME, &ts);

  if(timeout >= 1000)
  {
    ts.tv_sec += timeout / 1000;
    timeout = timeout % 1000;
  }

  ts.tv_nsec += ((uint64_t) timeout) * 1000 * 1000;
  if(ts.tv_nsec >= 1000000000l)
  {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000ul;
  }

  for(;;)
  {
    pthread_mutex_lock(&mutex);
    if(cnt > 0)
    {
      cnt--;
      pthread_mutex_unlock(&mutex);
      return 0;
    }

    if(pthread_cond_timedwait(&cvar, &mutex, &ts))
    {
      if(cnt > 0)
      {
        cnt--;
        pthread_mutex_unlock(&mutex);
        return 0;
      }
      pthread_mutex_unlock(&mutex);
      return -1;
    }
    pthread_mutex_unlock(&mutex);
  }
# else
  if(::WaitForSingleObject(handle, timeout) != WAIT_OBJECT_0)
    return -1;
  ::ResetEvent(handle);
  return 0;
# endif
}

bool Signal::is_raised()
{
# ifdef SDPOS
  return ::is_raised(handle);
# elif defined(LINUX)
  return cnt > 0;
# else
  if(::WaitForSingleObject(handle, 0) == WAIT_OBJECT_0)
  {
    ::SetEvent(handle);
    return true;
  }
  return false;
# endif
}

static void my_sleep(uint32_t ms)
{
# ifdef LINUX
  usleep(ms * 1000);
# else
  ::Sleep(ms);
# endif
}

void sleep(uint32_t ms)
{
  my_sleep(ms);
}

RawFifo::RawFifo(uint32_t capacity)
{
  this->capacity  = capacity;
  fifo_first      = 0;
  fifo_size       = 0;
  buffer = (uint8_t *) malloc(capacity);
  deblocked = false;
  if(buffer == nullptr)
  {
    fprintf(stderr, "rawfifo(capacity = %d): malloc error.\n", capacity);
    fflush(stderr);
    this->capacity = 0;
  }
}

void RawFifo::deblock()
{
  deblocked = true;
  h_not_full.raise();
  h_not_empty.raise();
}

void RawFifo::clear()
{
  mutex.lock();
  fifo_first      = 0;
  fifo_size       = 0;
  mutex.unlock();
}

RawFifo::~RawFifo()
{
  if(buffer != nullptr)
    free(buffer);
}

uint32_t RawFifo::write(void *data_, uint32_t size)
{
  uint8_t *data = (uint8_t *) data_;
  uint32_t a = size;
  uint32_t b = capacity;

  if(a > b)
  {
    //size = capacity;
    /* Write by chunk of capacity */
    while(size > capacity)
    {
      write(data, capacity);
      data += capacity;
      size -= capacity;
    }
  }

  /*if(size > capacity)
    size = capacity;*/

  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = this->fifo_size;
    mutex.unlock();
    if(lsize + size < capacity)
      break;
    h_not_full.wait();
  }
  mutex.lock();
  uint32_t ffirst = fifo_first;
  mutex.unlock();


  if(size + ffirst < capacity)
  {
    memcpy(&(buffer[ffirst]), data, size);
  }
  else
  {
    memcpy(&(buffer[ffirst]), data, capacity - ffirst);
    memcpy(&(buffer[0]), &(data[capacity - ffirst]), size - (capacity - ffirst));
  }

  mutex.lock();
  fifo_first = (fifo_first + size) % capacity;
  fifo_size += size;
  mutex.unlock();

  h_not_full.raise();
  h_not_empty.raise();
  return size;
}

uint32_t RawFifo::read(void *data_, uint32_t size, uint32_t timeout)
{
  uint8_t *data = (uint8_t *) data_;
  uint32_t N = 0;

  if(size > capacity)
  {
    //size = capacity;
    /* Read by chunk of capacity */

    while(size > capacity)
    {
      uint32_t n = read(data, capacity, timeout);

      N += n;

      if(n != capacity)
        return N;

      data += capacity;
      size -= capacity;
    }

    if(size == 0)
      return N;
  }

  for(;;)
  {
    if(deblocked)
      return 0;


    uint32_t lsize;
    mutex.lock();
    lsize = this->fifo_size;
    mutex.unlock();
    if(lsize >= size)
      break;
    if(timeout > 0)
    {
      if(h_not_empty.wait(timeout))
      {
        printf(">>>>>>>>>>>>>>>>>>>>>>\nTIMEOUT FIFO READ: size=%d, requested=%d, timeout=%d ms.\n", fifo_size, size, timeout);
        fflush(0);

        mutex.lock();
        lsize = this->fifo_size;
        mutex.unlock();

        printf(">>>>> lsize = %d.\n", lsize);
        fflush(0);

        if(lsize >= size)
          break;

        return 0;
      }
    }
    else
      h_not_empty.wait();
  }
  mutex.lock();
  uint32_t ffirst = fifo_first;
  uint32_t fsize  = fifo_size;
  mutex.unlock();

  // Now fsize >= size
  /* (1) */
  if(fsize <= ffirst)
  {
    memcpy(data, &(buffer[ffirst - fsize]), size);
  }
  else
  {
    /* (2b) */
    if(size > fsize - ffirst)
    {
      memcpy(data, &(buffer[capacity - (fsize - ffirst)]), fsize - ffirst);
      memcpy(&(data[fsize - ffirst]), buffer, size - (fsize - ffirst));
    }
    /* (2a) */
    else
    {
      memcpy(data, &(buffer[capacity - (fsize - ffirst)]), size);
    }
  }

  mutex.lock();
  fifo_size -= size;
  mutex.unlock();

  h_not_full.raise();
  h_not_empty.raise();
  return size + N;
}

bool RawFifo::full()
{
  bool res;
  mutex.lock();
  res = (fifo_size == capacity);
  mutex.unlock();
  return res;
}

bool RawFifo::empty()
{
  bool res;
  mutex.lock();
  res = (fifo_size == 0);
  mutex.unlock();
  return res;
}


#ifdef WIN
static LARGE_INTEGER base_tick;
static LARGE_INTEGER frequency;
static bool tick_init_done = false;
#endif

uint64_t get_tick_count_us()
{
# ifdef WIN
  LARGE_INTEGER tick;
  if(!tick_init_done)
  {
    if(!QueryPerformanceFrequency(&frequency))
    {
      printf("Failed to initialize 64 bits counter.\n");
      frequency.QuadPart = 1000 * 1000;
    }
    QueryPerformanceCounter(&base_tick);
    tick_init_done = true;
  }
  QueryPerformanceCounter(&tick);
  uint64_t result = (uint64_t) ((float)(tick.QuadPart-base_tick.QuadPart)*1000.0*1000.0 / frequency.QuadPart);
  return result;
# else
  struct timespec ts;

  if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
  {
    perror("clock_gettime().");
    return 0;
  }
  return (uint64_t) (ts.tv_nsec / 1000) + (((uint64_t) ts.tv_sec) * 1000 * 1000);
# endif
}

uint64_t get_tick_count_ms()
{
# ifdef WIN
  //return GetTickCount();
  LARGE_INTEGER tick;
  if(!tick_init_done)
  {
    if(!QueryPerformanceFrequency(&frequency))
    {
      printf("Failed to initialize 64 bits counter.\n");
      frequency.QuadPart = 1000 * 1000;
    }
    QueryPerformanceCounter(&base_tick);
    tick_init_done = true;
  }
  QueryPerformanceCounter(&tick);
  uint64_t result = (uint64_t) ((float)(tick.QuadPart-base_tick.QuadPart)*1000.0 / frequency.QuadPart);
  return result;
# else
  struct timespec ts;

  if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
  {
    perror("clock_gettime().");
    return 0;
  }

  return (uint64_t) (ts.tv_nsec / (1000 * 1000)) + ts.tv_sec * 1000;
# endif
}





}}
