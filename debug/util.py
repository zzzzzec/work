import random
import csv
UPDATE_TYPE_ADD_NODE = 1
UPDATE_TYPE_ADD_EDGE = 2
UPDATE_TYPE_DELETE_NODE = 3
UPDATE_TYPE_DELETE_EDGE = 4
DATASETADDR = "./Dataset/sx-mathoverflow/sx-mathoverflow0.txt"
QUERYFILEADDR = "../QueryFile/testQuery.txt"
UPDATEFILEADDR = "../updateFile/testUpdate.txt"
TIMEINTERVAL = 16
#转为python
def GenerateRandomAddEdgeUpdate(num, updateFileAddress):
    updateRecords = []
    beginList = []
    endList = []
    for i in range(num):
        begin = random.randint(0, 19)
        end = random.randint(0, 19)
        beginList.append(begin)
        endList.append(end)
        timeStamp = random.randint(0, TIMEINTERVAL - 1)
        updateRecords.append([UPDATE_TYPE_ADD_EDGE,begin,end,timeStamp])
    updateFile = open(updateFileAddress, 'w')
    for it in updateRecords:
        updateFile.write(str(it[0]) + ' ' + str(it[1]) + ' ' + str(it[2]) + ' ' + str(it[3]) + '\n')

    #追加写入
    #queryFile = open(QUERYFILEADDR, 'a')
    #for i in range(beginList.__len__()):
    #    timeB = random.randint(0, TIMEINTERVAL - 1)
    #    timeE = random.randint(timeB, TIMEINTERVAL - 1)
    #    type = random.randint(0, 1) 
    #    queryFile.write(str(beginList[i]) + ' ' + str(endList[i]) + ' ' + str(timeB) + ' ' + str(timeE) + ' ' + str(type) + '\n')    
    
def GenerateRandomDeleteEdgeUpdate(num, updateFileAddress):
    updateRecords = []
    for i in range(num):
        begin = random.randint(0, 12)
        end = random.randint(0, 12)
        timeStamp = random.randint(0, TIMEINTERVAL - 1)
        updateRecords.append([UPDATE_TYPE_DELETE_EDGE,begin,end,timeStamp])
    updateFile = open(updateFileAddress, 'w')
    for it in updateRecords:
        updateFile.write(str(it[0]) + ' ' + str(it[1]) + ' ' + str(it[2]) + ' ' + str(it[3]) + '\n')

def GenerateRandomQuery(num, datasetAddr, queryFileAddr):
    f = open(datasetAddr, 'r')
    data = []
    allSrc = []
    allDst = []
    for line in f:
        src = int(line.strip('\n').split(' ')[0])
        dst = int(line.strip('\n').split(' ')[1])
        time = int(line.strip('\n').split(' ')[2])
        allSrc.append(src)
        allDst.append(dst)
        data.append([src, dst, time])
    f.close()
    queryRecords = []

    for i in range(num):
        begin = allSrc[random.randint(0, len(allSrc) - 1)]
        end = allDst[random.randint(0, len(allDst) - 1)]
        timeB = random.randint(0, TIMEINTERVAL - 1)
        timeE = random.randint(timeB, TIMEINTERVAL - 1)
        type = random.randint(0, 1) 
        queryRecords.append([begin,end,timeB,timeE,type])
    queryFile = open(queryFileAddr, 'w')
    for it in queryRecords:
        queryFile.write(str(it[0]) + ' ' + str(it[1]) + ' ' + str(it[2]) + ' ' + str(it[3]) + ' ' + str(it[4]) + '\n')

def GenerateRandomAddEdgeUpdateFromFile(num, originGraphDir, fileHead):
    datas = []
    for i in range(TIMEINTERVAL):
        data = []
        filePath = originGraphDir +"\\" + fileHead + str(i) + '.txt'
        with open(filePath, 'r') as file:
            for line in file:
                src, dst, time = line.strip().split(' ')
                data.append(src)
                data.append(dst)
        datas.append(data)
    updateRecords = []
    for i in range(num):
        time = random.randint(0, TIMEINTERVAL - 1)
        begin = datas[time][random.randint(0, len(datas[time]) - 1)]
        end = datas[time][random.randint(0, len(datas[time]) - 1)]
        updateRecords.append([UPDATE_TYPE_ADD_EDGE,begin,end,time])
    return updateRecords

def GenerateRandomDelEdgeUpdateFromFile(num, originGraphDir, fileHead):
    datas = []
    for i in range(TIMEINTERVAL):
        data = []
        filePath = originGraphDir +"\\" + fileHead + str(i) + '.txt'
        with open(filePath, 'r') as file:
            for line in file:
                src, dst, time = line.strip().split(' ')
                data.append([src,dst])
        datas.append(data)
    updateRecords = []
    # 随机的从边里面选就好了
    for i in range(num):
        time = random.randint(0, TIMEINTERVAL - 1)
        index = random.randint(0, len(datas[time]) - 1)
        src = datas[time][index][0]
        dst = datas[time][index][1]
        while ([UPDATE_TYPE_DELETE_EDGE,src,dst,time] in updateRecords ) or (src == dst):
            time = random.randint(0, TIMEINTERVAL - 1)
            index = random.randint(0, len(datas[time]) - 1)
            src = datas[time][index][0]
            dst = datas[time][index][1]
        updateRecords.append([UPDATE_TYPE_DELETE_EDGE,src,dst,time])
        
    return updateRecords


if __name__ == "__main__":
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\wiki-talk-temporal\\update.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-stackoverflow\\update.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-askubuntu\\update.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\CollegeMsg\\update.txt"

    #updateRecords = GenerateRandomAddEdgeUpdateFromFile(500, 'D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-mathoverflow', 'sx-mathoverflow')

    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-mathoverflow\\update-del.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-stackoverflow\\update-del.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\sx-askubuntu\\update-del.txt"
    #updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\wiki-talk-temporal\\update-del.txt"
    updateFileAddress = "D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\CollegeMsg\\update-del.txt"
    updateRecords = GenerateRandomDelEdgeUpdateFromFile(500, 'D:\\desktop\\work\\code\\IG_NoOp_5\\Dataset\\CollegeMsg', 'CollegeMsg')
    with open(updateFileAddress,'w') as f:
        for r in updateRecords:
            f.write(str(r[0]) + ' ' + str(r[1]) + ' ' + str(r[2]) + ' ' + str(r[3]) + '\n')
