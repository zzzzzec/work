#data = '''timeStamp: 0 nodeNum: 743 edgeNum: 3447 SCCNum: 386 SCCEdgeNum: 495
#timeStamp: 1 nodeNum: 1718 edgeNum: 8077 SCCNum: 995 SCCEdgeNum: 1430
#timeStamp: 2 nodeNum: 2540 edgeNum: 10617 SCCNum: 1605 SCCEdgeNum: 2468
#timeStamp: 3 nodeNum: 7921 edgeNum: 34622 SCCNum: 5609 SCCEdgeNum: 10232
#timeStamp: 4 nodeNum: 6616 edgeNum: 26080 SCCNum: 4982 SCCEdgeNum: 9392
#timeStamp: 5 nodeNum: 6322 edgeNum: 20297 SCCNum: 5184 SCCEdgeNum: 9787
#timeStamp: 6 nodeNum: 6086 edgeNum: 17821 SCCNum: 5196 SCCEdgeNum: 10250
#timeStamp: 7 nodeNum: 6171 edgeNum: 16690 SCCNum: 5412 SCCEdgeNum: 10749'''

#data = '''timeStamp: 0 nodeNum: 1886 edgeNum: 16542 SCCNum: 723 SCCEdgeNum: 759
#timeStamp: 1 nodeNum: 2620 edgeNum: 22953 SCCNum: 1002 SCCEdgeNum: 1060
#timeStamp: 2 nodeNum: 3182 edgeNum: 23879 SCCNum: 1284 SCCEdgeNum: 1349
#timeStamp: 3 nodeNum: 3371 edgeNum: 21583 SCCNum: 1431 SCCEdgeNum: 1521
#timeStamp: 4 nodeNum: 3381 edgeNum: 17452 SCCNum: 1598 SCCEdgeNum: 1695
#timeStamp: 5 nodeNum: 3471 edgeNum: 16492 SCCNum: 1622 SCCEdgeNum: 1743
#timeStamp: 6 nodeNum: 3680 edgeNum: 16813 SCCNum: 1747 SCCEdgeNum: 1826
#timeStamp: 7 nodeNum: 3757 edgeNum: 16220 SCCNum: 1883 SCCEdgeNum: 1998
#timeStamp: 8 nodeNum: 4069 edgeNum: 17401 SCCNum: 2028 SCCEdgeNum: 2168
#timeStamp: 9 nodeNum: 3994 edgeNum: 15272 SCCNum: 2173 SCCEdgeNum: 2314
#timeStamp: 10 nodeNum: 4107 edgeNum: 14680 SCCNum: 2324 SCCEdgeNum: 2427
#timeStamp: 11 nodeNum: 4090 edgeNum: 13970 SCCNum: 2344 SCCEdgeNum: 2496
#timeStamp: 12 nodeNum: 3883 edgeNum: 13202 SCCNum: 2261 SCCEdgeNum: 2390
#timeStamp: 13 nodeNum: 3999 edgeNum: 13312 SCCNum: 2343 SCCEdgeNum: 2452
#timeStamp: 14 nodeNum: 4018 edgeNum: 12740 SCCNum: 2458 SCCEdgeNum: 2600
#timeStamp: 15 nodeNum: 4285 edgeNum: 13610 SCCNum: 2631 SCCEdgeNum: 2779'''

data = '''timeStamp: 0 nodeNum: 24 edgeNum: 17 SCCNum: 24 SCCEdgeNum: 17
timeStamp: 1 nodeNum: 319 edgeNum: 543 SCCNum: 262 SCCEdgeNum: 276
timeStamp: 2 nodeNum: 764 edgeNum: 1833 SCCNum: 592 SCCEdgeNum: 627
timeStamp: 3 nodeNum: 645 edgeNum: 2372 SCCNum: 436 SCCEdgeNum: 502
timeStamp: 4 nodeNum: 627 edgeNum: 2411 SCCNum: 503 SCCEdgeNum: 630
timeStamp: 5 nodeNum: 707 edgeNum: 2014 SCCNum: 610 SCCEdgeNum: 830
timeStamp: 6 nodeNum: 1028 edgeNum: 2604 SCCNum: 948 SCCEdgeNum: 1336
timeStamp: 7 nodeNum: 1303 edgeNum: 2477 SCCNum: 1253 SCCEdgeNum: 1762
timeStamp: 8 nodeNum: 1707 edgeNum: 3529 SCCNum: 1629 SCCEdgeNum: 2394
timeStamp: 9 nodeNum: 1538 edgeNum: 2377 SCCNum: 1516 SCCEdgeNum: 2156
timeStamp: 10 nodeNum: 1899 edgeNum: 2842 SCCNum: 1883 SCCEdgeNum: 2708
timeStamp: 11 nodeNum: 2437 edgeNum: 3353 SCCNum: 2424 SCCEdgeNum: 3249
timeStamp: 12 nodeNum: 2592 edgeNum: 3280 SCCNum: 2585 SCCEdgeNum: 3262
timeStamp: 13 nodeNum: 2224 edgeNum: 2815 SCCNum: 2221 SCCEdgeNum: 2801
timeStamp: 14 nodeNum: 1646 edgeNum: 2095 SCCNum: 1644 SCCEdgeNum: 2091
timeStamp: 15 nodeNum: 1566 edgeNum: 1967 SCCNum: 1562 SCCEdgeNum: 1955'''

sum = 0
for line in data.split('\n'):
    parts = line.split()
    timestamp = int(parts[1])
    node_num = int(parts[3])
    edge_num = int(parts[5])
    density = edge_num / (node_num * (node_num -1))
    sum += density
    print(f'Timestamp {timestamp}: density = {density}')
print("avg " + str(sum/8))