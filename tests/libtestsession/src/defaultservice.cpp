/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include "defaultservice.hpp"

#include <qitype/genericobjectbuilder.hpp>
#include "defaultservice.hpp"

std::string __test_ping()
{
  return "pong";
}

qi::ObjectPtr DefaultService::getDefaultService()
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("ping", &__test_ping);
  return qi::ObjectPtr(ob.object());
}

bool DefaultService::generateUniqueServiceName(std::string &name)
{
  std::stringstream ss;
  qi::os::timeval   p;

  if (qi::os::gettimeofday(&p) < 0)
    return false;

  ss << "__default" << p.tv_sec << p.tv_usec;
  name = ss.str();
  return true;
}