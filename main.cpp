#include <string>
#include <map>
#include <typeinfo>
#include <boost/lexical_cast.hpp>

	struct prop_holder {
		prop_holder() : read_only(false) {}
		virtual ~prop_holder() {}
		virtual void from_string(void* owner, const std::string&) const = 0;
		virtual std::string to_string(const void* owner) const = 0;
		bool read_only;
	};

	template<class OwnerType, typename ValueType>
	struct prop_holder_t : prop_holder{
		virtual ValueType get(const OwnerType*) const = 0;
		virtual void set(OwnerType*, const ValueType&) const = 0;
	};

	template<class T, typename V>
	struct prop_holder_m_const : prop_holder_t<T, V>{
		typedef typename V const T::* const mptr_t;
		mptr_t ptr;

		prop_holder_m_const (mptr_t p): ptr(p) {read_only = true;}	

		virtual void set(T* po, const V& v) const {return;}
		virtual void from_string(void* owner, const std::string& str) const {return;}
		
		virtual V get(const T* po) const { return po ? po->*(ptr) : V(0);}
		virtual std::string to_string(const void* owner) const {
			T* ot = (T*)owner;
			return boost::lexical_cast<std::string>((*ot).*ptr);
		}
	};

	template<class T, typename V>
	struct prop_holder_m : prop_holder_t<T, V>{
		typedef typename V T::*mptr_t;
		mptr_t ptr;

		prop_holder_m (mptr_t p): ptr(p) {read_only = false;}	

		virtual V get(const T* po) const { return po ? po->*(ptr) : V(0);}

		virtual void set(T* po, const V& v) const {
			if (read_only || !po) return;
			po->*(ptr) = v;
		}

		virtual void from_string(void* owner, const std::string& str) const {
			T* ot = (T*)owner;
			if (read_only || !owner) return;
			(*ot).*ptr = boost::lexical_cast<V>(str);
		}

		virtual std::string to_string(const void* owner) const {
			T* ot = (T*)owner;
			return boost::lexical_cast<std::string>((*ot).*ptr);
		}
	};

	template<class OwnerType, typename ValueType>
	struct prop_holder_f : prop_holder_t<OwnerType, ValueType>{
		//typedef typename ValueType OwnerType::*mptr_t;
		typedef void (OwnerType::* setter_t) (ValueType);
		typedef ValueType (OwnerType::* getter_t) () const;

		getter_t getter;
		setter_t setter;

		prop_holder_f(getter_t g, setter_t s): getter(g), setter(s) 
			{read_only = s ? false : true;}

		virtual ValueType get(const OwnerType* po) const {
			return po ? (po->*getter)() : ValueType(0);
		}

		virtual void set(OwnerType* po, const ValueType& v) const {
			if (read_only || !po) return;
			(po->*setter)(v);
		}

		virtual void from_string(void* owner, const std::string& str) const {
			if (read_only || !owner) return;
			OwnerType* ot = (OwnerType*)owner;			
			ValueType v = boost::lexical_cast<ValueType>(str);
			(ot->*setter)(v);
		}

		virtual std::string to_string(const void* owner) const {
			if (!owner) return std::string();
			OwnerType* ot = (OwnerType*)owner;			
			ValueType v = (ot->*getter)();
			return boost::lexical_cast<std::string>(v);
		}
	};


class property {

public:
	property() : value_type(typeid(void)), owner_type(typeid(void)), holder(0){}
	property(const std::type_info& owner_type, const std::type_info& value_type, prop_holder* holder)
		: value_type(value_type), owner_type(owner_type), holder(holder)
	{}

	~property() {
		delete holder; holder = 0;
	}
	
