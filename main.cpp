#include "props.h"

#include <iostream>

inline void pl(const char* str) { std::cout << str << std::endl; }

struct base_a {
};

class A {
public:
	static void reg(){
		registry::class_<A>()
			.base<base_a>()
			.reg("uval", &A::uval)
			.reg("value", &A::value, false, "editor=1")
			.reg("bvalue", &A::get, &A::set)
			.reg("cvalue", &A::cget)
			.reg("cvalue", &A::dvalue, true)
			.reg("const_value", &A::const_value);
	}
	A() :value(-1), const_value(10.0f){
	}
	
private:
	void set(int i) {
		pl("set value");
		bvalue = i;
	}
	int get() const {
		return bvalue;
	}
	int cget() const { return cvalue; }

	unsigned int uval;
	int value, bvalue, cvalue, dvalue;
	const float const_value;
};

// variant type for simple properties
// nested properties
// inheritance

void test_props()
{
	A::reg();

	//const property& p = registry::get<A>("value");
	auto& p = registry::get<A>("value");
	auto& up = registry::get<A>("uval");

	auto class_reg = registry::class_<A>();

	if (!p) return;

	A a;
	p.set(&a, 2);
	p.set(&a, "5");

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
	test_props();

	return 0;
}
