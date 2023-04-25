import graphviz
import json
import os

os.chdir("D:\desktop\work\code\IG_NoOp_5\debug")
MAXLIFESPAN = 4
originGraphJSONPath = "../resultOriginGraph"
IGJSONPath = "../Result_Full_IG"
def printLifeSpan(string):
    return 

def binary_string_to_indices(s: str) -> list:
    result = []
    n = len(s)
    for i, char in enumerate(s[::-1]):
        if char == '1':
            result.append(i)
    return result

def visualizeOriginGraph():
    for i in range(MAXLIFESPAN):
        filePath = originGraphJSONPath + "/originGraph" + str(i) + ".json"
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
            dot.render(filename='graph' + str(i) + '.gv', directory=originGraphJSONPath, view=False)

def visualizeIG():
    filePath = IGJSONPath + "/IG.json"
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
        dot.render(filename= 'IG.gv', directory=IGJSONPath, view=False)

if __name__ == "__main__" :
    #visualizeOriginGraph() 
    visualizeIG()