#ifndef _QIMESSAGING_METHODTYPE_HPP_
#define _QIMESSAGING_METHODTYPE_HPP_

#include <qimessaging/function_type.hpp>

namespace qi
{
  class QIMESSAGING_API MethodType: public virtual FunctionType
  {
  public:
    /// Call with all values of the correct type
    virtual void* call(void* method, void* object,
      const std::vector<void*>& args) = 0;
    /// Convert and call
    virtual GenericValue call(void* method, GenericValue object,
      const std::vector<GenericValue>& args) = 0;
  };

  class QIMESSAGING_API GenericMethod
  {
  public:
    GenericValue call(GenericValue object, const std::vector<GenericValue> args)
    {
      return type->call(value, object, args);
    }
    std::string signature() const { return type->signature();}
    std::string sigreturn() const { return type->sigreturn();}
    ///@return equivalent function value
    GenericFunction toGenericFunction();
    MethodType* type;
    void*       value;
  };

  template<typename T> MethodType* methodTypeOf();

  template<typename M>
  GenericMethod makeGenericMethod(const M& method);
}

#include <qimessaging/details/method_type.hxx>

#endif