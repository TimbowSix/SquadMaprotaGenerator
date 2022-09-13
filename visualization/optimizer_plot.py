import plotly.express as px
import pandas as pd
import json
from PIL import Image
import tempfile
import math
import re
import imageio.v2 as imageio
import os


#-# Config #-#

run_info_path = "run_info_9732be4e-b882-480a-b0e1-3993a6c22d8a_AAS.json"
history_path = "optimizer_maps_history_AAS_9732be4e-b882-480a-b0e1-3993a6c22d8a.json"
deviation = True
plot_template = "plotly_dark"
save_single_pics=False
gif_duration = 0.5
value_check = True

#-#

sr = run_info_path.replace(".json", "").split("_")
sh = history_path.replace(".json", "").split("_")

mode_r = sr[-1]
mode_h = sh[-2]

id_r = sr[-2]
id_h = sh[-1]

if value_check:
    if not (mode_r == mode_h):
        raise ValueError("history and run_info modes are different")
    if not (id_r == id_h):
        raise ValueError("history and run_info run IDs are different")


def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ] 
    return sorted(data, key=alphanum_key)

def num_zeros(decimal):
    return -math.floor(math.log10(decimal)) - 1

def round_x_decimal(num):
    return round(num+5*10**-(num_zeros(num)+2), num_zeros(num)+1)  

with open(run_info_path, "r") as fr, open(history_path, "r") as fh:
    run_info = json.load(fr)
    history = json.load(fh)

# Gif
dfs = []
max_ = 0
for ind, maps in enumerate(history):
    count = [{"map": str(map_), "count": count} for map_ , count in maps[mode_h].items()]
    df = pd.DataFrame(count)
    max_n = round(df["count"].max()+500, -3)
    max_ = max_n if max_n > max_ else max_
    df.sort_values(by="count", inplace=True, ascending=False)
    dfs.append(df)

with tempfile.TemporaryDirectory() as t_dir:
    for ind, df in enumerate(dfs):
        title = f"{ind} {mode_h}<br><sup>{id_h}</sup>"
        fig = px.bar(df, x="map", y="count", template="plotly_dark", title=title)
        fig.update_xaxes(categoryorder="category ascending")
        fig.update_yaxes(range=[0, max_])
        fig.write_image(f"{t_dir}/fig_{ind}.png")
        if ind == 0:
            fig.write_image(f"first.png")
        elif ind == len(dfs)-1:
            fig.write_image(f"last.png")

    images = sorted_alphanumeric(os.listdir(t_dir))
    images_read = []
    for filename in images:
        if ".png" in filename:
            images_read.append(imageio.imread(f"{t_dir}/{filename}"))
    imageio.mimsave(f'output.gif', images_read, duration=gif_duration)

# history / run_info comparison

ri = [{"map": str(map_), "count": count} for map_ , count in run_info.items()]
run_info_df = pd.DataFrame(ri)
run_info_df.sort_values(by="map", inplace=True, ascending=True)
run_info_df.reset_index(drop=True,inplace=True)

h = [{"map": str(map_), "count": count} for map_ , count in history[-1][mode_r].items()]
history_df = pd.DataFrame(h)
history_df.sort_values(by="map", inplace=True, ascending=True)
history_df.reset_index(drop=True, inplace=True)

history_df["percentage"] = history_df["count"]/history_df["count"].sum()  

max_h = round_x_decimal(history_df["percentage"].max())
max_r = round_x_decimal(run_info_df["count"].max())

max_value = max((max_h, max_r))

title = f"History {mode_h}<br><sup>{id_h}</sup>"
history_plot = px.bar(history_df, x="map", y="percentage", template=plot_template, title=title)
history_plot.update_xaxes(categoryorder="category ascending")
history_plot.update_yaxes(range=[0, max_value])
title = f"Run Info {mode_r}<br><sup>{id_r}</sup>"
run_info_plot = px.bar(run_info_df, x="map", y="count", template=plot_template, title=title)
run_info_plot.update_xaxes(categoryorder="category ascending")
history_plot.update_yaxes(range=[0, max_value])

if deviation:
    history_df["deviation"] = history_df["percentage"]-run_info_df["count"]
    title = f"History-Run Info Deviation {mode_r}<br><sup>{id_r}</sup>"
    deviation_plot = px.bar(history_df, x="map", y="deviation", template=plot_template, title=title)
    deviation_plot.update_xaxes(categoryorder="category ascending")
    deviation_plot.write_image("deviation.png")

with tempfile.TemporaryFile() as hist, tempfile.TemporaryFile() as run:
    history_plot.write_image(hist)
    history_img = Image.open(hist)
    run_info_plot.write_image(run)
    run_info_img = Image.open(run)

    if save_single_pics:
        history_plot.write_image("history.png")
        run_info_plot.write_image("run_info.png")

    combined = Image.new('RGB', (history_img.width + run_info_img.width, history_img.height))
    combined.paste(history_img, (0, 0))
    combined.paste(run_info_img, (history_img.width, 0))
    combined.save("history_run_info.png")