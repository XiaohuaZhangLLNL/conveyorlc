import numpy as np
import pandas as pd


drug_df=pd.read_csv('results/drug_13.txt', skipinitialspace=True)
durg_sort_df=drug_df.sort_values(by='ligName', ascending=True)

bindGB_min_s=durg_sort_df.groupby(['ligName'], sort=True)['bindGB'].min()
bindGB_min_df=bindGB_min_s.to_frame()
dock_min_s=durg_sort_df.groupby(['ligName'], sort=True)['dockScore'].min()
dock_min_df=dock_min_s.to_frame()

data_df = dock_min_df.merge(bindGB_min_df, left_index=True, right_index=True)

data_dock_sort=data_df.sort_values(by='dockScore', ascending=True)
data_dock_sort['dock_rank'] = np.arange(len(data_dock_sort))
data_gbsa_sort=data_dock_sort.sort_values(by='bindGB', ascending=True)
data_gbsa_sort['gbsa_rank'] = np.arange(len(data_gbsa_sort))

data_gbsa_sort['dock_gbsa'] = data_gbsa_sort.dock_rank + data_gbsa_sort.gbsa_rank
dock_gbsa_sort=data_gbsa_sort.sort_values(by='dock_gbsa', ascending=True)
dock_gbsa_sort['dock_gbsa_rank'] = np.arange(len(dock_gbsa_sort))

dock_gbsa_sort.to_csv("dock_gbsa_rank.csv")
