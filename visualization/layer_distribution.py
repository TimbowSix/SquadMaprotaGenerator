import pandas as pd
import utils
import plotly.express as px
import json
import collections

layer_path = "./layer_1.cfg"
#layer_path = "../test_layer_1.cfg"
out_path = "visualization/output_test/"
template = "plotly_white"
save_single = True
output_type = "pdf"

with open("./data/current_map_dist.json", "r") as e, open(layer_path, "r") as l:
    expected = json.load(e)
    layers = l.read().splitlines()

df = pd.DataFrame(dict(layer=layers))

df["map"] = df.apply(lambda row: utils.parseLayer(row.layer)[0], axis=1)
df["mode"] = df.apply(lambda row: utils.parseLayer(row.layer)[1], axis=1)

groups = df.groupby("mode")

def freq_df(df, column, count_name="Häufigkeit"):
    freq = df[column].value_counts().reset_index()
    freq.rename(columns={column:count_name,"index":column}, inplace=True)
    return freq

#df = freq_df(groups.get_group("AAS"), "map", "count")
dfs = []
for name, group in groups:
    ndf = freq_df(group, "map", "count")
    ndf["mode"] = name
    dfs.append(ndf)
all_df = pd.concat(dfs)

exp = [{"map":map_, "dist":dist, "mode": mode} for map_ in expected for mode, dist in expected[map_].items()]
exp_df = pd.DataFrame(exp)

plots = []
for mode in all_df["mode"].unique():
    fig1 = px.bar(all_df.loc[all_df["mode"] == mode], x="map", y="count", template=template, title=f"generated<br><sup>{mode}</sup>")
    fig1.update_xaxes(categoryorder="category ascending")
    fig2 = px.bar(exp_df.loc[exp_df["mode"] == mode], x="map", y="dist", template=template, title=f"expected<br><sup>{mode}</sup>")
    fig2.update_xaxes(categoryorder="category ascending")
    plots.append({"mode":mode, "generated": fig1, "expected": fig2})

import tempfile
from PIL import Image

for data in plots:
    with tempfile.TemporaryFile() as genf, tempfile.TemporaryFile() as expf:
        data["generated"].write_image(genf)
        generated_plt = Image.open(genf)
        data["expected"].write_image(expf)
        expected_plt = Image.open(expf)

        combined = Image.new('RGB', (generated_plt.width + expected_plt.width, generated_plt.height))
        combined.paste(generated_plt, (0, 0))
        combined.paste(expected_plt, (generated_plt.width, 0))
        combined.save(f"{out_path}{data['mode']}.png")

        if save_single:
            data["generated"].write_image(f"{out_path}generated{data['mode']}.{output_type}")
            generated_plt = Image.open(genf)
            data["expected"].write_image(f"{out_path}expected{data['mode']}.{output_type}")