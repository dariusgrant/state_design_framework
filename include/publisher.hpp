#include <vector>
#include <type_traits>
#include <iostream>

template <class T>
class Publisher {
    template <typename... Args>
    using CallbackType = void (T::*)(Args...);

    protected:
    std::vector<T*> subscribers;

    public:
    void add_subscriber(T* sub) {
        subscribers.push_back(sub);
    }

    template <class C>
    void remove_subscriber(const C& c){
        std::erase_if(subscribers, [c](T* sub){ 
            auto t = std::is_base_of_v<T, C>;
            if (t == true) {
                std::cout << "Removing subscriber: " << c.name << "\n";
            }
            return t;  
        });
    }

    template <class... Args>
    void notify(T* sub, CallbackType<Args...> func, [[maybe_unused]] Args... args) {
        (sub->*func)(args...);
    }
    
    template <class... Args>
    void notify_all(CallbackType<Args...> func, [[maybe_unused]] Args... args) {
        for(auto& sub : subscribers) {
            notify(sub, func, args...);
        }
    }
};

template <class T>
class Subscriber {
    
    std::vector<Publisher<T>*> publishers;
    public:
    const char* name;
    public:
    Subscriber(const char* name) : name(name) {}
    void subscribe_to(Publisher<T>& publisher) {
        publisher.add_subscriber(static_cast<T*>(this));
    }

    void unsubscribe_from(Publisher<T>& publisher) {
        publisher.remove_subscriber(*this);
    }
};