#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPE_HPP_
#define _QIMESSAGING_TYPE_HPP_

#include <typeinfo>
#include <boost/preprocessor.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/details/dynamicvalue.hpp>

namespace qi{

/** Interface for all the operations we need on any type:
 *
 *  - cloning/destruction in clone() and destroy()
 *  - type conversion is made by going through the generic container
 *    GenericValue, using the toValue() and fromValue() functions,
 *  - Serialization through serialize() and deserialize() to transmit
 *    the value through some kind of pipe.
 *
 * Our aim is to transport arbitrary values through:
 *  - synchronous calls: Nothing to do, values are just transported and
      converted.
 *  - asynchronous call/thread change: Values are copied.
 *  - process change: Values are serialized.
 *
 */
class QIMESSAGING_API Type
{
public:
  virtual const std::type_info& info() =0;
  const char* infoString() { return info().name();} // for easy gdb access
  /// @return the serialization signature used by this type.
  virtual std::string signature()=0;

  // Initialize and return a new storage, from nothing or a T*
  virtual void* initializeStorage(void* ptr=0)=0;
  // Get pointer to type from pointer to storage
  // No reference to avoid the case where the compiler makes a copy on the stack
  virtual void* ptrFromStorage(void**)=0;

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  virtual bool  toValue(const void*, qi::detail::DynamicValue&)=0;
  virtual void* fromValue(const qi::detail::DynamicValue&)=0;

  // Default impl does toValue.serialize()
  virtual void  serialize(ODataStream& s, const void*)=0;
  // Default impl does deserialize(GenericValue&) then fromValue
  virtual void* deserialize(IDataStream& s)=0;

  /* When someone makes a call with arguments that do not match the
   * target signature (ex: int vs float), we want to handle it.
   * For this, given the known correct signature S and known incorrect
   * GenericValue v, we want to be able to obtain a new Type T that
   * serializes with type S, and then try to convert o into type T.
   *
   * For this we need a map<Signature, Type> that we will feed with
   * known types.
   *
   */
  typedef std::map<std::string, Type*> TypeSignatureMap;
  static TypeSignatureMap& typeSignatureMap()
  {
    static TypeSignatureMap res;
    return res;
  }

  static Type* getCompatibleTypeWithSignature(const std::string& sig)
  {
    TypeSignatureMap::iterator i = typeSignatureMap().find(sig);
    if (i == typeSignatureMap().end())
      return 0;
    else
      return i->second;
  }

  static bool registerCompatibleType(const std::string& sig,
    Type* mt)
  {
    typeSignatureMap()[sig] = mt;
    return true;
  }

  #define QI_REGISTER_MAPPING(sig, type) \
  static bool BOOST_PP_CAT(_qireg_map_ , __LINE__) = ::qi::Type::registerCompatibleType(sig, \
    ::qi::typeOf<type>());
};


namespace detail {
  template<typename T> struct TypeFactory
  {
    void* operator()() { return new T();}
  };
};
#define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
namespace qi { namespace detail { \
template<> struct TypeFactory<T> { void* operator()() { return 0;}};}}
/** Meta-type specialization.
 *  Use the aspect pattern, make a class per feature group
 *  (Clone, GenericValue, Serialize)
 *
 */

/// Access API that stores a T* in storage
template<typename T> class TypeDefaultAccess
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return *storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    if (ptr)
      return ptr;
    void* res = detail::TypeFactory<T>()();
    if (!res)
      qiLogError("qi.meta") << "initializeStorage error on " << typeid(T).name();
    return res;
  }
};

/// Access api that stores T in storage
template<typename T> class TypeDirectAccess
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    if (ptr)
    {
      T val = *(T*)ptr;
      return *(void**)&val;
    }
    else
      return 0;
  }
};

template<typename A> class TypeDefaultClone
{
public:
  typedef typename A::type type;
  static void* clone(void* src)
  {
    const type* ptr = (const type*)A::ptrFromStorage(&src);
    void* res = A::initializeStorage();
    type* tres = (type*)A::ptrFromStorage(&res);
    *tres = *ptr;
    return res;
  }

  static void destroy(void* src)
  {
    type* ptr = (type*)A::ptrFromStorage(&src);
    delete ptr;
  }
};

template<typename A> class TypeNoClone
{
public:
  static void* clone(void* src)
  {
    return src;
  }

  static void destroy(void* ptr)
  {
    /* Assume a TypeNoClone is not serializable
     * So it cannot have been allocated by us.
     * So the destroy comes after a clone->ignore it
     */
  }
};

template<typename A>class TypeDefaultValue
{
public:
  typedef typename A::type type;
  static bool toValue(const void* src, qi::detail::DynamicValue& val)
  {
    const type* ptr = (const type*)A::ptrFromStorage((void**)&src);
    detail::DynamicValueConverter<type>::writeDynamicValue(*ptr, val);
    return true;
  }

  static void* fromValue(const qi::detail::DynamicValue& val)
  {
    void* storage;
    storage = A::initializeStorage();
    void* vptr = A::ptrFromStorage(&storage);
    qiLogDebug("wtf") << storage << ' ' << &storage << ' ' << vptr;
    detail::DynamicValueConverter<type>::readDynamicValue(val, *(type*)vptr);
    return storage;
  }
};


