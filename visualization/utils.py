import plotly.graph_objects as go
import plotly.subplots as sp
import re, math

def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ] 
    return sorted(data, key=alphanum_key)

def make_subplots(*plots):
    all_traces = []

    for plot in plots:
        traces = []
        for trace in range(len(plot["data"])):
            traces.append(plot["data"][trace])
        all_traces.append(traces)

    this_figure = sp.make_subplots(rows=1, cols=len(plots)) 

    for ind, traces in enumerate(all_traces):
        for trace in traces:
            this_figure.append_trace(trace, row=1, col=ind+1)
    
    final_graph = go.Figure(this_figure)

    return final_graph

def num_zeros(decimal):
    return -math.floor(math.log10(decimal)) - 1

def round_x_decimal(num):
    return round(num+5*10**-(num_zeros(num)+2), num_zeros(num)+1)  

def normalize(arr):
    s = sum(arr)
    if s == 0:
        t = 1/len(arr)
        return [t for i in arr]
    else:
        return [i/s for i in arr]

def parseLayer(layer: str):
    layer = layer.split("_")
    ver = layer[-1].lower()
    mode = layer[-2]
    map = layer[0]
    return map, mode, ver