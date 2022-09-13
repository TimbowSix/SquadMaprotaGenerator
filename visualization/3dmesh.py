import plotly.graph_objects as go
import pandas as pd

url = "https://docs.google.com/spreadsheets/d/1ZHwL8aoE1x3cMArbmIGGhdruHK7uNo86fLa_oHB7RtY/export?format=xlsx&gid=1900290044"

df = pd.read_excel(url)
x = df["new neighbors"]
y = df["p RAAS"]
z = 25*y*x**1.8

fig = go.Figure(data=[go.Mesh3d(x=x, y=y, z=z, opacity=0.50)])
fig.show()