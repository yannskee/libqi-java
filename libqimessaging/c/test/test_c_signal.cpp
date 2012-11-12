/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/os.hpp>

#include <qimessaging/c/application_c.h>
#include <qimessaging/c/signal_c.h>

int res0 = 0;
int res1 = 0;
bool pass0 = false;
bool pass1 = false;

void signal_callback_0(void* val)
{
  int* value = (int*)val;

  res0 = (*value);
  pass0 = true;
}

void signal_callback_1(void* val)
{
  int* value = (int*)val;

  res1 = (*value);
  pass1 = true;
}

TEST(TestSignal, CreateAndDestroy)
{
  qi_signal_t* signal = qi_signal_create();
  qi_signal_destroy(signal);
}

TEST(TestSignal, SimpleSignalConnect)
{
  qi_signal_t* signal = qi_signal_create();

  qi_signal_connect(signal, signal_callback_0);

  int n = 42;
  pass0 = false;
  pass1 = false;
  qi_signal_trigger(signal, &n);

  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  qi_signal_destroy(signal);
}

TEST(TestSignal, MultipleSignalConnect)
{
  qi_signal_t* signal = qi_signal_create();

  qi_signal_connect(signal, signal_callback_0);
  qi_signal_connect(signal, signal_callback_1);

  int n = 21;
  pass0 = false;
  pass1 = false;
  qi_signal_trigger(signal, &n);

  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0 && pass1)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  EXPECT_EQ(res1, n);
  qi_signal_destroy(signal);
}

TEST(TestSignal, SignalDisconnect)
{
  qi_signal_t* signal = qi_signal_create();

  qi_signal_connect(signal, signal_callback_0);
  unsigned int l = qi_signal_connect(signal, signal_callback_1);
  EXPECT_TRUE(qi_signal_disconnect(signal, l));

  int n = 11;
  pass0 = false;
  pass1 = false;
  qi_signal_trigger(signal, &n);

  int timeout = 1000;
  while (timeout > 0)
  {
    qi::os::msleep(1);
    if (pass0)
      break;
    timeout -= 1;
  }

  EXPECT_EQ(res0, n);
  EXPECT_EQ(pass1, false);
  qi_signal_destroy(signal);
}

TEST(TestSignal, SignalDisconnectAll)
{
  qi_signal_t* signal = qi_signal_create();

  qi_signal_connect(signal, signal_callback_0);
  qi_signal_connect(signal, signal_callback_1);
  EXPECT_TRUE(qi_signal_disconnect_all(signal));

  pass0 = false;
  pass1 = false;
  int n = 1337;
  qi_signal_trigger(signal, &n);

  qi::os::msleep(5);

  EXPECT_EQ(pass1 && pass0, false);
  EXPECT_NE(res0, n);
  EXPECT_NE(res1, n);
  qi_signal_destroy(signal);
}

int main(int argc, char **argv) {
  qi_application_t*     app;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  int ret = RUN_ALL_TESTS();
  qi_application_destroy(app);
  return ret;
}