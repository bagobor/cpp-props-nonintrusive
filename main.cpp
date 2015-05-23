#include "props.h"

#include <iostream>

inline void pl(const char* str) { std::cout << str << std::endl; }

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
			.base<base_a>()
			.reg("uval", &A::uval)
			.reg("value", &A::value, false, "min=0;max=10;default=5;step=1;editor=1")
			.reg("bvalue", &A::get, &A::set)
			.reg("cvalue", &A::cget)
			.reg("cvalue", &A::dvalue, true)
			.reg("const_value", &A::const_value);
	}
	A() :value(-1), const_value(10.0f){
		//this->auto_reg;
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
