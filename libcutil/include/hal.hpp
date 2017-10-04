#ifndef HAL_HPP
#define HAL_HPP


#ifdef WIN
# include <windows.h>
#endif

#include <stdint.h>
#include <deque>
#include <string>
#include <vector>

/** @file hal.hpp
 *  @brief OS abstraction layer */

namespace utils
{

/** @brief Portable miscellaneous functions */
namespace hal
{


template<class A>
  void thread_start(A *target_class,
                    void (A:: *target_function)(),
                   std::string name = "anonymous");

extern void sleep(uint32_t ms);

/** Tick counts in ms */
extern uint64_t get_tick_count_ms();

/** Tick counts in µs */
extern uint64_t get_tick_count_us();


extern void os_thread_start(void *prm);

extern uint64_t get_native_tick_counter();

extern uint32_t ticks_to_ms(uint64_t ticks);

extern uint64_t ticks_to_us(uint64_t ticks);



/** @brief Portable mutex class */
class Mutex
{
public:
  Mutex();
  ~Mutex();
  void lock();
  void unlock();
private:
# ifdef LINUX
  pthread_mutex_t mutex;
# else
  CRITICAL_SECTION critical;
# endif
};

/** @brief Portable signal class */
class Signal
{
public:
  Signal();
  ~Signal();
  void raise();
  void clear();
  void wait();
  int wait(unsigned int timeout);
  static int wait_multiple(unsigned int timeout, std::vector<Signal *> sigs);
  bool is_raised();
# ifdef WIN
  HANDLE get_handle();
# endif
private:
# ifdef LINUX
  pthread_cond_t  cvar;
  pthread_mutex_t mutex;
  int cnt;
# else
  HANDLE handle;
# endif
};


/** @brief Portable thread-safe FIFO class */
template<typename T>
class Fifo
{
public:
  /** @brief Build a fifo with the specified number of "T" elements. */
  Fifo(uint32_t capacity = 16);

  /** @brief Push an element in the fifo. Blocks the caller if fifo is full. */
  void push(T t);

  /** @brief Push n elements in the fifo.
   *  Blocks the caller if fifo space is to short. */
  void push(const T *t, uint32_t nelem);

  /** @brief Pop an element from the fifo.
   *  Blocks the caller if fifo is empty. */
  T pop();

  int pop_with_timeout(uint32_t timeout_ms, T &res);

  /** @brief Pop n elements from the fifo.
   *  Blocks the caller if the fifo do not contain enough elements. */
  int pop(T *t, uint32_t nelem, uint32_t timeout);

  /** @returns true if the fifo is full */
  bool full() const;

  /** @returns true if the fifo is empty */
  bool empty() const;

  /** @brief Clear the FIFO */
  void clear();

  uint32_t size() const;

private:
  Mutex mutex;
public:
  uint32_t capacity;
private:
  std::deque<T> list;
public:
  Signal h_not_full, h_not_empty;
};


/** @cond not-documented */
class ThreadFunctor
{
public:
  virtual void call() = 0;
};

template <class A>
class SpecificThreadFunctor: public ThreadFunctor
{
public:
  SpecificThreadFunctor(A *object, void (A::*m_function)())
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual void call()
  {
    (*object.*m_function)();
  }
private:
  void (A::*m_function)();
  A *object;
};
/** @endcond */




/** @brief Raw fifo of unformated data (bytes) */
class RawFifo
{
public:
  RawFifo(uint32_t capacity);
  ~RawFifo();
  uint32_t write(void *data, uint32_t size);
  uint32_t read (void *data, uint32_t size, uint32_t timeout);
  bool full();
  bool empty();
  void clear();
  void deblock();
  int size();
private:
  Mutex mutex;
  uint32_t capacity;
  uint32_t fifo_first, fifo_size;
  uint8_t *buffer;
  Signal h_not_full, h_not_empty;
  bool deblocked;
};

}

}








#endif
