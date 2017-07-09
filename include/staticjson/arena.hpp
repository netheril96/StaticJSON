#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace staticjson
{

constexpr size_t DEFAULT_BLOCK_SIZE = 4000;

template <size_t BlockSize = DEFAULT_BLOCK_SIZE>
class Arena
{
private:
    Arena(const Arena&) = delete;
    Arena(Arena&&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena& operator=(Arena&&) = delete;

private:
    struct Block
    {
        Block* next;
        bool is_dynamic;
        std::array<unsigned char, BlockSize> buffer;
    };

    Block m_inlined_block;
    Block* m_head_block;
    uintptr_t m_cursor;

public:
    explicit Arena()
    {
        m_inlined_block.next = nullptr;
        m_inlined_block.is_dynamic = false;
        m_head_block = &m_inlined_block;
        m_cursor = reinterpret_cast<uintptr_t>(m_inlined_block.buffer.data());
    }

    ~Arena()
    {
        while (m_head_block)
        {
            Block* next = m_head_block->next;
            if (m_head_block->is_dynamic)
            {
                ::operator delete(m_head_block);
            }
            m_head_block = next;
        }
    }

    template <class T>
    T* allocate(size_t n)
    {
        size_t bytes = n * sizeof(T);
        if (bytes + alignof(T) > BlockSize)
            return static_cast<T*>(::operator new(bytes));

        uintptr_t aligned_cursor = (m_cursor + (alignof(T) - 1)) & ~(alignof(T) - 1);
        assert(aligned_cursor >= m_cursor && aligned_cursor % alignof(T) == 0);
        if (aligned_cursor + bytes
            <= reinterpret_cast<uintptr_t>(m_head_block->buffer.data() + BlockSize))
        {
            m_cursor = aligned_cursor + bytes;
            return reinterpret_cast<T*>(aligned_cursor);
        }
        else
        {
            auto new_block = static_cast<Block*>(::operator new(sizeof(Block)));
            new_block->next = m_head_block;
            new_block->is_dynamic = true;
            m_head_block = new_block;
            m_cursor = reinterpret_cast<uintptr_t>(m_head_block->buffer.data());
            return allocate<T>(n);
        }
    }

    template <class T>
    static void deallocate(T* ptr, size_t n)
    {
        size_t bytes = n * sizeof(T);
        if (bytes + alignof(T) > BlockSize)
            ::operator delete(ptr);
        // Otherwise no-op
    }
};

template <class T, size_t BlockSize = DEFAULT_BLOCK_SIZE>
class ArenaAllocator
{
private:
    Arena<BlockSize>* m_arena;

public:
    explicit ArenaAllocator(Arena<BlockSize>* arena) : m_arena(arena) {}

    T* allocate(size_t n) { return m_arena->template allocate<T>(n); }

    void deallocate(T* ptr, size_t n) { return m_arena->template deallocate<T>(ptr, n); }

    Arena<BlockSize>* get_arena() const noexcept { return m_arena; }

    bool operator==(const ArenaAllocator& other) const noexcept
    {
        return get_arena() == other.get_arena();
    }
    bool operator!=(const ArenaAllocator& other) const noexcept
    {
        return get_arena() != other.get_arena();
    }

    template <class U>
    ArenaAllocator(const ArenaAllocator<U, BlockSize>& other) : m_arena(other.get_arena())
    {
    }

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;
    typedef T value_type;

    ArenaAllocator select_on_container_copy_construction() const { return ArenaAllocator(m_arena); }

    template <typename U>
    struct rebind
    {
        typedef ArenaAllocator<U> other;
    };
};

typedef std::basic_string<char, std::char_traits<char>, ArenaAllocator<char>> astring;

template <class T, size_t BlockSize = DEFAULT_BLOCK_SIZE>
class ArenaPtr
{
private:
    T* m_ptr;

public:
    explicit ArenaPtr() : m_ptr(nullptr) {}

    template <class... Args>
    explicit ArenaPtr(Arena<BlockSize>* arena, Args&&... args)
    {
        m_ptr = arena->template allocate<T>(1);
        try
        {
            new (m_ptr) T(std::forward<Args>(args)...);
        }
        catch (...)
        {
            arena->template deallocate<T>(m_ptr, 1);
        }
    }

    ~ArenaPtr() { reset(); }

    T* get() noexcept { return m_ptr; }
    const T* get() const noexcept { return m_ptr; }
    T& operator*() noexcept { return *m_ptr; }
    const T& operator*() const noexcept { return *m_ptr; }
    T* operator->() noexcept { return get(); }
    const T* operator->() const noexcept { return get(); }

    void reset()
    {
        if (m_ptr)
        {
            m_ptr->~T();
            Arena<BlockSize>::template deallocate<T>(m_ptr, 1);
        }
        m_ptr = nullptr;
    }

    void swap(ArenaPtr& other) noexcept { std::swap(m_ptr, other.m_ptr); }
    void swap(ArenaPtr&& other) noexcept { std::swap(m_ptr, other.m_ptr); }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    bool operator!() const noexcept { return m_ptr == nullptr; }
};

template <class T, size_t BlockSize = DEFAULT_BLOCK_SIZE>
void swap(ArenaPtr<T, BlockSize>& a, ArenaPtr<T, BlockSize>& b)
{
    a.swap(b);
}
}
