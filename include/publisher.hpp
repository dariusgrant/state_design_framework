#include <vector>
#include <functional>

template <class T>
class Publisher {
    protected:
    std::vector<T*> subscribers;

    public:
    void add_subscriber(T* sub) {
        subscribers.push_back(sub);
    }

    template <class... Args>
    void notify(T* sub, std::function<void(T*)> func, [[maybe_unused]] Args... args) {
        func(sub);
    }
    
    template <class... Args>
    void notify_all(std::function<void(T*)> func, [[maybe_unused]] Args... args) {
        for(auto& sub : subscribers) {
            notify(sub, func);
        }
    }

};