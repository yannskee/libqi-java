/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     AL_MESSAGING_DETAIL_TYPESIGNATURE_HPP_
# define     AL_MESSAGING_DETAIL_TYPESIGNATURE_HPP_

# include <string>
# include <vector>
# include <map>
# include <boost/utility.hpp>
# include <boost/function_types/is_function_pointer.hpp>
# include <boost/function_types/is_function.hpp>

namespace AL {
  namespace detail {

    #define SIMPLE_SIGNATURE(THETYPE, THENAME)                               \
    template <>                                                              \
    struct signature<THETYPE>  {                                             \
      static std::string &value(std::string &val) {                          \
        val += THENAME;                                                      \
        return val;                                                          \
      }                                                                      \
    };

    template <typename T, class Enable = void>
    struct signature {
      static std::string &value(std::string &val) {
        val += "UNKNOWN";
        return val;
      }
    };

    SIMPLE_SIGNATURE(void       , "v");
    SIMPLE_SIGNATURE(bool       , "b");
    SIMPLE_SIGNATURE(char       , "c");
    SIMPLE_SIGNATURE(int        , "i");
    SIMPLE_SIGNATURE(float      , "f");
    SIMPLE_SIGNATURE(double     , "d");
    SIMPLE_SIGNATURE(std::string, "s");

    //pointer
    template <typename T>
    struct signature<T*, typename boost::disable_if< boost::function_types::is_function<T> >::type> {
      static std::string &value(std::string &val) {
        AL::detail::signature<T>::value(val);
        val += "*"; return val;
      }
    };

    //& and const are not used for function signature, it's only a compiler detail
    template <typename T>
    struct signature<T&> {
      static std::string &value(std::string &val) {
        return AL::detail::signature<T>::value(val);
      }
    };

    template <typename T>
    struct signature<const T> {
      static std::string &value(std::string &val) {
        return AL::detail::signature<T>::value(val);
      }
    };

    //STL
    template <typename U>
    struct signature< std::vector<U> > {
      static std::string &value(std::string &val) {
        val += "[";
        AL::detail::signature<U>::value(val);
        val += "]";
        return val;
      }
    };

    template <typename T1, typename T2>
    struct signature< std::map<T1, T2> > {
      static std::string &value(std::string &val) {
        val += "{";
        AL::detail::signature<T1>::value(val);
        AL::detail::signature<T2>::value(val);
        val += "}";
        return val;
      }
    };

  }
}

#endif      /* !AL_MESSAGING_DETAIL_TYPESIGNATURE_HPP_ */
