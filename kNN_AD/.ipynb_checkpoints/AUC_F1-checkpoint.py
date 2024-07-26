import pandas as pd
import numpy as np
from sklearn import metrics
import sys

dataset=str(sys.argv[1])
path_to_scores_file=str(sys.argv[2])
#path_to_output_results=str(sys.argv[3])


pred_scores = pd.read_csv(path_to_scores_file,sep=' ')
actual_labels = pd.read_csv('./datasets/'+dataset+'/'+dataset+'_labels.csv',sep=' ',header=None)
num_anomalies=actual_labels[0].value_counts()[1]

best_auc = 0;
best_Fscore=0;
k_list = pred_scores.columns
for col_header in k_list:
    print(col_header,end=' ')
    scores_labels = pd.DataFrame(columns=['score','actual_labels'])
    scores_labels['score'] = pred_scores[col_header]
    scores_labels['actual_labels'] = actual_labels
    scores_labels.sort_values(by='score',ascending=False,inplace=True)
    pred_labels = np.zeros(len(actual_labels))
    pred_labels[:num_anomalies]=1
    scores_labels['pred_labels']=pred_labels
    f_score=metrics.f1_score(scores_labels['actual_labels'],scores_labels['pred_labels'])
    fpr, tpr, thresholds = metrics.roc_curve(scores_labels['actual_labels'], scores_labels['score'], pos_label=0)
    auc_score = metrics.auc(fpr, tpr)
    print(f_score,auc_score)
    if best_auc < auc_score:
        best_aus=auc_score
        best_Fscore=f_score
print(best_Fscore,best_auc)
