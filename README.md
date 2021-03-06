XGBoost PHP Wrapper
===================

Abstract
--------
This is a PHP wrapper for [XGBoost](https://github.com/tqchen/xgboost), a popular and powerful machine learning library for gradient boosting. Currently, this wrapper only has support for making predictions using pre-trained models. The idea is that data scientists would build XGBoost models using Python, R or their favorite language for data analysis. Those models could then be easily used in PHP production environment.

PHP isn't a popular language for data science. Consequently, this wrapper doesn't currently have the capability to build models. However, such capability could easily be added if there is demand for it. 

Installation
------------
- Clone the repository ```git clone --recursive https://github.com/bpachev/xgboost-php```
- Build XGBoost
   ```
   cd xgboost
   make
   ```
- change directories to src
   ```
   cd ..
   cd src
   ```
- At this point, the installation process is the same as when building any PHP extension from source.
- If on Ubuntu, you can run ./easy_install.sh, which will install the extension and use phpenmod to enable it.
- Otherwise, do the following:
  ```
  phpize
  ./configure
  make
  sudo make install
  ```
- Then, you'll need to enable your extension by adding the line extension=xgboost.so to php.ini.
- See [this tutorial](https://www.sitepoint.com/install-php-extensions-source/) for further reading on installing PHP extensions.
- On Ubuntu, if phpize is not installed, you can install it and other PHP development tools by sudo apt install php-dev. 

Documentation
-------------
This extension adds two classes to PHP: XGDMatrix, which wraps the native XGBoost DMatrix, and XGBooster, which wraps XGBoost's Booster.

### XGDMatrix
```php
$matrix = new XGDMatrix($data, $numColumns, $missing=null);
```
where 
```
$data - a PHP array of arrays. Each sub-array should contain only numeric elements or nulls.
  If strings are provided, they will be converted to floats.
$numColumns -- the expected number of columns in the DMatrix.
   Even if some sub-arrays contain more than $numColumns columns,
   only the first $numColumns entries will be read.
$missing -- the value to interpret as missing. If provided, must be a float. Regardless of the value of $missing, null is always interpreted as missing.
```

with methods
```php
#Return the integer number of columns in the underlying DMatrix
$matrix->getNumCol();

#Return the integer number of rows in the underlying DMatrix
$matrix->getNumCol();
```

### XGBooster
```php
$booster = new XGBooster();
```
with methods
```php
#Load a prebuilt XGBoost model from the file named $filename
$booster->loadModel($filename);

#If a model has been loaded, we can use it to make predictions
#$matrix must be an XGDMatrix.
#$num_tree_limit is an optional second parameter. If set to 0, all trees are used.
#If a positive integer, only the first $num_tree_limit trees are used to make predictions.
#$preds will be an array of predictions with length equal to the number of rows in $matrix
$preds = $booster->predict($matrix, $num_tree_limit=0)

#Get model attribute 'my_data'
print($booster->getAttr('my_data'));

#Set model attribute 'my_data' to 'my_value'
$booster->setAttr('my_data', 'my_value');

#Get XGB last error
print(XGBooster::getLastError());

#Set the number of threads to use for prediction (for a full list of parameters, check out the XGBoost documentation)
#Unlike setAttr, parameters have an effect on the behavior of the booster.
$booster->setParam("nthread", 1);
```

Examples
--------

A basic example of how to create a DMatrix, load a model, and get predictions.

```php
<?php
#Create a 2-D PHP array with some data.
#We can pass in mixed types and it will all be converted to float
$data = [[1,1], [1., True], [0, 7]];
$numColumns = 2;
$dmat = new XGDMatrix($data, $numColumns);
print "Number of columns: ".$dmat->getNumCol()."\n";
print "Number of rows: ".$dmat->getNumRow()."\n";

$bst = new XGBooster();
#Load a saved model file
#example.model would be a binary XGBoost model file
$bst->loadModel("example.model");
#$preds will be a PHP array with three entries corresponding to the three rows in $dmat
$preds = $bst->predict($dmat);
?>
```

Another very useful feature is model attributes. You can store some extra information in model file before exporting model then get that data back after model loading:

```
# Python code ...
model = xgb.train(params, matrix)
model.set_attr(my_data='model ver 1.0')

#Save a model for PHP to use
model.save_model("titanic.model")
```

```
# Then in PHP...
$bst = new XGBooster();
#Load a saved model file
#example.model would be a binary XGBoost model file
$bst->loadModel("example.model");
print $bst->getAttr('my_data');
```

For a more realistic demo, see 
[Titanic demo](https://github.com/bpachev/xgboost-php/blob/master/demo/titanic_demo.php)

For an example of how to control the number of threads used in prediction, see
[Benchmark](https://github.com/bpachev/xgboost-php/blob/master/demo/benchmark.php)

Running Tests
-------------
Simply run 
```
php tests/tests.php
```
