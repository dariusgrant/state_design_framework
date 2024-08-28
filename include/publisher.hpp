#include <vector>

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