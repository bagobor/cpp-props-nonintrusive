#include <string>
#include <map>
#include <typeinfo>
#include <memory>
#include <sstream>
#include <vector>
#include <functional>
// #include <boost/lexical_cast.hpp>

// template<typename T>
// std::string to_string(const T& value);

// std::string to_string(int value) {return std::to_string(value);}
// std::string to_string(long value) {return std::to_string(value);}

namespace aux {
	template <typename T, typename V>
	T lexical_cast(const V& v)
	{
		std::ostringstream src;
		src << v;

	    std::istringstream iss;
	    iss.str(src.str());

		T out;
	    iss >> out;
	    // deal with any error bits that may have been set on the stream
		return out;
	}

	//template <typename T>
	//std::string lexical_cast(const T& value)
	//{
	//	std::istringstream iss;
	//	iss << value;
	//	// deal with any error bits that may have been set on the stream
	//	return iss.str();
	//}
}


	struct prop_holder {
		enum data_type_t {
			wchar_value,
			char16_value,
			char32_value,
			char_value,			
			uchar_value,
			short_value,
			ushort_value,
			integer_value, 
			uinteger_value, 
			float_value,
			double_value,
			bool_value,
			longlong_value,
			unknowns_type_value,
			data_type_mask = 0xff
		};

		
		template<typename V> static data_type_t get_type_enum() { return unknowns_type_value; }
		template<> static data_type_t get_type_enum<int>() { return integer_value; }
		template<> static data_type_t get_type_enum<unsigned int>() { return uinteger_value; }
		template<> static data_type_t get_type_enum<wchar_t>() { return wchar_value; }
		//template<> static data_type_t get_type_enum<char16_t>() { return char16_value; }
		//template<> static data_type_t get_type_enum<char32_t>() { return char32_value; }
		template<> static data_type_t get_type_enum<char>() { return char_value; }
		template<> static data_type_t get_type_enum<unsigned char>() { return uchar_value; }
		template<> static data_type_t get_type_enum<short>() { return short_value; }
		template<> static data_type_t get_type_enum<unsigned short>() { return ushort_value; }
		template<> static data_type_t get_type_enum<float>() { return float_value; }
		template<> static data_type_t get_type_enum<double>() { return double_value; }
		template<> static data_type_t get_type_enum<bool>() { return bool_value; }
		template<> static data_type_t get_type_enum<long long>() { return longlong_value; }


		//TODO:
		enum container_type_t {
			stdstring_type = 1 << 16,
			stdvector_type = 2 << 16,
			stdlist_type = 4 << 16,
			stdcontainer_type_mask = 0xff0000
		};
		//template<typename >
		//template<> static data_type_t get_type_enum<std::string>() { return std_string_value; }

		virtual data_type_t get_valuetype_enum() const { return unknowns_type_value; }
		
		prop_holder() : read_only(false) {}
		virtual ~prop_holder() {}
		virtual void from_string(void* owner, const std::string&) const = 0;
		virtual std::string to_string(const void* owner) const = 0;
		bool read_only;
	};

	template<class OwnerType, class ValueType>
	struct prop_holder_t : prop_holder{

		data_type_t get_valuetype_enum() const { return get_type_enum<ValueType>(); }

		virtual ValueType get(const OwnerType*) const = 0;
		virtual void set(OwnerType*, const ValueType&) const = 0;
	};

	template<class T, class V>
	struct prop_holder_m_const : prop_holder_t<T, V>{
		typedef typename V const T::* const mptr_t;
		mptr_t ptr;

		prop_holder_m_const (mptr_t p): ptr(p) {read_only = true;}	

		virtual void set(T* po, const V& v) const {return;}
		virtual void from_string(void* owner, const std::string& str) const {return;}
		
		virtual V get(const T* po) const { return po ? po->*(ptr) : V(0);}
		virtual std::string to_string(const void* owner) const {
			T* ot = (T*)owner;
			return aux::lexical_cast<std::string>((*ot).*ptr);
		}
	};

	template<class T, class V>
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
			(*ot).*ptr = aux::lexical_cast<V>(str);
		}

		virtual std::string to_string(const void* owner) const {
			T* ot = (T*)owner;
			return aux::lexical_cast<std::string>((*ot).*ptr);
		}
	};

	template<class OwnerType, typename ValueType>
	struct prop_holder_f : prop_holder_t<OwnerType, ValueType> {
		//typedef void (OwnerType::* setter_t) (ValueType);
		//typedef ValueType (OwnerType::* getter_t) () const;
		typedef std::function<void(OwnerType*, ValueType)> setter_t;
		typedef std::function<ValueType(const OwnerType*)> getter_t;
		//typedef ValueType (OwnerType::* getter_t) () const;
		//std::function<ValueType(OwnerType*)> ffff;

		getter_t getter;
		setter_t setter;

		prop_holder_f(getter_t g, setter_t s): getter(g), setter(s) 
			{read_only = s ? false : true;}

		virtual ValueType get(const OwnerType* po) const {
			//return po ? (po->*getter)() : ValueType(0);
			return po ? getter(po) : ValueType(0);
		}

		virtual void set(OwnerType* po, const ValueType& v) const {
			if (read_only || !po) return;
			//(po->*setter)(v);
			setter(po, v);
		}

		virtual void from_string(void* owner, const std::string& str) const {
			if (read_only || !owner) return;
			OwnerType* ot = (OwnerType*)owner;
			ValueType v = aux::lexical_cast<ValueType>(str);
			//(ot->*setter)(v);
			setter(ot, v);
		}

		virtual std::string to_string(const void* owner) const {
			if (!owner) return std::string();
			OwnerType* ot = (OwnerType*)owner;			
			//ValueType v = (ot->*getter)();
			ValueType v = getter(ot);
			return aux::lexical_cast<std::string>(v);
		}
	};


