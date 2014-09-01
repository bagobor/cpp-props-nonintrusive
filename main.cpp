#include "props.h"

class A {
public:
	static void reg(){
		registry::reg("value", &A::value);
		registry::reg("bvalue", &A::get, &A::set);
		registry::reg("cvalue", &A::cget);
		registry::reg("cvalue", &A::dvalue, true);
		registry::reg("const_value", &A::const_value);
	}
	A() :value(-1), const_value(10.0f){
	}
private:
	void set(int i) {
		bvalue = i;
	}
	int get() const {
		return bvalue;
	}
	int cget() const { return cvalue; }

	int value, bvalue, cvalue, dvalue;
	const float const_value;
};

void test_props()
{
	A::reg();

	const property& p = registry::get<A>("value");

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
