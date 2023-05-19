type2 = []
type1 = []
type3 = []
type4 = []
#LOG = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-mathoverflow\\res.txt"
#LOG = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\wiki-talk-temporal\\res.txt"
#LOG = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-stackoverflow\\res.txt"
#LOG = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-askubuntu\\res.txt"
LOG = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\CollegeMsg\\res.txt"
with open(LOG, 'r') as f:
    for line in f:
        if 'finish' in line:
            time = float(line.split('(')[1].split('ms')[0])
            next(f)
            type_line = next(f)
            if 'type = 1' in type_line:
                type1.append(time)
            elif 'type = 2' in type_line:
                type2.append(time)
            elif 'type = 3' in type_line:
                type3.append(time)
            elif 'type = 4' in type_line:
                type4.append(time)

print("type 1 = " + str(len(type1)))
print("type 2 = " + str(len(type2)))
print("type 3 = " + str(len(type3)))
print("type 4 = " + str(len(type4)))
print("all = " + str(len(type1) + len(type2) + len(type3) + len(type4)))
print("type1-avg = " + str(sum(type1) / len(type1)))
print("type2-avg = " + str(sum(type2) / len(type2)))
print("type3-avg = " + str(sum(type3) / len(type3)))
print("type4-avg = " + str(sum(type4) / len(type4)))
total = sum(type1) + sum(type2) + sum(type3) + sum(type4)
print("total = " + str(total))
print("avg = " + str(total/ 200))