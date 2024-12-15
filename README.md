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




## Sequential results 

```
Results for ./tspcc executions:

Command: ./tspcc ./data/dj1.tsp
Standard Output:
shortest [0: 0, 0]

Standard Error:

Elapsed Time: 0.002978 seconds
----------------------------------------

Command: ./tspcc ./data/dj2.tsp
Standard Output:
shortest [582: 0, 1, 0]

Standard Error:

Elapsed Time: 0.003160 seconds
----------------------------------------

Command: ./tspcc ./data/dj3.tsp
Standard Output:
shortest [1598: 0, 1, 2, 0]

Standard Error:

Elapsed Time: 0.002428 seconds
----------------------------------------

Command: ./tspcc ./data/dj4.tsp
Standard Output:
shortest [1602: 0, 1, 3, 2, 0]

Standard Error:

Elapsed Time: 0.002178 seconds
----------------------------------------

Command: ./tspcc ./data/dj5.tsp
Standard Output:
shortest [1719: 0, 1, 3, 4, 2, 0]

Standard Error:

Elapsed Time: 0.002815 seconds
----------------------------------------

Command: ./tspcc ./data/dj6.tsp
Standard Output:
shortest [1818: 0, 1, 5, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.002428 seconds
----------------------------------------

Command: ./tspcc ./data/dj7.tsp
Standard Output:
shortest [1883: 0, 1, 5, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.003010 seconds
----------------------------------------

Command: ./tspcc ./data/dj8.tsp
Standard Output:
shortest [2101: 0, 1, 5, 7, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.002851 seconds
----------------------------------------

Command: ./tspcc ./data/dj9.tsp
Standard Output:
shortest [2134: 0, 1, 5, 7, 8, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.004058 seconds
----------------------------------------

Command: ./tspcc ./data/dj10.tsp
Standard Output:
shortest [2577: 0, 1, 3, 2, 4, 6, 8, 7, 5, 9, 0]

Standard Error:

Elapsed Time: 0.010198 seconds
----------------------------------------

Command: ./tspcc ./data/dj11.tsp
Standard Output:
shortest [3014: 0, 1, 3, 2, 4, 10, 8, 7, 6, 5, 9, 0]

Standard Error:

Elapsed Time: 0.034190 seconds
----------------------------------------

Command: ./tspcc ./data/dj12.tsp
Standard Output:
shortest [3026: 0, 1, 3, 2, 4, 10, 11, 8, 7, 6, 5, 9, 0]

Standard Error:

Elapsed Time: 0.166892 seconds
----------------------------------------

Command: ./tspcc ./data/dj13.tsp
Standard Output:
shortest [3125: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 9, 0]

Standard Error:

Elapsed Time: 1.051346 seconds
----------------------------------------

Command: ./tspcc ./data/dj14.tsp
Standard Output:
shortest [3161: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 9, 0]

Standard Error:

Elapsed Time: 2.346247 seconds
----------------------------------------

Command: ./tspcc ./data/dj15.tsp
Standard Output:
shortest [3171: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 10.504623 seconds
----------------------------------------

Command: ./tspcc ./data/dj16.tsp
Standard Output:
shortest [3226: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 15, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 37.298841 seconds
----------------------------------------

Command: ./tspcc ./data/dj17.tsp
Standard Output:
shortest [3249: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 15, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 184.547035 seconds
----------------------------------------

Command: ./tspcc ./data/dj18.tsp
Standard Output:
shortest [3270: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 17, 15, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 1037.006030 seconds
----------------------------------------
```