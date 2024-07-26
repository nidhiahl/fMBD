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


def f_measure(clusters,classes):
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
    return f_measure    

def entropy(clusters,classes):
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
    return entropy



dataset_name = sys.argv[1]
step_size = float(sys.argv[2])
max_percent = float(sys.argv[3])
clusteringOutput_filepath = sys.argv[4]

actualLabel_filepath = "./datasets/"+dataset_name+"/"+dataset_name+"_labels.csv"

actual_labels = pd.read_csv(actualLabel_filepath,sep=' ',header=None)

clustering_output = pd.read_csv(clusteringOutput_filepath,sep=' ',header=None) 

runs = int(max_percent/step_size)
totalPoints = (len(clustering_output)/(runs+1))-1
print(runs, max_percent/step_size)
max_Fmeasure = 0.0
max_Dc = 0.1
max_randIndex = 0.0
max_entropy = 0.0

for run in range(runs+1):
    cutoff_id = int(run*(totalPoints+1))
    
    df = pd.DataFrame(columns=['actual','predicted'])
    df['actual'] = actual_labels
    df['predicted'] = np.array(clustering_output.loc[cutoff_id+1:cutoff_id+totalPoints,[0]])
    clusters = df.groupby('actual')['predicted'].apply(list)
    classes = df.groupby('predicted')['actual'].apply(list)
    
    f=f_measure(clusters,classes)
    r=rand_index(np.array(df['actual']).astype(int),np.array(df['predicted']).astype(int))
    e=entropy(clusters,classes)
    print(run*step_size+step_size,clustering_output.loc[cutoff_id,0],f,r,e)
    
    if max_Fmeasure < f:
        max_Dc = clustering_output.loc[cutoff_id,0]
        max_Fmeasure = f
        max_randIndex = r
        max_entropy = e
print("maximum_fmeasure",max_Dc,max_Fmeasure,max_randIndex,max_entropy)







