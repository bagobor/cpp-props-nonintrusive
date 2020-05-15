[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/bagobor/cpp-props-nonintrusive) 

_Intro:_

Once I've been working on a MMORPG PC project based on modified version of Reality Engine (more info http://www.artificialstudios.com/features.php). 
Engine architecture was centered around Actor class and its derivatives. 
And serialization was done via stored list of class propterties ie something like 
```c++
class Actor {
   ...
   std::list<Property> m_props;
   ...
};
```
and 
```c++
struct Property {
    std::string name;
    union {...};
};
```

So you can imaging having more than 10 properties only in base Actor class and some more in it's every derivatives and over 10k actors loaded ... 
And it was targeted for 32bit :D
Huge mess! 
Not just memory allocation/fragmentation but loading/unloading times as well. 

So I can to idea create a lightweigh way to declare properties with storing this information inside the class itself. 
And here we are ... 

_Example:_
```c++
class A : public props_auto_reg<A>{
public:
	static void reg(){
		registry::class_<A>()
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
```

```c++
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
```