class property {
public:	
	property(const std::type_info& owner_type, const std::type_info& value_type, prop_holder* holder, const char* name, const char* attribs=nullptr)
		: value_type(value_type), owner_type(owner_type), holder(holder), name(name?name:""), atributes(attribs? attribs:"")
	{
		data_type = holder->get_valuetype_enum();
	}

	~property() {
		delete holder; 
		holder = 0;
	}
	
	//data member property
	template<class T, class V>
	static std::shared_ptr<property> create(V T::* p, bool read_only, const char* name, const char* attribs) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);
		const auto value_type_name = vt.name();
		prop_holder* holder = 0;
		if (read_only)
			holder = new prop_holder_m_const <T,V>(p);
		else
			holder = new prop_holder_m <T,V>(p);

		std::shared_ptr<property> out ( new property(ot, vt, holder, name, attribs) );
		return out;
	}

	//handle case of const data member
	template<class T, class V>
	static std::shared_ptr<property> create(V const T::* const p, const char* name, const char* attribs) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);
		const auto value_type_name = vt.name();
		prop_holder* holder = new prop_holder_m_const <T,V>(p);
		std::shared_ptr<property> out ( new property(ot, vt, holder, name, attribs) );
		return out;
	}

	template<class T, class V>
	static std::shared_ptr<property> create(V(T::* getter) () const, void (T::* setter) (V), const char* name, const char* attribs) {
		const std::type_info& ot = typeid(T);
		const std::type_info& vt = typeid(V);		
		prop_holder* holder = new prop_holder_f<T,V>(getter, setter);
		std::shared_ptr<property> out (new property(ot, vt, holder, name, attribs) );
		return out;
	}

	template<class T>
	void set(T* owner, const char* s) const{
		if (typeid(const char*)!=value_type)
			set(owner, std::string(s));
		else
			set<T, const char*>(owner, s);
	}

	template<class T, class V>
	void set(T* owner, V v) const{
		if (typeid(T)!=owner_type) return;
		if (typeid(V)==value_type){
			prop_holder_t<T, V>* h = static_cast<prop_holder_t<T, V>*>(holder);
			h->set(owner, v);
		}
		else{
			std::string s = aux::lexical_cast<std::string>(v);
			holder->from_string(owner, s);
		}
	}

	template<class T, class V>
	V get(const T* owner) const{
		if (typeid(T)!=owner_type) return V(0);
		if (typeid(V)!=value_type)
			return aux::lexical_cast<V>(holder->to_string(owner));

		prop_holder_t<T, V>* h = (prop_holder_t<T, V>*)holder;
		return h->get(owner);
	}

	const std::type_info& owner_type;
	const std::type_info& value_type;

	operator bool() const { return this!=nullptr && owner_type != typeid(void); }

	std::string name;
	std::string atributes;
	prop_holder::data_type_t data_type;

private:

	prop_holder* holder;
};

struct registry{
	typedef std::map<std::string, std::shared_ptr<property> > properties_t;

	struct void_type {};

	struct class_record_t {

		std::vector<class_record_t*> base_classes;

		static const property& empty_property() {
			static std::shared_ptr<property> empty_one = property::create<void_type, int>(nullptr, true, nullptr, nullptr);
			return *empty_one;
		}


		class_record_t(const type_info& ti) :ti(ti){}
		properties_t properties;
		const type_info& ti;


		template<class T, class V>
		class_record_t& prop(const char* name, V T::*ptr, bool read_only = false, const char* attribs = nullptr){
			//TODO: check is nested property!
			std::shared_ptr<property> p = property::create<T, V>(ptr, read_only, name,attribs);
			properties[name] = p;
			return *this;
		}

		template<class T, class V>
		class_record_t& prop(const char* name, V const T::* const ptr, const char* attribs = nullptr){
			std::shared_ptr<property> p = property::create<T, V>(ptr,name,attribs);
			properties[name] = p;
			return *this;
		}

		template<class T, class V>
		class_record_t& reg(const char* name, V(T::* getter) () const, void (T::* setter) (V) = nullptr, const char* attribs = nullptr){
			std::shared_ptr<property> p = property::create<T, V>(getter, setter,name,attribs);
			properties[name] = p;
			return *this;
		}

		template<class T>
		class_record_t& base() {
			base_classes.push_back(&registry::class_<T>());
			return *this;
		}

	};

	typedef std::map<std::string, std::shared_ptr<class_record_t>> types_t;

	template<class T>
	static class_record_t& class_() {
		const type_info& ti = typeid(T);
		auto it = types.find(ti.name());
		if (it == types.end()) {
			std::shared_ptr<class_record_t> p(new class_record_t(ti));
			types[ti.name()] = p;
			return *p;
		}

		return *it->second;
	}

	template<class T>
	static const property& get(const char* name){
		const type_info& ti = typeid(T);

		auto t = types.find(ti.name());
		if (t == types.end()) return class_record_t::empty_property();

		auto & properties = t->second->properties;
		auto p = properties.find(name);
		if (p == properties.end()) return class_record_t::empty_property();

		return *(p->second);
	}

	static void Clear() { types.clear(); }
private:
	static types_t types;
};

registry::types_t registry::types;
