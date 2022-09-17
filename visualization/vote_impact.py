import streamlit as st
import pandas as pd
import plotly.express as px
import json
import utils

with open("../config.json", "r") as f:
    config = json.load(f)

def initialize_maps(config):
    pass