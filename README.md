RI - Rust inspired iterators for C++
====================================

This is an attempt to reproduce Rust-like iterators in C++.

I have read about [`ranges-v3`](https://github.com/ericniebler/range-v3) and also Rust's [`Iterator` trait](https://doc.rust-lang.org/std/iter/trait.Iterator.html).

I believe both approaches produce similar functionality. However, I was surprised with simplicity of `Iterator` implementaiton over `ranges-v3`.

So I decided to give it a try and play with those ideas in C++.

# Rust's `Iterator`

To iterate over a collection in Rust you would use a function from its Iterator trait. To implement this trait your type needs to define following:

```rust
trait Iterator {
    type Item;
    fn next(&mut self) -> Option<Self::Item>;
}
```

And that's it. If there's another element, next shall return Some(x), if the iterator is at end it produces None. No sentinels, std::end() and such. Very simple and elegant solution.

And that's not all - if you look at the definition of this trait in Rust source code you will see that it also defines a bunch of structs and adapter functions that can perform different operations on the iterator. Similar to views and actions in ranges-v3, but under the same umbrella.

And all this functionality relies on your implementation of next() function only. To me this already sounds like a very sound abstraction.

# Working with iterators

Here's a task taken from ranges-v3 documentation:

Generate an infinite list of integers starting at 1, square them, take the first 10, and sum them.

## Ranges (C++):

```c++
int sum = accumulate(view::ints(1)
                   | view::transform([](int i){return i*i;})
                   | view::take(10), 0);
```

## Rust:

```rust
let sum : i32 = (1..).map(|x| x*x).take(10).sum();
```

## With ri we want to be able to write (C++)

```c++
int sum = ri::gen(1)->map([](int x){ return x*x;})->take(10)->sum();
```

# Implementing Iterator in C++

Rust's trait loosely translates to an interface in C++. There is a crucial difference, but we'll leave it for later.

So we define an interface:

```c++
template <typename T>
class IIterator<T>
{
  public:
    // things we need to implement when we inherit
    virtual T* next() = 0;
    virtual ~IIterator(){};

    // and then add all those adapter functions and classes we get for free
    auto nth(int n);
    auto filter(std::function(bool(const T&)> pred);
  
    template <typename T2>
    auto map(std::function(T2(const T&)> fun);
  ...
}
```

TODO: write more


# Using ri

TODO: writeme

# The cost of using ri

A comparison of performance Rust Iterator, C++ stl iterators, range-v3 and ri on 10 million numbers. How much does adding another adapter cost?

TODO: writeme
