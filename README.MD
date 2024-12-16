# PCM PROJECT


## 11/12/2024
Issues:
1. segfaults from time to time
2. slower than sequential
3. Counter values are probably wrong (test atomic increment?)


## 13/12/2024
Issues:
1. Fucking sanitizer fixes some issues ???!!!


## CURRENT ISSUES / OPTIS
- Not free pool
- Address sanitizer 
- No altruism, not helping others to finish their work in the queue





## Sequential results 

```
Results for ./tspcc executions:

Command: ./tspcc ./data/dj1.tsp
Standard Output:
shortest [0: 0, 0]

Elapsed Time: 0.002978 seconds
----------------------------------------

Command: ./tspcc ./data/dj2.tsp
Standard Output:
shortest [582: 0, 1, 0]

Elapsed Time: 0.003160 seconds
----------------------------------------

Command: ./tspcc ./data/dj3.tsp
Standard Output:
shortest [1598: 0, 1, 2, 0]

Elapsed Time: 0.002428 seconds
----------------------------------------

Command: ./tspcc ./data/dj4.tsp
Standard Output:
shortest [1602: 0, 1, 3, 2, 0]

Elapsed Time: 0.002178 seconds
----------------------------------------

Command: ./tspcc ./data/dj5.tsp
Standard Output:
shortest [1719: 0, 1, 3, 4, 2, 0]

Elapsed Time: 0.002815 seconds
----------------------------------------

Command: ./tspcc ./data/dj6.tsp
Standard Output:
shortest [1818: 0, 1, 5, 4, 2, 3, 0]

Elapsed Time: 0.002428 seconds
----------------------------------------

Command: ./tspcc ./data/dj7.tsp
Standard Output:
shortest [1883: 0, 1, 5, 6, 4, 2, 3, 0]

Elapsed Time: 0.003010 seconds
----------------------------------------

Command: ./tspcc ./data/dj8.tsp
Standard Output:
shortest [2101: 0, 1, 5, 7, 6, 4, 2, 3, 0]

Elapsed Time: 0.002851 seconds
----------------------------------------

Command: ./tspcc ./data/dj9.tsp
Standard Output:
shortest [2134: 0, 1, 5, 7, 8, 6, 4, 2, 3, 0]

Elapsed Time: 0.004058 seconds
----------------------------------------

Command: ./tspcc ./data/dj10.tsp
Standard Output:
shortest [2577: 0, 1, 3, 2, 4, 6, 8, 7, 5, 9, 0]

Elapsed Time: 0.010198 seconds
----------------------------------------

Command: ./tspcc ./data/dj11.tsp
Standard Output:
shortest [3014: 0, 1, 3, 2, 4, 10, 8, 7, 6, 5, 9, 0]

Elapsed Time: 0.034190 seconds
----------------------------------------

Command: ./tspcc ./data/dj12.tsp
Standard Output:
shortest [3026: 0, 1, 3, 2, 4, 10, 11, 8, 7, 6, 5, 9, 0]

Elapsed Time: 0.166892 seconds
----------------------------------------

Command: ./tspcc ./data/dj13.tsp
Standard Output:
shortest [3125: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 9, 0]

Elapsed Time: 1.051346 seconds
----------------------------------------

Command: ./tspcc ./data/dj14.tsp
Standard Output:
shortest [3161: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 9, 0]

Elapsed Time: 2.346247 seconds
----------------------------------------

Command: ./tspcc ./data/dj15.tsp
Standard Output:
shortest [3171: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 13, 9, 0]

Elapsed Time: 10.504623 seconds
----------------------------------------

Command: ./tspcc ./data/dj16.tsp
Standard Output:
shortest [3226: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 15, 12, 14, 13, 9, 0]

Elapsed Time: 37.298841 seconds
----------------------------------------

Command: ./tspcc ./data/dj17.tsp
Standard Output:
shortest [3249: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 15, 12, 14, 13, 9, 0]

Elapsed Time: 184.547035 seconds
----------------------------------------

Command: ./tspcc ./data/dj18.tsp
Standard Output:
shortest [3270: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 17, 15, 12, 14, 13, 9, 0]

Elapsed Time: 1037.006030 seconds
----------------------------------------
```




## Threaded measurements (6 threads, split @ 12)

