#Author Benjamin Pachev <benjaminpachev@greenseedtechnologies.com>
#
#Python script to generate a simple XGBoost model for the titanic dataset

from pandas import read_csv
from numpy import array, mean
import xgboost as xgb

df = read_csv("titanic.csv")
labels = array(df["Survived"])
#Dummy code the Sex column
df["Sex_is_female"] = df["Sex"] == "female"

missing_val = -999
matrix = xgb.DMatrix(df[["Sex_is_female", "Age", "Fare"]].fillna(-999), label=labels, missing=missing_val)

params = {"max_depth":3, "num_rounds":10, "objective":"binary:logistic"}
model = xgb.train(params, matrix)

preds = model.predict(matrix)
pred_labels = preds > .5
print "Accuracy on train set {:.4f}%".format(100*mean(pred_labels==labels))
#Save a model for PHP to use
model.save_model("titanic.model")
