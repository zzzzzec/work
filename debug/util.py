import random
UPDATE_TYPE_ADD_NODE = 1
UPDATE_TYPE_ADD_EDGE = 2
UPDATE_TYPE_DELETE_NODE = 3
UPDATE_TYPE_DELETE_EDGE = 4
DATASETADDR = "./Dataset/sx-mathoverflow/sx-mathoverflow0.txt"
QUERYFILEADDR = "../QueryFile/testQuery.txt"
UPDATEFILEADDR = "../updateFile/testUpdate.txt"
TIMEINTERVAL = 4
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

if __name__ == "__main__" :
    updateFileAddress = "./updateFile/testUpdate.txt"
    GenerateRandomAddEdgeUpdate(10, updateFileAddress) 
    #GenerateRandomDeleteEdgeUpdate(50, updateFileAddress)
    #GenerateRandomQuery(100, DATASETADDR, "./QueryFile/testQuery.txt")