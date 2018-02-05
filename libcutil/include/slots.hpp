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

#ifndef SLOTS_H
#define SLOTS_H

#include "hal.hpp"

#include <vector>
#include <deque>


namespace utils
{

template <class Type> class Provider;
template <class Type> class Listener;

template <class Type> class CProvider;
template <class Type> class CListener;




/** @cond not-documented */
template <class B>
class EventFunctor
{
public:
  virtual void call( B &b) = 0;
};

#if 0
class VoidEventFunctor
{
public:
  virtual void call() = 0;
};
#endif

template <class A, class B>
class SpecificEventFunctor: public EventFunctor<B>
{
public:
  SpecificEventFunctor(A *object, void(A::*m_function)( B &b))
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual int call( B &b)
  {
    return (*object.*m_function)(b);
  }
private:
  int (A::*m_function)( B &);
  A *object;
};


#if 0
template <class A>
class SpecificVoidEventFunctor: public VoidEventFunctor
{
public:
  SpecificVoidEventFunctor(A *object, void (A::*m_function)())
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
#endif



/** Event provider mother class. */
template <class Type>
class Provider
{
    friend class Listener<Type>;
public:
    void add_listener(Listener<Type> *lst);
    void remove_listener(Listener<Type> *lst);
    void remove_all_listeners();
    void dispatch( Type &evt);
    /** Block until all dispatch is done. */
    //void wait_dispatch_done();

    template<class A>
    int add_listener(A *target,
                     void (A:: *fun)( Type &));

    template<class A>
    int remove_listener(A *target,
                        void (A:: *fun)( Type &));

    virtual ~Provider<Type>(){remove_all_listeners();}

private:
    std::deque<void *> listeners;
    std::deque<EventFunctor<Type> *> functors;
};

#if 0
class VoidEventProvider
{
public:
    void remove_all_listeners();
    void dispatch();

    template<class A>
    int add_listener(A *target,
                     void (A:: *fun)());

    template<class A>
    int remove_listener(A *target,
                        void (A:: *fun)());

    virtual ~VoidEventProvider(){remove_all_listeners();}

private:
    std::deque<VoidEventFunctor *> functors;
};
#endif

/** Event listener mother class. */
template <class Type>
class Listener
{
    friend class Provider<Type>;
public:
    virtual ~Listener() {}
    std::string listener_name;
    //private:
    virtual void on_event( Type &evt) = 0;
};

template <class Type>
void Provider<Type>::add_listener(Listener<Type> *lst)
{
    listeners.push_back(lst);
}

template <class Type>
void Provider<Type>::remove_listener(Listener<Type> *lst)
{
  std::deque<void *>::iterator it;
  for(it =  listeners.begin(); it !=  listeners.end(); it++)
  {
    void *cur = *it;
    if(cur == (void *) lst)
    {
      listeners.erase(it);
      return;
    }
  }
}

template <class Type>
void Provider<Type>::remove_all_listeners()
{
  listeners.clear();
  functors.clear();
}



template <class Type>
void Provider<Type>::dispatch( Type &evt)
{
  //mutex.lock();

  std::vector<void *> copy;
  for(unsigned int i = 0; i < listeners.size(); i++)
    copy.push_back(listeners[i]);

  for(unsigned int i = 0; i < copy.size(); i++)
  {
      void *cur = copy[i];
      Listener<Type> *pt = (Listener<Type> *) cur;
      pt->on_event(evt);
  }

  std::vector<EventFunctor<Type> *> copy2;
  for(unsigned int i = 0; i < functors.size(); i++)
    copy2.push_back(functors[i]);

  for(unsigned int i = 0; i < copy2.size(); i++)
  {
    EventFunctor<Type> *ef = copy2[i];
    ef->call(evt);
  }
  //mutex.unlock();
}

#if 0
template<class A>
int VoidEventProvider::add_listener(A *target,
                                    void (A:: *fun)())
{
  /* TODO: check if not already registered */
  VoidEventFunctor *f = new SpecificVoidEventFunctor<A>(target, fun);
  functors.push_back(f);
  return 0;
}
#endif

template<class Type>
template<class A>
int Provider<Type>::add_listener(A *target,
                              void (A:: *fun)( Type &))
{
  /* TODO: check if not already registered */
  SpecificEventFunctor<A,Type> *f = new SpecificEventFunctor<A,Type>(target, fun);
  functors.push_back(f);
  return 0;
}


#if 0
template<class A>
int VoidEventProvider::remove_listener(A *target,
                                    void (A:: *fun)())
{
  return 0;
}
#endif

template<class Type>
template<class A>
int Provider<Type>::remove_listener(A *target,
                                    void (A:: *fun)( Type &))
{
  //std::deque<SpecificEventFunctor<A,Type> *>::iterator it;

  /*for(it = functors.begin(); it !=  functors.end(); it++)
  {
    EventFunctor<Type> *cur = *it;
    SpecificEventFunctor<A,Type> *cst = (SpecificEventFunctor<A,Type>) cur;
    if((cst->object == target)
        && (cst->m_function == fun))
    {
      functors.erase(it);
      return;
    }
  }*/
  return 0;
}

// ###############################################################################

/** @cond not-documented */
template <class B>
class CEventFunctor
{
public:
  virtual void call(const B &b) = 0;
};

template <class A, class B>
class SpecificCEventFunctor: public CEventFunctor<B>
{
public:
  SpecificCEventFunctor(A *object, void(A::*m_function)(const B &b))
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual void call(const B &b)
  {
    (*object.*m_function)(b);
  }
private:
  void (A::*m_function)(const B &);
  A *object;
};
/** @endcond */



/** Event provider mother class. */
template <class Type>
class CProvider
{
    friend class CListener<Type>;
public:
    void add_listener(CListener<Type> *lst);
    void remove_listener(CListener<Type> *lst);
    void remove_all_listeners();
    void dispatch(const Type &evt);
    /** Block until all dispatch is done. */
    //void wait_dispatch_done();

