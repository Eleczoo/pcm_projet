# PCM PROJECT


## 11/12/2024
Issues:
1. segfaults from time to time
2. slower than sequential
3. Counter values are probably wrong (test atomic increment?)


## 13/12/2024
Issues:
1. Fucking sanitizer fixes some issues ???!!!
2. Error below pops up when adding more pushes into queues


```bash
==238182==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000010 (pc 0x779352a923d8 bp 0x77934d5ff9c0 sp 0x77934d5ff9c0 T3)
==238182==The signal is caused by a READ memory access.
AddressSanitizer==238182==Hint: address points to the zero page.
:DEADLYSIGNAL
FREE NODE : 0x50300002a160
AddressSanitizer:DEADLYSIGNAL
AddressSanitizer:DEADLYSIGNAL
    #0 0x779352a923d8 in atomic_load_n /usr/src/debug/gcc/gcc/libatomic/config/x86/host-config.h:129
    #1 0x779352a923d8 in libat_load_16_i1 /usr/src/debug/gcc/gcc/libatomic/load_n.c:39
    #2 0x60f9284a1575 in atomic_stamped<Node>::get(unsigned long&) src/atomic.hpp:67
    #3 0x60f92849f5b4 in LockFreeQueue::__enqueue_node(atomic_stamped<Node>*, Node*) src/fifo.hpp:359
    #4 0x60f92849eb66 in LockFreeQueue::enqueue(Path*) src/fifo.hpp:137
    #5 0x60f92849fc5c in worker_routine(int) src/test_fifo.c:40
    #6 0x60f9284a2969 in void std::__invoke_impl<void, void (*)(int), int>(std::__invoke_other, void (*&&)(int), int&&) /usr/include/c++/14.2.1/bits/invoke.h:61
    #7 0x60f9284a2886 in std::__invoke_result<void (*)(int), int>::type std::__invoke<void (*)(int), int>(void (*&&)(int), int&&) /usr/include/c++/14.2.1/bits/invoke.h:96
    #8 0x60f9284a27f6 in void std::thread::_Invoker<std::tuple<void (*)(int), int> >::_M_invoke<0ul, 1ul>(std::_Index_tuple<0ul, 1ul>) /usr/include/c++/14.2.1/bits/std_thread.h:301
    #9 0x60f9284a27af in std::thread::_Invoker<std::tuple<void (*)(int), int> >::operator()() /usr/include/c++/14.2.1/bits/std_thread.h:308
    #10 0x60f9284a2793 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (*)(int), int> > >::_M_run() /usr/include/c++/14.2.1/bits/std_thread.h:253
    #11 0x779351ee1c33 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #12 0x77935225d109 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #13 0x779351ca339c  (/usr/lib/libc.so.6+0x9439c) (BuildId: 98b3d8e0b8c534c769cb871c438b4f8f3a8e4bf3)
    #14 0x779351d2849b  (/usr/lib/libc.so.6+0x11949b) (BuildId: 98b3d8e0b8c534c769cb871c438b4f8f3a8e4bf3)

```