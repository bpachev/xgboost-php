XGBoost PHP Package
===================

Installation
------------
- Start by cloning and building xgboost from source.
- change directories to php-package/xgboost
- At this point, the installation process is the same as when building any PHP extension from source.
- If on Ubuntu, you can run ./easy_install.sh
- Otherwise, do the following:
  ```
  phpize
  ./configure --enable-xgboost
  make
  sudo make install
  ```
- Then, you'll need to enable your extension by adding the line extension=xgboost.so to php.ini.
- See [this tutorial](https://www.sitepoint.com/install-php-extensions-source/) for further reading on installing PHP extensions.

Examples
--------

A basic example of how to create a DMatrix, load a model, and get predictions.

```
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
$bst->loadModel("example.model");
#$preds will be a PHP array with three entries corresponding to the three rows in $dmat
$preds = $bst->predict($dmat);
?>
```
