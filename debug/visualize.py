import graphviz
import json
import os

os.chdir("D:\desktop\work\code\IG_NoOp_5\debug")
JSONPATH = "../Result_Full_IG/IG.json"
MAXLIFESPAN = 4
def printLifeSpan(string):
    return 

if __name__ == "__main__" :
    with open(JSONPATH, 'r') as f:
        data = json.load(f)
        pass
        dot = graphviz.Digraph(comment='IindexGraph')
        for node in data:
            string = 'uuid = ' + str(node['uuid']) + '\nsouID = ' + str(node['souID']) + '\nLifeSpan = ' + node["souLifespan"][-MAXLIFESPAN:]
            dot.node(str(node['uuid']), string)
        
        for node in data:
            if node['arcs'] == None:
                continue
            for edge in node['arcs']:
                dot.edge(str(node['uuid']), str(edge['uuid']))
        dot.render('graph.gv')