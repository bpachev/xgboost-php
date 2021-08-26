
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xgboost.h"
#include "zend_exceptions.h"

/**
 * @author Benjamin Pachev <benjamin.pachev@greedseedtech.com>
 *
 * begin
 */
#include <stdint.h>
#include "xgboost/c_api.h"
#include <cmath>
#include <limits>
#include <stdlib.h>

using namespace std;

/* If you declare any globals in php_xgboost.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(xgboost)
*/

/* True global resources - no need for thread safety here */
static int le_xgboost;

/* {{{ xgboost_functions[]
 *
 * Every user visible function must have an entry in xgboost_functions[].
 */
const zend_function_entry xgboost_functions[] = {
// 	PHP_FE(confirm_xgboost_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in xgboost_functions[] */
};
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_dmatrix___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, input_arr)
	ZEND_ARG_INFO(0, ncol)
	ZEND_ARG_INFO(0, missing)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dmatrix_getNumRow, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dmatrix_getNumCol, 0, 0, 0)
ZEND_END_ARG_INFO()

zend_function_entry dmatrix_methods[] = {
	PHP_ME(DMatrix, __construct, arginfo_dmatrix___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(DMatrix, getNumRow, arginfo_dmatrix_getNumRow, ZEND_ACC_PUBLIC)
	PHP_ME(DMatrix, getNumCol, arginfo_dmatrix_getNumCol, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_object_handlers dmatrix_object_handlers;
zend_class_entry* dmatrix_ce;

//For compatibility with PHP 7, the zend_object must be the last entry of the struct as it is variable length
struct dmatrix_object
{
	XG_CUST_OBJ_STRUCT_START()
	DMatrixHandle handle;
	XG_CUST_OBJ_STRUCT_END()
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_loadModel, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_predict, 0, 0, 1)
	ZEND_ARG_INFO(0, dmatrix)
	ZEND_ARG_INFO(0, ntree_limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_setParam, 0, 0, 2)
	ZEND_ARG_INFO(0, paramname)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_setAttr, 0, 0, 2)
	ZEND_ARG_INFO(0, attrname)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_getAttr, 0, 0, 1)
	ZEND_ARG_INFO(0, attrname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_booster_getLastError, 0, 0, 0)
ZEND_END_ARG_INFO()

zend_function_entry booster_methods[] = {
	PHP_ME(Booster, __construct, arginfo_booster___construct,  ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Booster, getLastError, arginfo_booster_getLastError, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Booster, getAttr, arginfo_booster_getAttr, ZEND_ACC_PUBLIC)
	PHP_ME(Booster, setAttr, arginfo_booster_setAttr, ZEND_ACC_PUBLIC)
	PHP_ME(Booster, setParam, arginfo_booster_setParam, ZEND_ACC_PUBLIC)
	PHP_ME(Booster, loadModel, arginfo_booster_loadModel, ZEND_ACC_PUBLIC)
	PHP_ME(Booster, predict, arginfo_booster_predict, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_object_handlers booster_object_handlers;
zend_class_entry* booster_ce;

struct booster_object
{
	XG_CUST_OBJ_STRUCT_START()
	BoosterHandle handle;
	XG_CUST_OBJ_STRUCT_END()
};


/* {{{ xgboost_module_entry
 */
zend_module_entry xgboost_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"xgboost",
	xgboost_functions,
	PHP_MINIT(xgboost),
	NULL,/*PHP_MSHUTDOWN(xgboost),*/
	NULL,/*PHP_RINIT(xgboost),*/		/* Replace with NULL if there's nothing to do at request start */
	NULL,/*PHP_RSHUTDOWN(xgboost),*/	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(xgboost),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_XGBOOST_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XGBOOST
ZEND_GET_MODULE(xgboost)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("xgboost.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_xgboost_globals, xgboost_globals)
    STD_PHP_INI_ENTRY("xgboost.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_xgboost_globals, xgboost_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_xgboost_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_xgboost_init_globals(zend_xgboost_globals *xgboost_globals)
{
	xgboost_globals->global_value = 0;
	xgboost_globals->global_string = NULL;
}
*/
/* }}} */

inline bool check_xgboost_call(int return_val)
{
	return !return_val;
}

void dmatrix_free_storage(XG_OBJ_FREE_STORAGE_ARG_TYPE object)
{
	dmatrix_object * d_obj = XG_OBJ_FREE_GET_CUST_STRUCT_P(dmatrix_object, object);
	XGDMatrixFree(d_obj->handle);
	zend_object_std_dtor(&(d_obj->std));
#if PHP_MAJOR_VERSION >= 7
#else
	efree(d_obj);
#endif
}

//Defines the create handler
XG_DEFAULT_CREATE_OBJECT_HANDLER(dmatrix_object, dmatrix_create_handler, dmatrix_object_handlers, dmatrix_free_storage)

void booster_free_storage(XG_OBJ_FREE_STORAGE_ARG_TYPE object)
{
	booster_object * d_obj = XG_OBJ_FREE_GET_CUST_STRUCT_P(booster_object, object);
	XGBoosterFree(d_obj->handle);
	zend_object_std_dtor(&(d_obj->std));
#if PHP_MAJOR_VERSION >= 7
#else
	efree(d_obj);
#endif
}

XG_DEFAULT_CREATE_OBJECT_HANDLER(booster_object, booster_create_handler, booster_object_handlers, booster_free_storage)

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xgboost)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "XGDMatrix", dmatrix_methods);
	dmatrix_ce = zend_register_internal_class(&ce);
	dmatrix_ce->create_object = dmatrix_create_handler;
	memcpy(&dmatrix_object_handlers,
		zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dmatrix_object_handlers.clone_obj = NULL;

	zend_class_entry booster_temp_ce;
	INIT_CLASS_ENTRY(booster_temp_ce, "XGBooster", booster_methods);
	booster_ce = zend_register_internal_class(&booster_temp_ce);
	booster_ce->create_object = booster_create_handler;
	memcpy(&booster_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	booster_object_handlers.clone_obj = NULL;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
PHP_MSHUTDOWN_FUNCTION(xgboost)
{
	// uncomment next line if you have INI entries
	//UNREGISTER_INI_ENTRIES();
	return SUCCESS;
} */
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
PHP_RINIT_FUNCTION(xgboost)
{
	return SUCCESS;
} */
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
PHP_RSHUTDOWN_FUNCTION(xgboost)
{
	return SUCCESS;
} */
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xgboost)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "xgboost support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_xgboost_compiled(string arg)
   Return a string to confirm that the module is compiled in */
// PHP_FUNCTION(confirm_xgboost_compiled)
// {
// 	char *arg = NULL;
// 	int arg_len, len;
// 	char *strg;
//
// 	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
// 		return;
// 	}
//
// 	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "xgboost", arg);
// 	RETURN_STRINGL(strg, len, 0);
// }
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto DMatrix contruct(array input_arr, int num_col)
   Construct an xgboost DMatrix from the input array.
   input_arr is an array of arrays. Each element of input_arr is an array of numeric values,
   and corresponds to a row in the DMatrix.
   The number of columns must be specified with num_col. Each element of input_arr should be of length num_col.
   */
PHP_METHOD(DMatrix, __construct)
{
	zval* input_arr;
	bst_ulong nrow;
	bst_ulong ncol;
	double missing = NAN;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "al|d", &input_arr, &ncol, &missing) == FAILURE) {
		RETURN_NULL();
	}

	float float_missing = (isnan(missing)) ? std::numeric_limits<float>::quiet_NaN() : (float)missing;

	nrow = (bst_ulong)zend_hash_num_elements(Z_ARRVAL_P(input_arr));

	HashPosition pos;

	bst_ulong row = 0;
	//Create a buffer to store the current row
	float* row_buf = (float*) emalloc(ncol * sizeof(float));
	//Allocate memory for the dmatrix
	float* data = (float*) emalloc(ncol * nrow * sizeof(float));
	memset(data, 0, ncol*nrow*sizeof(float));
	zval * entry;
	XG_FOREACH_VAL(Z_ARRVAL_P(input_arr), entry) {
		bst_ulong col = 0;
 		if (Z_TYPE_P(entry) == IS_ARRAY) {
 			zval * row_entry;

 			XG_FOREACH_VAL(Z_ARRVAL_P(entry), row_entry) {

				register float el = float_missing;
				if (Z_TYPE_P(row_entry) != IS_NULL) {
					XG_EXTRACT_DVAL_P(row_entry, el);
				}
				row_buf[col] = el;
				col++;
				if (col >= ncol) break;
			} XG_FOREACH_END();
 		}
 		// Copy the row buffer to the corresponding row
		memcpy(data+row*ncol, row_buf, ncol * sizeof(float));
		row++;
		if (row >= nrow) break;
	} XG_FOREACH_END();

	dmatrix_object * obj = XG_GET_THIS(dmatrix_object);
	if (!check_xgboost_call(XGDMatrixCreateFromMat(data, nrow, ncol, float_missing, &(obj->handle)))) {
		RETURN_NULL();
	}

	efree(row_buf);
	efree(data);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto DMatrix long getNumRow()
   Return the number of rows in the DMatrix
   */
PHP_METHOD(DMatrix, getNumRow)
{
	dmatrix_object * obj = XG_GET_THIS(dmatrix_object);
	bst_ulong numRow;

	if (check_xgboost_call(XGDMatrixNumRow(obj->handle, &numRow))) {
		RETURN_LONG(numRow);
	}
	else RETURN_NULL();
}
/* }}} */

/* {{{ proto DMatrix long getNumCol()
   Return the number of columns in the DMatrix
   */
PHP_METHOD(DMatrix, getNumCol)
{
	dmatrix_object * obj = XG_GET_THIS(dmatrix_object);
	bst_ulong numCol;

	if (check_xgboost_call(XGDMatrixNumCol(obj->handle, &numCol))) {
		RETURN_LONG(numCol);
	}
	else RETURN_NULL();

}
/* }}} */

/* {{{ proto string Booster::getLastError()
 * Returns XGB last error (XGBGetLastError in c_api.h) */
PHP_METHOD(Booster, getLastError)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	const char *result = XGBGetLastError();

	RETURN_STRINGL(result, sizeof(*result)-1);
}
/* }}}*/

/* {{{ proto Booster contruct()
   Construct an xgboost Booster.
   */
PHP_METHOD(Booster, __construct)
{
	booster_object * obj = XG_GET_THIS(booster_object);
	bst_ulong num_dmats = 0;
	DMatrixHandle dmats[0];

	if (!check_xgboost_call(XGBoosterCreate(dmats, num_dmats, &(obj->handle)))) {
		RETURN_NULL();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Booster string getAttr(string attrname)
   Return the attribute value of Booster
   */
PHP_METHOD(Booster, getAttr)
{
	zend_string *attrname;
	int success = 0;
	const char *result;

	#undef IS_UNDEF
	#define IS_UNDEF Z_EXPECTED_LONG
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(attrname)
	ZEND_PARSE_PARAMETERS_END();
	#undef IS_UNDEF
	#define IS_UNDEF 0

	booster_object * obj = XG_GET_THIS(booster_object);
	if (check_xgboost_call(XGBoosterGetAttr(obj->handle, ZSTR_VAL(attrname), &result, &success)) && success) {
		RETURN_STR(zend_string_init(result, strlen(result), 0));
	}
	else RETURN_NULL();
}
/* }}} */

/* {{{ proto Booster bool setAttr(string attrname, string value)
   Set the attribute value of Booster
   */
PHP_METHOD(Booster, setAttr)
{
	zend_string *attrname, *value;

	#undef IS_UNDEF
	#define IS_UNDEF Z_EXPECTED_LONG
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(attrname)
		Z_PARAM_STR(value)
	ZEND_PARSE_PARAMETERS_END();
	#undef IS_UNDEF
	#define IS_UNDEF 0

	booster_object * obj = XG_GET_THIS(booster_object);
	if (check_xgboost_call(XGBoosterSetAttr(obj->handle, ZSTR_VAL(attrname), ZSTR_VAL(value)))) {
		RETURN_TRUE;
	}
	else RETURN_FALSE;
}
/* }}} */

/* {{{ proto Booster bool setParam(string paramname, string value)
   Set a parameter value of the booster (such as nthread). This will affect the behavior of the booster.
   */
PHP_METHOD(Booster, setParam)
{
	zend_string *paramname, *value;

	#undef IS_UNDEF
	#define IS_UNDEF Z_EXPECTED_LONG
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(paramname)
		Z_PARAM_STR(value)
	ZEND_PARSE_PARAMETERS_END();
	#undef IS_UNDEF
	#define IS_UNDEF 0

	booster_object * obj = XG_GET_THIS(booster_object);
	if (check_xgboost_call(XGBoosterSetParam(obj->handle, ZSTR_VAL(paramname), ZSTR_VAL(value)))) {
		RETURN_TRUE;
	}
	else RETURN_FALSE;
}
/* }}} */

/* {{{ proto Booster loadModel(string filename)
   Construct an xgboost Booster.
   */
PHP_METHOD(Booster, loadModel)
{
	char *filename;
	size_t filename_len;

	//https://cismon.net/2017/12/18/Fast-ZPP-s-Incompatibility-with-CPP/
	#undef IS_UNDEF
	#define IS_UNDEF Z_EXPECTED_LONG // Which is zero
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_PATH(filename, filename_len)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_NULL());
	#undef IS_UNDEF
	#define IS_UNDEF 0

	if (php_check_open_basedir(filename)) {
		RETURN_NULL();
	}

	booster_object * obj = XG_GET_THIS(booster_object);
	if (!check_xgboost_call(XGBoosterLoadModel(obj->handle, filename))) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array Booster::predict(DMatrix matrix, long ntree_limit)
   Return predictions on the rows of matrix as a php array of floats.
   */
PHP_METHOD(Booster, predict)
{
	zval * dmatrix_zval;
	long ntree_limit = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|l", &dmatrix_zval, dmatrix_ce, &ntree_limit) == FAILURE) {
 		RETURN_NULL();
	}

	booster_object * bst = XG_GET_THIS(booster_object);
	dmatrix_object * dmat = XG_ZVAL_P_TO_CUSTOM_STRUCT_P(dmatrix_object, dmatrix_zval);

 	bst_ulong res_len = 0;
	//Not in training mode
	int training = 0;
 	const float * res_arr;
 	int option_mask = 0; //The default
	if (!check_xgboost_call(XGBoosterPredict(bst->handle, dmat->handle, option_mask, ntree_limit, training, &res_len, &res_arr))) {
		RETURN_NULL();
	}

	array_init(return_value);
	for (bst_ulong i=0; i < res_len; i++) {
		add_index_double(return_value, i, res_arr[i]);
	}
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
