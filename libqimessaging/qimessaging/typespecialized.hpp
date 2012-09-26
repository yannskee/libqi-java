#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPESPECIALIZED_HPP_
#define _QIMESSAGING_TYPESPECIALIZED_HPP_

#include <qimessaging/type.hpp>

namespace qi
{
// Interfaces for specialized types
class GenericIterator;
class QIMESSAGING_API TypeInt: public Type
{
public:
  virtual int64_t get(void* value) const = 0;
  virtual void set(void** storage, int64_t value) = 0;
  virtual Kind kind() const { return Int;}
};

class QIMESSAGING_API TypeFloat: public Type
{
public:
  virtual double get(void* value) const = 0;
  virtual void set(void** storage, double value) = 0;
  virtual Kind kind() const { return Float;}
};

class QIMESSAGING_API TypeIterator: public Type
{
public:
  virtual GenericValue dereference(void* storage) = 0; // must not be destroyed
  virtual void  next(void** storage) = 0;
  virtual bool equals(void* s1, void* s2) = 0;
};

class QIMESSAGING_API TypeList: public Type
{
public:
  virtual Type* elementType(void* storage) const = 0;
  virtual GenericIterator begin(void* storage) = 0; // Must be destroyed
  virtual GenericIterator end(void* storage) = 0;  //idem
  virtual void pushBack(void* storage, void* valueStorage) = 0;
  virtual Kind kind() const { return List;}
};

}

#include <qimessaging/details/typelist.hxx>
#endif