    template<class A>
    int add_listener(A *target,
                     void (A:: *fun)(const Type &));

    template<class A>
    int remove_listener(A *target,
                        void (A:: *fun)(const Type &));

    virtual ~CProvider<Type>(){remove_all_listeners();}

private:
    std::deque<void *> listeners;
    std::deque<CEventFunctor<Type> *> functors;
    //OSMutex mutex;
};

/** Event listener mother class. */
template <class Type>
class CListener
{
    friend class CProvider<Type>;
public:
    virtual ~CListener() {}
    std::string listener_name;
    //private:
    virtual void on_event(const Type &evt) = 0;
};

template <class Type>
void CProvider<Type>::add_listener(CListener<Type> *lst)
{
    listeners.push_back(lst);
}

template <class Type>
void CProvider<Type>::remove_listener(CListener<Type> *lst)
{
  std::deque<void *>::iterator it;
  for(it =  listeners.begin(); it !=  listeners.end(); it++)
  {
    void *cur = *it;
    if(cur == (void *) lst)
    {
      listeners.erase(it);
      return;
    }
  }
}

template <class Type>
void CProvider<Type>::remove_all_listeners()
{
  //mutex.lock();
  listeners.clear();
  functors.clear();
  //mutex.unlock();
}

/*template <class Type>
void Provider<Type>::wait_dispatch_done()
{
  mutex.lock();
  listeners.clear();
  functors.clear();
  mutex.unlock();
}*/


template <class Type>
void CProvider<Type>::dispatch(const Type &evt)
{
  //mutex.lock();

  std::vector<void *> copy;
  for(unsigned int i = 0; i < listeners.size(); i++)
    copy.push_back(listeners[i]);

  for(unsigned int i = 0; i < copy.size(); i++)
  {
      void *cur = copy[i];
      CListener<Type> *pt = (CListener<Type> *) cur;
      pt->on_event(evt);
  }

  std::vector<CEventFunctor<Type> *> copy2;
  for(unsigned int i = 0; i < functors.size(); i++)
    copy2.push_back(functors[i]);

  for(unsigned int i = 0; i < copy2.size(); i++)
  {
    CEventFunctor<Type> *ef = copy2[i];
    ef->call(evt);
  }
  //mutex.unlock();
}


template<class Type>
template<class A>
int CProvider<Type>::add_listener(A *target,
                              void (A:: *fun)(const Type &))
{
  /* TODO: check if not already registered */
  SpecificCEventFunctor<A,Type> *f = new SpecificCEventFunctor<A,Type>(target, fun);
  functors.push_back(f);
  return 0;
}

template<class Type>
template<class A>
int CProvider<Type>::remove_listener(A *target,
                                    void (A:: *fun)(const Type &))
{
  //std::deque<SpecificEventFunctor<A,Type> *>::iterator it;

  /*for(it = functors.begin(); it !=  functors.end(); it++)
  {
    EventFunctor<Type> *cur = *it;
    SpecificEventFunctor<A,Type> *cst = (SpecificEventFunctor<A,Type>) cur;
    if((cst->object == target)
        && (cst->m_function == fun))
    {
      functors.erase(it);
      return;
    }
  }*/
  return 0;
}

}

#endif