template<typename A>class TypeNoValue
{
public:
  typedef typename A::type type;
  static bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    qiLogWarning("qi.type") << "toValue not implemented for type " << typeid(type).name();
    return false;
  }

  static void* fromValue(const qi::detail::DynamicValue& val)
  {
    qiLogWarning("qi.type") << "fromValue not implemented for type " << typeid(type).name();
    return 0; // We may not have a default constructor
  }
};

template<typename A> class TypeDefaultSerialize
{
public:
  typedef typename A::type type;
  static void  serialize(ODataStream& s, const void* src)
  {
    const type* ptr = (const type*)A::ptrFromStorage((void**)&src);
    s << *ptr;
  }

  static void* deserialize(IDataStream& s)
  {
    void* storage = A::initializeStorage();
    type* ptr = (type*)A::ptrFromStorage(&storage);
    s >> *ptr;
    return storage;
  }
  static std::string signature()
  {
    return signatureFromType<type>::value();
  }
};

template<typename T> class TypeNoSerialize
{
public:
  static void serialize(ODataStream& s, const void* ptr)
  {
    qiLogWarning("qi.meta") << "serialize not implemented for " << typeid(T).name();
  }

  static void* deserialize(IDataStream& s)
  {
    qiLogWarning("qi.meta") << "deserialize not implemented for " << typeid(T).name();
    return 0;
  }
 static  std::string signature()
  {
    std::string res;
    res += (char)Signature::Type_Unknown;
    return res;
  }
};


/* TypeImpl implementation that bounces to the various aspect
 * subclasses.
 *
 * That way we can split the various aspects in different classes
 * for better reuse, without the cost of a second virtual call.
 */
template<typename T, typename Access    = TypeDefaultAccess<T>
                   , typename Cloner    = TypeDefaultClone<Access>
                   , typename Value     = TypeNoValue<Access>
                   , typename Serialize = TypeNoSerialize<Access>
         > class DefaultTypeImpl
: public Cloner
, public Value
, public Serialize
, public virtual Type
{
  virtual void* initializeStorage(void* ptr=0)
  {
    return Access::initializeStorage(ptr);
  }

  virtual void* ptrFromStorage(void** storage)
  {
    return Access::ptrFromStorage(storage);
  }

  virtual const std::type_info& info()
  {
    return typeid(T);
  }

  virtual void* clone(void* src)
  {
    return Cloner::clone(src);
  }

  virtual void destroy(void* ptr)
  {
    Cloner::destroy(ptr);
  }

  virtual bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    return Value::toValue(ptr, val);
  }

  virtual void* fromValue(const qi::detail::DynamicValue& val)
  {
    return Value::fromValue(val);
  }

  virtual std::string signature()
  {
    return Serialize::signature();
  }

  virtual void  serialize(ODataStream& s, const void* ptr)
  {
    Serialize::serialize(s, ptr);
  }

  virtual void* deserialize(IDataStream& s)
  {
    return Serialize::deserialize(s);
  }
};

/* Type "factory". Specialize this class to provide a custom
 * Type for a given type.
 */
template<typename T> class TypeImpl: public virtual DefaultTypeImpl<T>
{
};

/// Declare a type that is convertible to GenericValue, and serializable
#define QI_TYPE_CONVERTIBLE_SERIALIZABLE(T)  \
namespace qi {                                   \
template<> class TypeImpl<T>:                \
  public DefaultTypeImpl<T,                  \
    TypeDefaultAccess<T>,                    \
    TypeDefaultClone<TypeDefaultAccess<T> >, \
    TypeDefaultValue<TypeDefaultAccess<T> >,  \
    TypeDefaultSerialize<TypeDefaultAccess<T> >  \
  >{}; }

/** Declare that a type is not convertible to GenericValue.
 *  Must be called outside any namespace.
 */
#define QI_TYPE_SERIALIZABLE(T)              \
namespace qi {                                   \
template<> class TypeImpl<T>:                \
  public DefaultTypeImpl<T,                  \
    TypeDefaultAccess<T>,                    \
    TypeDefaultClone<TypeDefaultAccess<T> >, \
    TypeNoValue<TypeDefaultAccess<T> >,       \
    TypeDefaultSerialize<TypeDefaultAccess<T> >   \
  >{}; }

/// Declare that a type has no metatype and cannot be used in a Value
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}

/// Declare that a type with default TypeImpl is not clonable
#define QI_TYPE_NOT_CLONABLE(T) \
namespace qi {    \
  template<> struct TypeDefaultClone<TypeDefaultAccess<T> >: public TypeNoClone<TypeDefaultAccess<T> >{}; \
}

/// Type factory getter. All other type access mechanism bounce here
QIMESSAGING_API Type*  getType(const std::type_info& type);
/// Type factory setter
QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type);

/** Get type from a type. Will return a static TypeImpl<T> if T is not registered
 */
template<typename T> Type* typeOf();

/// Get type from a value. No need to delete the result
template<typename T> Type* typeOf(const T& v)
{
  return typeOf<T>();
}

}

QI_TYPE_SERIALIZABLE(Buffer);

#include <qimessaging/details/type.hxx>

#endif  // _QIMESSAGING_TYPE_HPP_
