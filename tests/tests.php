<?php
# @author Benjamin Pachev <benjamin.pachev@greenseedtech.com>
#
# A script to test the PHP xgboost extension. Compares computed predictions in PHP to those in an input JSON file

function safe_arr_get($arr, $key)
{
	if (!is_array($arr) || !array_key_exists($key, $arr)) return false;
	else return $arr[$key];
}

function fail($msg)
{
	echo "$msg\n";
	exit(1);
}

function safe_json_read($fname)
{
	if (!($file = fopen($fname, "r")))
	{
		fail("Failure reading from file $fname");
		return;
	}
	$res = json_decode(fread($file, filesize($fname)), true);
	switch (json_last_error()) {
        case JSON_ERROR_NONE:
        break;
        case JSON_ERROR_DEPTH:
            fail(' - Maximum stack depth exceeded');
        break;
        case JSON_ERROR_STATE_MISMATCH:
            fail(' - Underflow or the modes mismatch');
        break;
        case JSON_ERROR_CTRL_CHAR:
            fail(' - Unexpected control character found');
        break;
        case JSON_ERROR_SYNTAX:
            fail(' - Syntax error, malformed JSON');
        break;
        case JSON_ERROR_UTF8:
            fail(' - Malformed UTF-8 characters, possibly incorrectly encoded');
        break;
        default:
            fail(' - Unknown error');
        break;
    }
	fclose($file);
	return $res;
}

function expect($condition, $msg="")
{
	if (!$condition) fail("Failed test, error: $msg\n");
}

function expect_key_exists($key, $arr)
{
	expect(array_key_exists($key, $arr), "Array missing key $key.");
}

function expect_array($var)
{
	expect(is_array($var), "Expected array, got ".gettype($var));
}

//Assert that two floats are within a specified relative tolerance u <= (1+epsilon)
function float_close($u, $v, $tol)
{
	if (abs($u-$v) <= $tol) return True;
	else
	{
		$bound1 = (1-$tol) * $v;
		$bound2 = (1+$tol) * $v;
		return (max($bound1, $bound2) >= $u) && (min($bound1, $bound2) <= $u);
	}
}

function expect_arr_allclose($u, $v, $tol=1e-5)
{
	expect_equal_len($u, $v);
	$len = count($u);
	for ($i = 0; $i < $len; $i++)
	{
		expect(float_close($u[$i], $v[$i], $tol), "Index $i, ".$u[$i]." is not close to ".$v[$i]);
	}
}

function expect_equal_len($u, $v)
{
	expect_array($u);
	expect_array($v);
	$len_u = count($u);
	$len_v = count($v);
	expect($len_u == $len_v, "Array 1 has length ".$len_u." array 2 has length ".$len_v);
}

function validate_json_input($data)
{
	expect_key_exists("model_filename", $data);
	expect_key_exists("input_rows", $data);
	expect_key_exists("expected_predictions", $data);
	$input_rows = $data["input_rows"];
	$preds = $data["expected_predictions"];
	expect_equal_len($input_rows, $preds);
}

function run_test($test_json_filename)
{
	echo "On test case $test_json_filename\n";
	$data = safe_json_read($test_json_filename);
	validate_json_input($data);
	$bst = new XGBooster();
	$bst->loadModel($data["model_filename"]);

	if (!count($data["input_rows"]))
	{
		//nothing to test
		return;
	}

	$num_cols = count($data["input_rows"][0]);
	$mat = new XGDMatrix($data["input_rows"], $num_cols);

	$missing_val = 0;
	$preds = $bst->predict($mat, $missing_val);
	expect_arr_allclose($preds, $data["expected_predictions"]);
}


foreach (glob(__DIR__."/test*.json") as $test_case_filename)
{
	run_test($test_case_filename);
}
echo "All tests pass.\n";

?>