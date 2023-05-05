import graphviz
import json
import os

os.chdir("D:\desktop\work\code\IG_NoOp_5\debug")
MAXLIFESPAN = 4
def printLifeSpan(string):
    return 

def binary_string_to_indices(s: str) -> list:
    result = []
    n = len(s)
    for i, char in enumerate(s[::-1]):
        if char == '1':
            result.append(i)
    return result

def visualizeOriginGraph(dir:str):
    for i in range(MAXLIFESPAN):
        filePath = dir + "/originGraph" + str(i) + ".json"
        with open(filePath, 'r') as f:
            data = json.load(f)
            dot = graphviz.Digraph(comment='OriginGraph' + str(i), engine= 'fdp')
            for node in data:
                string = 'souID = ' + str(node['souID']) + '\ntime' + str(node['time'])
                dot.node(str(node['souID']), string)

            for node in data:
                if node['arcs'] == None:
                    continue
                for edge in node['arcs']:
                    dot.edge(str(node['souID']), str(edge['tarID']))
            dot.render(filename='graph' + str(i) + '.gv', directory=dir, view=False)

def visualizeIG(directory:str, fileName:str):
    filePath = os.path.join(directory, fileName)
    with open(filePath, 'r') as f:
        data = json.load(f)
        pass
        dot = graphviz.Digraph(comment='IindexGraph', engine= 'fdp')
        for node in data:
            string = 'uuid = ' + str(node['uuid']) + '\nsouID = ' + str(node['souID']) + '\nLifeSpan = ' + str(binary_string_to_indices(node['souLifespan']))
            dot.node(str(node['uuid']), string)
        for node in data:
            if node['arcs'] == None:
                continue
            for edge in node['arcs']:
                dot.edge(str(node['uuid']), str(edge['uuid']))
        dot.render(filename= fileName.split(".")[0] + ".gv" , directory=directory, view=False)
    
def visualizeSCC(dir:str):
    for i in range(MAXLIFESPAN):
        filePath = dir + "/sccGraph" + str(i) + ".json"
        with open(filePath, 'r') as f:
            data = json.load(f)
            dot = graphviz.Digraph(comment='sccGraph' + str(i), engine= 'fdp')
            for node in data:
                string = 'ID = ' + str(node['ID']) + '\ntime' + str(node['time'])
                oNode = []
                for originNode in node['originNodeSet']:
                    oNode.append(originNode['originID'])
                string += "\n" + str(oNode)
                dot.node(str(node['ID']), string)

            for node in data:
                if node['arcs'] == None:
                    continue
                for edge in node['arcs']:
                    dot.edge(str(node['ID']), str(edge['dstID']))
            dot.render(filename='graph' + str(i) + '.gv', directory=dir, view=False)

def visualizeAll(dir:str):
    visualizeOriginGraph(dir + "/originGraph")
    visualizeSCC(dir + "/SCCGraph")
    visualizeIG(dir + "/IG", "IG.json")

if __name__ == "__main__" :
    #visualizeIG("../", "IG.json")
    visualizeAll("../gt")
    visualizeAll("../test")
    print("done")