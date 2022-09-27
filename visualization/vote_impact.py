import streamlit as st
import pandas as pd
import plotly.express as px
import json
import utils
import vote_impact_calc as calc

st.set_page_config(page_title="Vote Impact", layout="wide")

with open("config.json", "r") as f:
    config = json.load(f)

all_modes = []
for pool in config["mode_distribution"]["pools"]:
    for mode in config["mode_distribution"]["pools"][pool]:
        if config["mode_distribution"]["pools"][pool][mode] > 0:
            all_modes.append(mode)

def getMaps():
    maps = calc.initialize_maps(config)
    return maps

maps = getMaps()

st.sidebar.header("Filter")

modes = st.sidebar.multiselect(
    "Modes:",
    options=all_modes,
    default=all_modes
)

for map_ in maps:
    with st.sidebar.expander(map_.name):
        for mode in modes:
            if mode in map_.layers:
                for layer in map_.layers[mode]:
                    st.number_input(layer.name, value=layer.votes, key=layer.name)
                    #st.write(layer.name, layer.votes)

for map_ in maps:
    for mode in map_.layers:
        if mode in modes:
            for layer in map_.layers[mode]:
                layer.votes = st.session_state[layer]
    map_.add_mapvote_weights(config["mapvote_slope"], config["mapvote_shift"])

distribution = calc.normalize_mapvote_weights_by_mode(maps)

dist = [{"map":map_, "dist":dist, "mode": mode} for map_ in distribution for mode, dist in distribution[map_].items()]
dist_df = pd.DataFrame(dist)

rows = []
for i in range(int(len(all_modes)/2+0.5)):
    col1, col2 = st.columns(2)
    rows.append((col1,col2))

for ind, mode in enumerate(modes):
    col = rows[int(ind/2)][ind%2]
    fig = px.bar(dist_df.loc[dist_df["mode"] == mode], x="map", y="dist", template="plotly_dark", title=f"{mode}")
    fig.update_xaxes(categoryorder="category ascending")
    col.plotly_chart(fig)
