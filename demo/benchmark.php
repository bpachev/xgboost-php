<?php

function make_matrix($n=7000000)
{
	$data = [];
	for ($i=0; $i < $n; $i++) $data[] = [$i/$n, 1/(1+$i), log(1+$i)];
	$matrix = new XGDMatrix($data, 3);
	return $matrix;
}

$mat = make_matrix();
echo "Created matrix.\n";
$booster = new XGBooster();
$booster->loadModel(__DIR__."/titanic.model");
$start = microtime(True);
$preds = $booster->predict($mat);
$elapsed = microtime(True) - $start;
echo "Time with all threads $elapsed.\n";
$booster->setParam("nthread", 1);
$start = microtime(True);
$preds = $booster->predict($mat);
$elapsed = microtime(True) - $start;
echo "Time with 1 thread $elapsed\n";
?>
