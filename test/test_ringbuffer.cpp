#include "ringbuffer.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <thread>

using namespace std::chrono_literals;

TEST(RingBufferTest, StartsEmpty) {
  RingBuffer<int> q(8);
  EXPECT_FALSE(q.pop().has_value());
}

TEST(RingBufferTest, PushPopSingleElement) {
  RingBuffer<int> q(8);
  EXPECT_TRUE(q.emplace(42));
  auto out = q.pop();
  ASSERT_TRUE(out.has_value());
  EXPECT_EQ(*out, 42);
  EXPECT_FALSE(q.pop().has_value());
}

TEST(RingBufferTest, FillUntilFull) {
  constexpr size_t capacity = 4;
  RingBuffer<int> q(capacity);
  size_t emplaced = 0;
  while (q.emplace(emplaced))
    emplaced++;
  EXPECT_FALSE(q.emplace(999));
}

TEST(RingBufferTest, WrapAround) {
  constexpr size_t capacity = 4;
  RingBuffer<int> q(capacity);
  EXPECT_TRUE(q.emplace(1));
  EXPECT_TRUE(q.emplace(2));
  EXPECT_TRUE(q.emplace(3));

  EXPECT_EQ(q.pop(), 1);
  EXPECT_EQ(q.pop(), 2);

  EXPECT_TRUE(q.emplace(4));
  EXPECT_TRUE(q.emplace(5));

  EXPECT_EQ(q.pop(), 3);
  EXPECT_EQ(q.pop(), 4);
  EXPECT_EQ(q.pop(), 5);
  EXPECT_FALSE(q.pop().has_value());
}

TEST(RingBufferTest, ThreadedSPSCIntegrity) {
  constexpr size_t capacity = 1024;
  constexpr size_t iterations = 1'000'000;
  RingBuffer<size_t> q(capacity);
  std::vector<size_t> consumed;
  consumed.reserve(iterations);

  std::jthread producer([&] {
    for (size_t i = 0; i < iterations; ++i) {
      while (!q.emplace(i))
        std::this_thread::yield();
    }
  });

  std::jthread consumer([&] {
    size_t expected = 0;
    while (expected < iterations) {
      if (auto val = q.pop()) {
        ASSERT_EQ(*val, expected);
        consumed.emplace_back(*val);
        ++expected;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();
  ASSERT_EQ(consumed.size(), iterations);
}

TEST(RingBufferTest, ThreadedSmallCapacityWrap) {
  constexpr size_t capacity = 8;
  constexpr size_t iterations = 200'000;
  RingBuffer<int> q(capacity);
  std::atomic<bool> producer_done{false};
  std::atomic<int> last_seen{-1};

  std::jthread producer([&] {
    for (int i = 0; i < static_cast<int>(iterations); ++i) {
      while (!q.emplace(static_cast<size_t>(i)))
        std::this_thread::yield();
    }
    producer_done.store(true, std::memory_order_release);
  });

  std::jthread consumer([&] {
    int expected = 0;
    while (!producer_done.load(std::memory_order_acquire) ||
           expected < static_cast<int>(iterations)) {
      if (auto val = q.pop()) {
        EXPECT_EQ(*val, expected);
        last_seen.store(*val, std::memory_order_relaxed);
        ++expected;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();
  EXPECT_EQ(last_seen.load(), static_cast<int>(iterations - 1));
}