```
Results for ./tspcc executions:

Command: ./build/main ./data/dj1.tsp
Standard Output:

Elapsed Time: 0.007575 seconds
----------------------------------------

Command: ./build/main ./data/dj2.tsp
Standard Output:
Starting 6 threads
0 - STARTED WORKER 01 - STARTED WORKER 1

3 - STARTED WORKER 3
2 - STARTED WORKER 2
5 - STARTED WORKER 5
4 - STARTED WORKER 4
shortest [582: 0, 1, 0]
bound (per level): 0 0
bound equivalent (per level):  0 0
bound equivalent (total): 0
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.017688 seconds
----------------------------------------

Command: ./build/main ./data/dj3.tsp
Standard Output:
Starting 6 threads
1 - STARTED WORKER 1
3 - STARTED WORKER 3
0 - STARTED WORKER 0
2 - STARTED WORKER 2
4 - STARTED WORKER 4
5 - STARTED WORKER 5
shortest [1598: 0, 1, 2, 0]
bound (per level): 0 0 0
bound equivalent (per level):  0 0 0
bound equivalent (total): 0
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.011191 seconds
----------------------------------------

Command: ./build/main ./data/dj4.tsp
Standard Output:
Starting 6 threads
5 - STARTED WORKER 5
4 - STARTED WORKER 4
0 - STARTED WORKER 0
2 - STARTED WORKER 2
3 - STARTED WORKER 3
1 - STARTED WORKER 1
shortest [1602: 0, 1, 3, 2, 0]
bound (per level): 0 0 0 0
bound equivalent (per level):  0 0 0 0
bound equivalent (total): 0
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.010179 seconds
----------------------------------------

Command: ./build/main ./data/dj5.tsp
Standard Output:
Starting 6 threads
5 - STARTED WORKER 5
1 - STARTED WORKER 1
3 - STARTED WORKER 3
4 - STARTED WORKER 4
2 - STARTED WORKER 2
0 - STARTED WORKER 0
shortest [1719: 0, 1, 3, 4, 2, 0]
bound (per level): 0 0 0 0 3
bound equivalent (per level):  0 0 0 0 3
bound equivalent (total): 3
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.010334 seconds
----------------------------------------

Command: ./build/main ./data/dj6.tsp
Standard Output:
Starting 6 threads
3 - STARTED WORKER 3
2 - STARTED WORKER 2
1 - STARTED WORKER 1
0 - STARTED WORKER 0
5 - STARTED WORKER 5
4 - STARTED WORKER 4
shortest [1818: 0, 1, 5, 4, 2, 3, 0]
bound (per level): 0 0 0 0 6 28
bound equivalent (per level):  0 0 0 0 12 28
bound equivalent (total): 40
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.011223 seconds
----------------------------------------

Command: ./build/main ./data/dj7.tsp
Standard Output:
Starting 6 threads
3 - STARTED WORKER 32 - STARTED WORKER 2

0 - STARTED WORKER 0
4 - STARTED WORKER 4
51 - STARTED WORKER 1
 - STARTED WORKER 5
shortest [1883: 0, 1, 5, 6, 4, 2, 3, 0]
bound (per level): 0 0 0 0 9 66 187
bound equivalent (per level):  0 0 0 0 54 132 187
bound equivalent (total): 373
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.011841 seconds
----------------------------------------

Command: ./build/main ./data/dj8.tsp
Standard Output:
Starting 6 threads
42 -  - STARTED WORKER 2STARTED WORKER 4

1 - STARTED WORKER 1
5 - STARTED WORKER 5
3 - STARTED WORKER 3
0 - STARTED WORKER 0
shortest [2101: 0, 1, 5, 7, 6, 4, 2, 3, 0]
bound (per level): 0 0 0 0 5 99 579 1251
bound equivalent (per level):  0 0 0 0 120 594 1158 1251
bound equivalent (total): 3123
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.015291 seconds
----------------------------------------

Command: ./build/main ./data/dj9.tsp
Standard Output:
Starting 6 threads
5 - STARTED WORKER 51 - STARTED WORKER 10 - STARTED WORKER 0

4 - STARTED WORKER 4

2 - STARTED WORKER 2
3 - STARTED WORKER 3
shortest [2134: 0, 1, 5, 7, 8, 6, 4, 2, 3, 0]
bound (per level): 0 0 0 0 12 191 1386 4476 8175
bound equivalent (per level):  0 0 0 0 1440 4584 8316 8952 8175
bound equivalent (total): 31467
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.020029 seconds
----------------------------------------

Command: ./build/main ./data/dj10.tsp
Standard Output:
Starting 6 threads
4 - STARTED WORKER 4
1 - STARTED WORKER 1
2 - STARTED WORKER 2
3 - STARTED WORKER 3
0 - STARTED WORKER 0
5 - STARTED WORKER 5
shortest [2577: 0, 1, 3, 2, 4, 6, 8, 7, 5, 9, 0]
bound (per level): 0 0 0 0 23 413 2952 13844 36398 46727
bound equivalent (per level):  0 0 0 0 16560 49560 70848 83064 72796 46727
bound equivalent (total): 339555
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.030070 seconds
----------------------------------------

Command: ./build/main ./data/dj11.tsp
Standard Output:
Starting 6 threads
4 - STARTED WORKER 4
5 - STARTED WORKER 5
0 - STARTED WORKER 0
1 - STARTED WORKER 1
2 - STARTED WORKER 2
3 - STARTED WORKER 3
shortest [3014: 0, 1, 3, 2, 4, 10, 8, 7, 6, 5, 9, 0]
bound (per level): 0 0 0 0 9 323 5139 34767 135279 310668 341345
bound equivalent (per level):  0 0 0 0 45360 232560 616680 834408 811674 621336 341345
bound equivalent (total): 3503363
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.228450 seconds
----------------------------------------

Command: ./build/main ./data/dj12.tsp
Standard Output:
Starting 6 threads
1 - STARTED WORKER 13
 - STARTED WORKER 3
0 - STARTED WORKER 0
2 - STARTED WORKER 2
4 - STARTED WORKER 4
5 - STARTED WORKER 5
shortest [3026: 0, 1, 3, 2, 4, 10, 11, 8, 7, 6, 5, 9, 0]
bound (per level): 0 0 0 0 22 632 9467 73714 355114 1070362 1780205 1329638
bound equivalent (per level):  0 0 0 0 887040 3185280 6816240 8845680 8522736 6422172 3560410 1329638
bound equivalent (total): 39569196
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 0.722963 seconds
----------------------------------------

Command: ./build/main ./data/dj13.tsp
Standard Output:
Starting 6 threads
1 - STARTED WORKER 1
20 - STARTED WORKER 2 - 
STARTED WORKER 0
3 - STARTED WORKER 53 - 
STARTED WORKER 5
4 - STARTED WORKER 4
shortest [3125: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 9, 0]
bound (per level): 0 0 0 0 17 639 13129 137364 864959 3623003 9357444 12866186 7770481
bound equivalent (per level):  0 0 0 0 6168960 25764480 66170160 98902080 103795080 86952072 56144664 25732372 7770481
bound equivalent (total): 477400349
check: total == verified + total bound equivalent

Standard Error:

Elapsed Time: 5.523872 seconds
----------------------------------------

Command: ./build/main ./data/dj14.tsp
Standard Output:
Starting 6 threads
0 - STARTED WORKER 0
2 - STARTED WORKER 2
4 - STARTED WORKER 4
5 - STARTED WORKER 5
3 - STARTED WORKER 3
1 - STARTED WORKER 1
shortest [3161: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 9, 0]
bound (per level): 0 0 0 0 23 1330 28504 296450 1850314 7708097 20891838 32937396 26001803 8497708
bound equivalent (per level):  0 0 0 0 83462400 482630400 1149281280 1494108000 1332226080 924971640 501404112 197624376 52003606 8497708
bound equivalent (total): 1931242306
check: total != verified + total bound equivalent

Standard Error:

Elapsed Time: 4.044034 seconds
----------------------------------------

Command: ./build/main ./data/dj15.tsp
Standard Output:
Starting 6 threads
2 - STARTED WORKER 32 - STARTED WORKER 30 - STARTED WORKER 0


4 - STARTED WORKER 4
1 - STARTED WORKER 1
5 - STARTED WORKER 5
shortest [3171: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 13, 9, 0]
bound (per level): 0 0 0 0 22 1566 41783 519741 3764760 18622574 63742466 137082895 161584878 92522648 21596048
bound equivalent (per level):  0 0 0 0 878169600 1387733504 -2017654144 -518879360 1794521216 523351392 -940838672 -1004977816 969509268 185045296 21596048
bound equivalent (total): 1277576332
check: total != verified + total bound equivalent

Standard Error:

Elapsed Time: 14.783706 seconds
----------------------------------------

Command: ./build/main ./data/dj16.tsp
Standard Output:
Starting 6 threads
5 - STARTED WORKER 5
3 - STARTED WORKER 3
4 - STARTED WORKER 4
2 - STARTED WORKER 2
1 - STARTED WORKER 1
0 - STARTED WORKER 0
shortest [3226: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 15, 12, 14, 13, 9, 0]
bound (per level): 0 0 0 0 27 2142 60630 830647 6926157 40665540 171220817 474077670 774388855 691068704 311091539 58397976
bound equivalent (per level):  0 0 0 0 48141312 -397560320 970811904 777472640 89776000 -1204108608 -1275063344 1054745552 1405463336 -148555072 622183078 58397976
bound equivalent (total): 2001704454
check: total != verified + total bound equivalent

Standard Error:

Elapsed Time: 66.158515 seconds
----------------------------------------

Command: ./build/main ./data/dj17.tsp
Standard Output:
Starting 6 threads
5 - STARTED WORKER 51 - STARTED WORKER 1

0 - STARTED WORKER 0
2 - STARTED WORKER 2
3 - STARTED WORKER 3
4 - STARTED WORKER 4
shortest [3249: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 15, 12, 14, 13, 9, 0]
bound (per level): 0 0 0 0 6 3360 87604 1282674 11930355 81185295 407312464 1387701828 2970687785 3862130064 2983316603 1273914801 236506448
bound equivalent (per level):  0 0 0 0 -1292580864 -1167360000 767968256 -1177137664 -39811968 626014848 -139548928 -1582063808 248632 -1798158976 720030434 -1747137694 236506448
bound equivalent (total): 1996903308
check: total != verified + total bound equivalent

Standard Error:

Elapsed Time: 338.223815 seconds
----------------------------------------

```








