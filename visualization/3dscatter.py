import pandas as pd
import plotly.express as px

url = "https://docs.google.com/spreadsheets/d/1ZHwL8aoE1x3cMArbmIGGhdruHK7uNo86fLa_oHB7RtY/export?format=xlsx&gid=1900290044"

df = pd.read_excel(url)

fig = px.scatter_3d(df, x="new neighbors", y="p RAAS", z="RAAS", template="plotly_dark")
fig.show()