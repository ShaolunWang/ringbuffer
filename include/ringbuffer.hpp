#include <atomic>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <optional>

template <typename T> class RingBuffer {
public:
  RingBuffer(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;
  RingBuffer &operator=(RingBuffer &&) = delete;

  explicit RingBuffer(const size_t &capacity)
      : m_capacity{capacity},
        m_container(static_cast<T *>(std::malloc(sizeof(T) * capacity))),
        m_head{0}, m_tail{0} {
    // checking invariants:
    assert(m_capacity >= 2);
    if (!m_container) {
      throw std::bad_alloc();
    }
  }
  ~RingBuffer() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      size_t tail = m_tail;
      size_t head = m_head;
      while (tail != head) {
        std::destroy(m_container[tail]);
        // handling wrap around
        if (tail == m_capacity) {
          tail = 0;
        }
        tail++;
      }
    }
  }
  /**
   * @brief pushing an element to the ringbuffer
   * this function will construct a temporary element
   * @param elem
   */
  template <typename U>
  bool emplace(U &&v)
    requires std::constructible_from<T, U &&>
  // enforcing T to be constructible from U
  // allows us to perfect foward without having to worry about the types
  // note that constructible_from means "any argument you can legally pass to a
  // T constructor."

  {

    // here we only need to ensure the ordering in between m_tail since
    // we will update m_tail later
    const std::size_t current_tail = m_tail.load(std::memory_order_relaxed);

    // we are loading with acquire since consumer can update it
    // we want that everything is as is when we are checking
    // the ordering of the container.
    // The container should have head and tail representing
    // exactly what's there at this next check

    // invariant - if ringbuffer is full, i.e. next tail is directly pointing at
    // head,

    if ((current_tail + 1) % m_capacity ==
        m_head.load(std::memory_order_acquire)) {
      return false;
    }

    if constexpr (std::is_trivially_constructible_v<T, U &&>) {
      // trivially constructable, no need for placement new
      m_container[current_tail] = std::forward<U>(v);

    } else {

      // placement new
      // construct in place the object, I think this is equivalent of
      // using std::construct_at
      new (&m_container[current_tail]) T(std::forward<U>(v));
    }

    // this will make sure that the placement new always
    // happens before the store
    m_tail.store((current_tail + 1) % m_capacity, std::memory_order_release);
    return true;
  }

  /**
   * @brief removing and returning an element
   * at the top
   */
  std::optional<T> pop() {
    // same as the producer code, we only need to make sure the head is read
    // at the right order
    const std::size_t current_head = m_head.load(std::memory_order_relaxed);

    // invariant - making sure we are not popping empty ringbuffer
    if (current_head == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt;
    };

    auto *ptr = &m_container[current_head];
    std::optional<T> out = std::move(*ptr);

    // don't do destroy at for floats and ints
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(&m_container[current_head]);
    }

    m_head.store((current_head + 1) % m_capacity, std::memory_order_release);
    return out;
  };

private:
  T* m_container;
  // get the machine cache line size
  // so that we don't get cache
#ifdef __cpp_lib_hardware_interference_size
  static constexpr size_t cache_line_size =
      std::hardware_destructive_interference_size;
#else
  static constexpr size_t cache_line_size = 64;
#endif
  // we want to align them so that we don't
  // really get false sharing
  // making sure m_head and m_tail are not on the same cacheline
  alignas(cache_line_size) std::atomic<int> m_head;
  alignas(cache_line_size) std::atomic<int> m_tail;
  const size_t m_capacity;
};
