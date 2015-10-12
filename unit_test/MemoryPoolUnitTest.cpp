#include <iostream>
#include <string>
#include <list>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <netinet/in.h>

#include "StandardMemoryPool.hpp"
#include "MemoryPool.hpp"

using namespace CppUnit;
using namespace std;

#define TEST_POOL_VOL 1024

//-----------------------------------------------------------------------------

class TestMemoryPool : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMemoryPool);
    CPPUNIT_TEST(testAllocation);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void);

protected:
    void testAllocation(void);

private:
    StandardMemoryPool *m_test_pool;
};

//-----------------------------------------------------------------------------

void TestMemoryPool::testAllocation(void)
{
  CPPUNIT_ASSERT(NULL == m_test_pool->allocate(0));
  CPPUNIT_ASSERT(NULL == m_test_pool->allocate(MAX_MEMPOOL_SIZE+1));
  CPPUNIT_ASSERT(NULL == m_test_pool->allocate(TEST_POOL_VOL-1));
  CPPUNIT_ASSERT(NULL == m_test_pool->allocate(TEST_POOL_VOL+1));
  CPPUNIT_ASSERT(NULL != m_test_pool->allocate(16));
}

void TestMemoryPool::setUp(void)
{
    m_test_pool = new StandardMemoryPool(TEST_POOL_VOL, 0);
}

void TestMemoryPool::tearDown(void)
{
    delete m_test_pool;
}

//-----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryPool);

int main(int argc, char* argv[])
{
    // informs test-listener about testresults
    CPPUNIT_NS::TestResult testresult;

    // register listener for collecting the test-results
    CPPUNIT_NS::TestResultCollector collectedresults;
    testresult.addListener (&collectedresults);

    // register listener for per-test progress output
    CPPUNIT_NS::BriefTestProgressListener progress;
    testresult.addListener (&progress);

    // insert test-suite at test-runner by registry
    CPPUNIT_NS::TestRunner testrunner;
    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
    testrunner.run(testresult);

    // output results in compiler-format
    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write ();

    // Output XML for Jenkins CPPunit plugin
    ofstream xmlFileOut("cppTestBasicMemoryPoolResults.xml");
    XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    xmlOut.write();

    // return 0 if tests were successful
    return collectedresults.wasSuccessful() ? 0 : 1;
}


