#include <algorithm>
#include <functional>
#include <tuple>



/*
Class `event` represents an event that occurs under the context of an object's behaviors or interactions. It can subscribe callbacks that contain `args_t` parameters within their signatures and can will notify each subscriber when invoked.
*/
template <typename... args_t> class event {
public:
  using argument_types = std::tuple<args_t...>;
  using event_signature = std::function<void(args_t...)>;

private:
  std::vector<event_signature> callbacks;

public:
  void subscribe(event_signature callback) { callbacks.push_back(callback); }

  //   void unsubscribe(event_signature callback) { callbacks.erase(callback); }

  void notify(args_t... args) {
    std::for_each(callbacks.begin(), callbacks.end(),
                  [&](event_signature cb) { cb(args...); });
  }
};
