import plotly.express as px
import pandas as pd
import json
from PIL import Image
import tempfile


#-# Config #-#

run_info_path = "run_info_eb191fb1-b617-4a50-a315-d5ad83111b79_AAS.json"
history_path = "optimizer_maps_history_AAS_eb191fb1-b617-4a50-a315-d5ad83111b79.json"
plot_template = "plotly_dark"

#-#

mode = run_info_path.split("_")[-1].replace(".json", "")

with open(run_info_path, "r") as fr, open(history_path, "r") as fh:
    run_info = json.load(fr)
    history = json.load(fh)

ri = [{"map": str(map_), "count": count} for map_ , count in run_info.items()]
run_info_df = pd.DataFrame(ri)
run_info_df.sort_values(by="count", inplace=True, ascending=False)

h = [{"map": str(map_), "count": count} for map_ , count in history[-1][mode].items()]
history_df = pd.DataFrame(h)
history_df.sort_values(by="count", inplace=True, ascending=False)

title = f"History"
history_plot = px.bar(history_df, x="map", y="count", template=plot_template, title=title)
history_plot.update_xaxes(categoryorder="category ascending")
title = f"Run Info"
run_info_plot = px.bar(run_info_df, x="map", y="count", template=plot_template, title=title)
run_info_plot.update_xaxes(categoryorder="category ascending")

with tempfile.TemporaryFile() as hist, tempfile.TemporaryFile() as run:
    history_plot.write_image(hist)
    history_img = Image.open(hist)
    run_info_plot.write_image(run)
    run_info_img = Image.open(run)

    combined = Image.new('RGB', (history_img.width + run_info_img.width, history_img.height))
    combined.paste(history_img, (0, 0))
    combined.paste(run_info_img, (history_img.width, 0))
    combined.save("history_run_info.png")