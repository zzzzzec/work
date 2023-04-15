import random
#转为python
def GenerateRandomAddEdgeUpdate(num, updateFileAddress):
    updateRecords = []
    for i in range(num):
        begin = random.randint(0, 19)
        end = random.randint(0, 19)
        timeStamp = random.randint(0, 3)
        updateRecords.append([3,begin,end,timeStamp])
    updateFile = open(updateFileAddress, 'w')
    for it in updateRecords:
        updateFile.write(str(it[0]) + ' ' + str(it[1]) + ' ' + str(it[2]) + ' ' + str(it[3]) + '\n')
    
if __name__ == "__main__" :
    updateFileAddress = "./updateFile/testUpdate.txt"
    GenerateRandomAddEdgeUpdate(20, updateFileAddress)