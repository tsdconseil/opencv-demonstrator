#ifndef CUTIL_PRIVATE_HPP
#define CUTIL_PRIVATE_HPP

#include "cutil.hpp"
#include "hal.hpp"

#ifdef WIN
//#include <windows.h>
#include <process.h>
#endif

/** @cond not-documented */


namespace utils{ namespace hal{
template <class A>
void thread_start(A *target_class,
                     void (A:: *target_function)(),
                      /*__attribute__((unused))*/ std::string name)
{
  SpecificThreadFunctor<A> *stf = new SpecificThreadFunctor<A>(target_class, target_function);

# ifdef WIN
  _beginthread(&utils::hal::os_thread_start, /*64000*/0, stf);
# elif defined(LINUX)
  pthread_t   thread_id;// = (pthread_t *) malloc(sizeof(pthread_t));
  pthread_attr_t  attr;

  if(pthread_attr_init(&attr))
  {
    perror("pthread_attr_init");
    return;  // EINVAL, ENOMEM
  }

  if(pthread_attr_setstacksize(&attr, 64000))
  {
    perror("pthread_attr_setstacksize");
    return;  // EINVAL, ENOSYS
  }

  if(pthread_create(&thread_id,
                    &attr,
                    (void*(*)(void*))&utils::hal::os_thread_start,
                    stf))
  {
    perror("pthead_create");
    return;
  }
# endif
}


template<class T>
Fifo<T>::Fifo(uint32_t capacity)
{
  this->capacity = capacity;
}

template<class T>
void Fifo<T>::push(T t)
{
  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = list.size();
    mutex.unlock();
    if(lsize < capacity)
      break;
    h_not_full.wait();
  }
  mutex.lock();
  h_not_full.raise();
  list.push_back(t);
  h_not_empty.raise();
  mutex.unlock();
}

template<class T>
uint32_t Fifo<T>::size() const
{
  /*uint32_t res;
  mutex.lock();
  res = list.size();
  mutex.unlock();
  return res;*/
  return list.size();
}

template<class T>
void Fifo<T>::push(T *t, uint32_t nelem)
{
  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = list.size();
    mutex.unlock();
    if(lsize + nelem < capacity)
      break;
    h_not_full.wait();
  }
  mutex.lock();
  h_not_full.raise();
  for(uint32_t i = 0; i < nelem; i++)
    list.push_back(t[i]);
  h_not_empty.raise();
  mutex.unlock();
}

template<class T>
int Fifo<T>::pop(T *t, uint32_t nelem, uint32_t timeout)
{
  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = list.size();
    mutex.unlock();
    if(lsize >= nelem)
      break;
    if(h_not_empty.wait(timeout))
      return 0;
  }
  h_not_empty.raise();
  mutex.lock();
  for(uint32_t i = 0; i < nelem; i++)
  {
    t[i] = list[0];
    list.pop_front();
  }
  mutex.unlock();
  h_not_full.raise();
  return nelem;
}

template<class T>
int Fifo<T>::pop_with_timeout(uint32_t timeout_ms, T &res)
{
  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = list.size();
    mutex.unlock();
    if(lsize > 0)
      break;
    if(h_not_empty.wait(timeout_ms))
      return -1;
  }
  h_not_empty.raise();
  mutex.lock();
  res = list[0];
  list.pop_front();
  mutex.unlock();
  h_not_full.raise();
  return 0;
}

template<class T>
T Fifo<T>::pop()
{
  T res;
  for(;;)
  {
    uint32_t lsize;
    mutex.lock();
    lsize = list.size();
    mutex.unlock();
    if(lsize > 0)
      break;
    h_not_empty.wait();
  }
  h_not_empty.raise();
  mutex.lock();
  res = list[0];
  list.pop_front();
  mutex.unlock();
  h_not_full.raise();
  return res;
}

template<class T>
bool Fifo<T>::full() const
{
  return list.size() == capacity;
}

template<class T>
bool Fifo<T>::empty() const
{
  return list.size() == 0;
}

template<class T>
void Fifo<T>::clear()
{
  mutex.lock();
  list.clear();
  h_not_full.raise();
  mutex.unlock();
}
}}

/** @endcond */

#endif
