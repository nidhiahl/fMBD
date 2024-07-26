#!/usr/bin/env python
# coding: utf-8
import sys


import pandas as pd
import numpy as np
from math import *
from scipy.special import comb



def rand_index(actual, pred):    #first argument: actual clusterid list of all points and second argument: predicted clusterid list

    tp_plus_fp = comb(np.bincount(actual), 2).sum()
    tp_plus_fn = comb(np.bincount(pred), 2).sum()
    A = np.c_[(actual, pred)]
    tp = sum(comb(np.bincount(A[A[:, 0] == i, 1]), 2).sum()
             for i in set(actual))
    fp = tp_plus_fp - tp
    fn = tp_plus_fn - tp
    tn = comb(len(A), 2) - tp - fp - fn
    return (tp + tn) / (tp + fp + fn + tn)

dataset_name = sys.argv[1]
clusteringOutput_filepath = "./clustering_output/"+dataset_name+"/cId.csv"
actualLabel_filepath = "./datasets/"+dataset_name+"/"+dataset_name+"_labels.csv"
#filepath = sys.argv[1]+'/intermediatefiles/mpc_cId.csv'

clustering_output = pd.read_csv(filepath,sep=' ') #Replace test.csv with path of the input file, input must contain actual and predicted clusterid.
actual_labels = pd.read_csv()
ls =[i/1000 for i in range(1, 1001)]
totalPoints = len(clustering_output)/len(ls)
max_Fmeasure = 0.0
max_Dc = 0.1
max_randIndex = 0.0
max_entropy = 0.0
for idx, val in enumerate(ls):
    #actual = np.array(clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1,['actual']])
    #actual = actual[:,0]
    predicted =np.array(clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1,['predicted']])
    predicted = predicted[:,0]
    #print(val,rand_index(actual, predicted))
    df = clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1]
    clusters = df.groupby('actual')['predicted'].apply(list)
    classes = df.groupby('predicted')['actual'].apply(list)
    precision= []
    recall = []
    for class_i in classes:
        class_i = pd.DataFrame(class_i)
        class_i = class_i[0].value_counts()
        precision_i = class_i.max() / class_i.sum()
        recall_i = class_i.max() / len(clusters[class_i.idxmax()])
        precision += [precision_i]
        recall += [recall_i]
    precision = np.array(precision)
    recall = np.array(recall)
    f_measure  = np.average((2 * precision*recall)/(precision+recall))
    #print(val, f_measure)
    cluster_entropy=[]
    for cluster in clusters:
        cluster = pd.DataFrame(cluster)
        classes=cluster[0].value_counts()
        classes = classes/float(cluster.count())
        e = (classes * [log(x, 2) for x in classes]).sum()
        cluster_entropy += [-e]
    cluster_size = np.array([len(c) for c in clusters])
    cluster_fraction = cluster_size/float(cluster_size.sum())
    entropy = (cluster_fraction * cluster_entropy).sum()
    #print (val, entropy)
    rand_Index = rand_index(actual, predicted)
    if max_Fmeasure < f_measure:
        max_Dc = val
        max_Fmeasure = f_measure
        max_randIndex = rand_Index
        max_entropy = entropy


'''filepath = dataset+'/'+mpc_or_dpc+'_time.csv'
runningTime_output = pd.read_csv(filepath,sep=' ') 
t = runningTime_output.iloc[int(max_Dc*1000)-1]

print('dpc',max_Dc*100,t['Dc(distance)'],max_Fmeasure, max_randIndex,max_entropy,t['dPTime'],t['matrixTime'],t['localdensityTime'],t['relativedistanceTime'],t['ccidentificationTime'],t['clusterassignTime'])
'''
filepath = dataset+'/intermediatefiles/'+mpc_or_dpc+'_ccPosition_in_ordrho.csv'
runningTime_output = pd.read_csv(filepath,sep=' ',header=None) 
t = runningTime_output.iloc[int(max_Dc*1000)-1]

print("dpc",max_Dc*100,t[0],max_Fmeasure, max_randIndex,max_entropy)




'''
filepath = sys.argv[1]+'/intermediatefiles/dpc_cId.csv'
clustering_output = pd.read_csv(r'liver/intermediatefiles/dpc_cId.csv',sep=' ') #Replace test.csv with path of the input file, input must contain actual and predicted clusterid.
ls =[i/1000 for i in range(1, 1001)]
#ls = [i/2 for i in range(1, 200)]
totalPoints = len(clustering_output)/len(ls)
for idx, val in enumerate(ls):
    actual = np.array(clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1,['actual']])
    actual = actual[:,0]
    predicted =np.array(clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1,['predicted']])
    predicted = predicted[:,0]
    #print(val,rand_index(actual, predicted))
    df = clustering_output.loc[idx*totalPoints:((idx+1)*totalPoints)-1]
    clusters = df.groupby('actual')['predicted'].apply(list)
    classes = df.groupby('predicted')['actual'].apply(list)
    precision= []
    recall = []
    for class_i in classes:
        class_i = pd.DataFrame(class_i)
        class_i = class_i[0].value_counts()
        precision_i = class_i.max() / class_i.sum()
        recall_i = class_i.max() / len(clusters[class_i.idxmax()])
        precision += [precision_i]
        recall += [recall_i]
    precision = np.array(precision)
    recall = np.array(recall)
    f_measure  = np.average((2 * precision*recall)/(precision+recall))
    #print(val, f_measure)
    cluster_entropy=[]
    for cluster in clusters:
        cluster = pd.DataFrame(cluster)
        classes=cluster[0].value_counts()
        classes = classes/float(cluster.count())
        e = (classes * [log(x, 2) for x in classes]).sum()
        cluster_entropy += [-e]
    cluster_size = np.array([len(c) for c in clusters])
    cluster_fraction = cluster_size/float(cluster_size.sum())
    entropy = (cluster_fraction * cluster_entropy).sum()
    #print (val, entropy)
    rand_Index = rand_index(actual, predicted)
    if max_Fmeasure < f_measure:
        max_Dc = val
        max_Fmeasure = f_measure
        max_randIndex = rand_Index
        max_entropy = entropy
print(max_Dc,max_Fmeasure, max_randIndex,max_entropy)


# In[6]:


#totalPoints


# In[ ]:

'''


