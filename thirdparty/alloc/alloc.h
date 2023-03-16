#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

template <typename T, std::size_t N>
class fixed_size_allocator
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template <typename U>
    struct rebind 
	{
        using other = fixed_size_allocator<U, N>;
    };

    fixed_size_allocator() noexcept {}
    fixed_size_allocator(const fixed_size_allocator&) noexcept {}
    template <typename U>
    fixed_size_allocator(const fixed_size_allocator<U, N>&) noexcept {}
    ~fixed_size_allocator() noexcept {}

    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }
        if (index_ + n > N) {
            throw std::bad_alloc();
        }
        pointer p = data_ + index_;
        index_ += n;
        return p;
    }

    void deallocate(pointer p, size_type n) noexcept {}

    size_type max_size() const noexcept {
        return N;
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

private:
    pointer data_ = reinterpret_cast<pointer>(std::malloc(N * sizeof(T)));
    size_type index_ = 0;
};

template <typename T, std::size_t N>
bool operator==(const fixed_size_allocator<T, N>&, const fixed_size_allocator<T, N>&) noexcept {
    return true;
}

template <typename T, std::size_t N>
bool operator!=(const fixed_size_allocator<T, N>&, const fixed_size_allocator<T, N>&) noexcept {
    return false;
}
