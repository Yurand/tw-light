//#include "scp.h"
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>


class DataPathTest: public CppUnit::TestFixture
{
private:
public:
  void setUp()
  {
  }
  
  void testAdd()
  {
    CPPUNIT_ASSERT(1);
  }
  void tearDown()
  {
  }
  
  CPPUNIT_TEST_SUITE(DataPathTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST_SUITE_END();

};
CPPUNIT_TEST_SUITE_REGISTRATION( DataPathTest );
