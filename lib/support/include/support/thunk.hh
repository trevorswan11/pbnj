#pragma once

#include <concepts>
namespace pbnj {

template <typename Signature> class thunk;
template <typename Ret, typename... Args> class thunk<Ret(Args...)> {
  public:
    template <std::convertible_to<Ret> ActualRet>
    explicit thunk(ActualRet (*fn)(Args...)) : fn_ptr_{reinterpret_cast<void*>(fn)} {
        thunk_ = [](void* fn_ptr, Args&&... args) -> Ret {
            auto* original_fn = reinterpret_cast<ActualRet (*)(Args...)>(fn_ptr);
            return original_fn(std::forward<Args>(args)...);
        };
    }

    auto operator()(Args&&... args) const -> Ret {
        return thunk_(fn_ptr_, std::forward<Args>(args)...);
    }

  private:
    void* fn_ptr_{nullptr};
    Ret (*thunk_)(void*, Args&&...){nullptr};
};

} // namespace pbnj
