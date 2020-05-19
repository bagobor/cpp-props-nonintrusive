#include "metaprops/props.h"

#include <iostream>
#include <functional>

#include <stdio.h>
// // #include "gtest/gtest.h"

// // IndependentMethod is a test case - here, we have 2 tests for this 1 test case
// TEST(IndependentMethod, ResetsToZero) {
// 	int i = 3;
// 	// independentMethod(i);
// 	EXPECT_EQ(0, i);

// 	i = 12;
// 	// independentMethod(i);
// 	EXPECT_EQ(0,i);
// }

// TEST(IndependentMethod, ResetsToZero2) {
// 	int i = 0;
// 	// independentMethod(i);
// 	EXPECT_EQ(0, i);
// }

// // The fixture for testing class Project1. From google test primer.
// class Project1Test : public ::testing::Test {
// protected:
// 	// You can remove any or all of the following functions if its body
// 	// is empty.

// 	Project1Test() {
// 		// You can do set-up work for each test here.
// 	}

// 	virtual ~Project1Test() {
// 		// You can do clean-up work that doesn't throw exceptions here.
// 	}

// 	// If the constructor and destructor are not enough for setting up
// 	// and cleaning up each test, you can define the following methods:
// 	virtual void SetUp() {
// 		// Code here will be called immediately after the constructor (right
// 		// before each test).
// 	}

// 	virtual void TearDown() {
// 		// Code here will be called immediately after each test (right
// 		// before the destructor).
// 	}

// 	// Objects declared here can be used by all tests in the test case for Project1.
// 	// Project1 p;
// };



// // Test case must be called the class above
// // Also note: use TEST_F instead of TEST to access the test fixture (from google test primer)
// TEST_F(Project1Test, MethodBarDoesAbc) {
// 	int i = 0;
// 	// p.foo(i); // we have access to p, declared in the fixture
// 	EXPECT_EQ(1, i);
// }



//inline void pl(const char* str) {
//	std::cout << str << std::endl; 
//}

struct base_a {
};

template<class T>
struct props_auto_reg {
public:
	typedef typename T _owner;
	props_auto_reg() { auto_reg; }
private:
	struct type_registrator {
		type_registrator() {
			_owner::reg();
		}
	};

	static const type_registrator auto_reg;
};

template<class T>
typename const props_auto_reg<T>::type_registrator props_auto_reg<T>::auto_reg;

class A : public props_auto_reg<A>{
public:
	static void reg(){
		registry::class_<A>()
		// registry::class_<A>("A")
			.base<base_a>()
			.prop("uval", &A::uval)
			.prop("value", &A::value, false, "min=0;max=10;default=5;step=1;editor=1")
			.reg("bvalue", &A::get, &A::set)
			.reg("cvalue", &A::cget)
			.prop("dvalue", &A::dvalue, true)
			.prop("const_value", &A::const_value);
	}
	A() :value(-1), const_value(10.0f){
	}
	
private:
	void set(int i) {
		printf("set value/n");
		//pl("set value");
		bvalue = i;
	}
	int get() const {
		return bvalue;
	}
	int cget() const { return cvalue; }

	unsigned int uval = 0;
	int value = 0;
	int bvalue = 0;
	int cvalue = 0;
	int dvalue = 0;;
	const float const_value;
};

// variant type for simple properties
// nested properties
// inheritance

void test_props()
{
	//const property& p = registry::get<A>("value");
	auto class_reg = registry::class_<A>();

	auto& p = registry::get<A>("value");
	auto& up = registry::get<A>("uval");

	if (!p) return;

	A a;
	p.set(&a, 2);
	// printf("%d", a.value);
	p.set(&a, "5");

	// return;
	std::string svalue = p.get<A, std::string>(&a);
	int ivalue = p.get<A, int>(&a);

	const property& pc = registry::get<A>("const_value");
	if (!pc) return;
	float const_value = pc.get<A, float>(&a);

	const property& pb = registry::get<A>("bvalue");
	pb.set(&a, 5);
	int bv = pb.get<A, int>(&a);

	registry::Clear();
}

int main()
{
	//Test test; test.foo();

	//std::function<int(Test*)> mt = &Test::foo;
	//mt(&test);
	printf("Hello from the test");

	test_props();

	return 0;
}