## Mesures de temps sequentielles sur le serveur

```
Results for ./tspcc executions:

Command: ./tspcc ./data/dj1.tsp
Standard Output:
shortest [0: 0, 0]

Standard Error:

Elapsed Time: 0.009876 seconds
----------------------------------------

Command: ./tspcc ./data/dj2.tsp
Standard Output:
shortest [582: 0, 1, 0]

Standard Error:

Elapsed Time: 0.009508 seconds
----------------------------------------

Command: ./tspcc ./data/dj3.tsp
Standard Output:
shortest [1598: 0, 1, 2, 0]

Standard Error:

Elapsed Time: 0.010283 seconds
----------------------------------------

Command: ./tspcc ./data/dj4.tsp
Standard Output:
shortest [1602: 0, 1, 3, 2, 0]

Standard Error:

Elapsed Time: 0.009648 seconds
----------------------------------------

Command: ./tspcc ./data/dj5.tsp
Standard Output:
shortest [1719: 0, 1, 3, 4, 2, 0]

Standard Error:

Elapsed Time: 0.009653 seconds
----------------------------------------

Command: ./tspcc ./data/dj6.tsp
Standard Output:
shortest [1818: 0, 1, 5, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.010183 seconds
----------------------------------------

Command: ./tspcc ./data/dj7.tsp
Standard Output:
shortest [1883: 0, 1, 5, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.010587 seconds
----------------------------------------

Command: ./tspcc ./data/dj8.tsp
Standard Output:
shortest [2101: 0, 1, 5, 7, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.011348 seconds
----------------------------------------

Command: ./tspcc ./data/dj9.tsp
Standard Output:
shortest [2134: 0, 1, 5, 7, 8, 6, 4, 2, 3, 0]

Standard Error:

Elapsed Time: 0.016317 seconds
----------------------------------------

Command: ./tspcc ./data/dj10.tsp
Standard Output:
shortest [2577: 0, 1, 3, 2, 4, 6, 8, 7, 5, 9, 0]

Standard Error:

Elapsed Time: 0.025438 seconds
----------------------------------------

Command: ./tspcc ./data/dj11.tsp
Standard Output:
shortest [3014: 0, 1, 3, 2, 4, 10, 8, 7, 6, 5, 9, 0]

Standard Error:

Elapsed Time: 0.140659 seconds
----------------------------------------

Command: ./tspcc ./data/dj12.tsp
Standard Output:
shortest [3026: 0, 1, 3, 2, 4, 10, 11, 8, 7, 6, 5, 9, 0]

Standard Error:

Elapsed Time: 0.605279 seconds
----------------------------------------

Command: ./tspcc ./data/dj13.tsp
Standard Output:
shortest [3125: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 9, 0]

Standard Error:

Elapsed Time: 4.111797 seconds
----------------------------------------

Command: ./tspcc ./data/dj14.tsp
Standard Output:
shortest [3161: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 9, 0]

Standard Error:

Elapsed Time: 8.862900 seconds
----------------------------------------

Command: ./tspcc ./data/dj15.tsp
Standard Output:
shortest [3171: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 47.677794 seconds
----------------------------------------

Command: ./tspcc ./data/dj16.tsp
Standard Output:
shortest [3226: 0, 1, 3, 2, 4, 5, 6, 7, 8, 10, 11, 15, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 234.202183 seconds
----------------------------------------

Command: ./tspcc ./data/dj17.tsp
Standard Output:
shortest [3249: 0, 1, 3, 2, 4, 5, 6, 7, 8, 11, 10, 16, 15, 12, 14, 13, 9, 0]

Standard Error:

Elapsed Time: 1169.255122 seconds
----------------------------------------

```