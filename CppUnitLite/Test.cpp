#include "Test.h"


#include "TestRegistry.h"
#include "SimpleString.h"

#include <cmath>


Test::Test (const SimpleString& testName)
:	mTestName(testName), mNextTest(0)
{
	TestRegistry::addTest(this);
}


Test::~Test()
{
}


Test*
Test::getNext() const
{
	return mNextTest;
}


void
Test::setNext(Test* test)
{
	mNextTest = test;
}


bool
Test::DoublesNearlyEqual(double lhs, double rhs, double epsilon)
{
	return (fabs(lhs - rhs) < epsilon);
}
