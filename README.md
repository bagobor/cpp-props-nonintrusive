_Intro:_

Once I've been working on a MMORPG PC project based on modified version of Reality Engine (more info http://www.artificialstudios.com/features.php). 
Engine architecture was centered around Actor class and its derivatives. 
And serialization was done via stored list of class propterties ie something like 
```
class Actor {
   ...
   std::list<Property> m_props;
   ...
};
```
and 
```
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
