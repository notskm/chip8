#ifndef CH8_OBSERVABLE_HPP
#define CH8_OBSERVABLE_HPP

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

namespace ch8 {
    template <typename... Args>
    class observable {
    public:
        auto attach(std::function<void(Args...)> observer) -> void
        {
            observers.emplace_back(std::move(observer));
        }

        auto notify(Args... args) -> void
        {
            for (auto& observer : observers) {
                observer(args...);
            }
        }

    private:
        std::vector<std::function<void(Args...)>> observers;
    };
} // namespace ch8

#endif // CH8_OBSERVABLE_HPP
