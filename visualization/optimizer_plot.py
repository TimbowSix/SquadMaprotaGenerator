import plotly.express as px
import pandas as pd
import os, json, shutil
import re
import imageio.v2 as imageio

## Setup

history_path = "optimizer_maps_history.json"
gif_duration = 0.5
custom_title = None


def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ] 
    return sorted(data, key=alphanum_key)

def main():
    path = "/.temp"
    exist = os.path.exists(path)
    if not exist:
        os.makedirs(path)
        
    with open(history_path, "r") as f:
        file = json.load(f)

    for ind, maps in enumerate(file):
        count = [{"map": str(map_), "count": count} for map_ , count in maps.items()]
        df = pd.DataFrame(count)
        df.sort_values(by="count", inplace=True, ascending=False)
        title = f"{ind}" if custom_title is None else custom_title
        fig = px.bar(df, x="map", y="count", template="plotly_dark", title=title)
        fig.update_xaxes(categoryorder="category ascending")
        fig.update_yaxes(range=[0, 3000])
        fig.write_image(f"{path}/fig_{ind}.png")
        if maps == file[0]:
            fig.write_image(f"first.png")
        elif maps == file[-1]:
            fig.write_image(f"last.png")

    images = os.listdir(path)
    images = sorted_alphanumeric(os.listdir(path))
    images_read = []
    for filename in images:
        if ".png" in filename:
            images_read.append(imageio.imread(f"{path}/{filename}"))
    imageio.mimsave(f'output.gif', images_read, duration=gif_duration)

    shutil.rmtree("/.temp")

if __name__ == "__main__":
    main()