	//data member property
	template<class T, typename V>
	static property* create(V T::* p, bool read_only) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);
		prop_holder* holder = 0;
		if (read_only)
			holder = new prop_holder_m_const <T,V>(p);
		else
			holder = new prop_holder_m <T,V>(p);

		property* out = new property(ot, vt, holder);
		return out;
	}

	//handle case of const data member
	template<class T, typename V>
	static property* create(V const T::* const p) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);
		prop_holder* holder = new prop_holder_m_const <T,V>(p);
		property* out = new property(ot, vt, holder);
		return out;
	}

	template<class T, typename V>
	static property* create(V (T::* getter) () const, void (T::* setter) (V)) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);		
		prop_holder* holder = new prop_holder_f<T,V>(getter, setter);
		property* out = new property(ot, vt, holder);
		return out;
		//return 0;
	}

	template<class T>
	void set(T* owner, const char* s) const{
		if (typeid(const char*)!=value_type)
			set(owner, std::string(s));
		else
			set<T, const char*>(owner, s);
	}

	template<class T, typename V>
	void set(T* owner, V v) const{
		if (typeid(T)!=owner_type) return;
		if (typeid(V)==value_type){
			prop_holder_t<T, V>* h = static_cast<prop_holder_t<T, V>*>(holder);
			h->set(owner, v);
		}
		else{
			holder->from_string(owner, boost::lexical_cast<std::string>(v));
		}
	}

	template<class T, typename V>
	V get(const T* owner) const{
		if (typeid(T)!=owner_type) return V(0);
		if (typeid(V)!=value_type)
			return boost::lexical_cast<V>(holder->to_string(owner));

		prop_holder_t<T, V>* h = (prop_holder_t<T, V>*)holder;
		return h->get(owner);
	}

	const std::type_info& owner_type;
	const std::type_info& value_type;

	operator bool() const { return owner_type != typeid(void); }

private:
	prop_holder* holder;
};

struct registry{
	typedef std::map<std::string, property*> properties_t;
	typedef std::map<std::string, properties_t> types_t;

	template<class T, typename V>
	static const property& reg(const char* name, V T::*ptr, bool read_only = false){
		const type_info& ti = typeid(T);
		property* p = property::create<T, V>(ptr, read_only);
		types[ti.name()][name] = p;
		return *p;
	}

	template<class T, typename V>
	static const property& reg(const char* name, V const T::* const ptr){
		const type_info& ti = typeid(T);
		property* p = property::create<T, V>(ptr);
		types[ti.name()][name] = p;
		return *p;
	}

	template<class T, typename V>
	static const property& reg(const char* name, V (T::* getter) () const, void (T::* setter) (V)=0){
		const type_info& ti = typeid(T);
		property* p = property::create<T, V>(getter, setter);
		types[ti.name()][name] = p;
		return *p;
	}

	template<class T>
	static const property& get(const char* name){
		const type_info& ti = typeid(T);		
		return *(types[ti.name()][name]);
	}
private:
	static types_t types;
};

registry::types_t registry::types;

class A {
public:
	static void reg(){
		registry::reg("value", &A::value);
		registry::reg("bvalue", &A::get, &A::set);
		registry::reg("cvalue", &A::cget);
		registry::reg("cvalue", &A::dvalue, true);
		registry::reg("const_value", &A::const_value);
	}
	A():value(-1), const_value(10.0f){
	}
private:
	void set(int i ) {
		bvalue = i;
	}
	int get() const {
		return bvalue;
	}
	int cget() const {return cvalue;}

	int value, bvalue, cvalue, dvalue;
	const float const_value;
};

int main()
{
	A::reg();

	const property& p = registry::get<A>("value");

	if (!p) return 1;

	A a;
	p.set(&a, 2);
	p.set(&a, "5");

	std::string svalue = p.get<A, std::string>(&a);
	int ivalue = p.get<A, int>(&a);

	const property& pc = registry::get<A>("const_value");
	if (!pc) return 1;
	float const_value = pc.get<A, float>(&a);

	const property& pb = registry::get<A>("bvalue");
	pb.set(&a, 5);
	int bv = pb.get<A, int>(&a);

	return 0;
}
