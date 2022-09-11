import plotly.express as px
import pandas as pd
import json, os
from PIL import Image


#-# Config #-#

run_info_path = "run_info_eb191fb1-b617-4a50-a315-d5ad83111b79_AAS.json"
history_path = "optimizer_maps_history_AAS_eb191fb1-b617-4a50-a315-d5ad83111b79.json"
remove_single_pics = True

#-#

mode = run_info_path.split("_")[-1].replace(".json", "")

with open(run_info_path, "r") as f:
    run_info = json.load(f)

with open(history_path, "r") as f:
    history = json.load(f)

ri = [{"map": str(map_), "count": count} for map_ , count in run_info.items()]
run_info_df = pd.DataFrame(ri)
run_info_df.sort_values(by="count", inplace=True, ascending=False)

h = [{"map": str(map_), "count": count} for map_ , count in history[-1][mode].items()]
history_df = pd.DataFrame(h)
history_df.sort_values(by="count", inplace=True, ascending=False)

template = "plotly_dark"
history_plot = px.bar(history_df, x="map", y="count", template=template)
history_plot.update_xaxes(categoryorder="category ascending")
run_info_plot = px.bar(run_info_df, x="map", y="count", template=template)
run_info_plot.update_xaxes(categoryorder="category ascending")

history_plot.write_image("history.png")
run_info_plot.write_image("run_info.png")

history_img = Image.open("history.png")
run_info_img = Image.open("run_info.png")

combined = Image.new('RGB', (history_img.width + run_info_img.width, history_img.height))
combined.paste(history_img, (0, 0))
combined.paste(run_info_img, (history_img.width, 0))
combined.save("history_run_info.png")

if remove_single_pics:
    os.remove("history.png")
    os.remove("run_info.png")