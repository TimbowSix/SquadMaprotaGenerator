import pandas as pd
import plotly.express as px
import scipy.optimize as opt
import numpy as np

def func(x, a, b):
    return x[1]*a*x[0]**b

url = "https://docs.google.com/spreadsheets/d/1ZHwL8aoE1x3cMArbmIGGhdruHK7uNo86fLa_oHB7RtY/export?format=xlsx&gid=1900290044"

df = pd.read_excel(url)
x = np.array(df["new neighbors"].to_list())
y = np.array(df["new neighbors"].to_list())
data = np.array(df["RAAS"].to_list())

side_x = x
side_y = y
X1, X2 = np.meshgrid(side_x, side_y)
size = X1.shape
x1_1d = X1.reshape((1, np.prod(size)))
x2_1d = X2.reshape((1, np.prod(size)))

xdata = np.vstack((x1_1d, x2_1d))
p0 = (3, 1, 0.5)

popt, pcov = opt.curve_fit(func, xdata, data)
print("original: {}\nfitted: {}".format(p0, popt))

fig = px.scatter_3d(df, x="new neighbors", y="p RAAS", z="RAAS", template="plotly_dark")
fig.show()