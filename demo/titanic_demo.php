<?php
define('TITANIC_NUM_FEATURE', 3);

function accuracy_score($labels, $preds)
{
    $total = count($preds);
    $num_correct = 0;
    foreach ($preds as $i => $pred)
    {
        $pred_label = ($pred > .5) ? 1 : 0;
        if ($pred_label == $labels[$i]) $num_correct += 1;
    }
    return $num_correct / $total;
}

function bail($msg)
{
    echo "$msg\n";
    exit(1);
}

function fix_null($v)
{
    #The default missing value for XGDMatrix is null, so we don't want to cast it to a float (which would make it zero).
    #XGBoost has special handling for missing values
    return (is_null($v)) ? $v : (float)$v;
}

function read_titanic_data(&$data, &$labels, $filename=__DIR__."/titanic.csv")
{
    if (!($file=fopen($filename, "r")))
    {
        bail("Error opening titanic dataset in $filename.");
    }
    #Read column names
    $column_names = fgetcsv($file);
    $names_to_inds = array_flip($column_names);

    $data = [];
    while (($row=fgetcsv($file)))
    {
        $is_female = ($row[$names_to_inds["Sex"]] == "female");
        $age = fix_null($row[$names_to_inds["Age"]]);
        $fare = fix_null($row[$names_to_inds["Fare"]]);
        $labels[] = (int)$row[$names_to_inds["Survived"]];
        $data[] = [$is_female, $age, $fare];
    }
}

read_titanic_data($data, $labels);

$matrix = new XGDMatrix($data, TITANIC_NUM_FEATURE);

$booster = new XGBooster();
$booster->loadModel(__DIR__."/titanic.model");
$preds = $booster->predict($matrix);

echo "Accuracy ".accuracy_score($labels, $preds)."\n";
$tree_limit = 1;
echo "Accuracy with one tree: ".accuracy_score($labels, $booster->predict($matrix, $tree_limit))."\n";
?>
