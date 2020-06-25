
#ifndef PHP_XGBOOST_H
#define PHP_XGBOOST_H

extern zend_module_entry xgboost_module_entry;
#define phpext_xgboost_ptr &xgboost_module_entry

#define PHP_XGBOOST_VERSION "0.2.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_XGBOOST_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_XGBOOST_API __attribute__ ((visibility("default")))
#else
#	define PHP_XGBOOST_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(xgboost);
PHP_MSHUTDOWN_FUNCTION(xgboost);
PHP_RINIT_FUNCTION(xgboost);
PHP_RSHUTDOWN_FUNCTION(xgboost);
PHP_MINFO_FUNCTION(xgboost);

PHP_FUNCTION(test_array);
PHP_FUNCTION(confirm_xgboost_compiled);	/* For testing, remove later. */

PHP_METHOD(DMatrix, __construct);
PHP_METHOD(DMatrix, getNumRow);
PHP_METHOD(DMatrix, getNumCol);

PHP_METHOD(Booster, getLastError);
PHP_METHOD(Booster, __construct);
PHP_METHOD(Booster, getAttr);
PHP_METHOD(Booster, setAttr);
PHP_METHOD(Booster, loadModel);
PHP_METHOD(Booster, predict);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(xgboost)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(xgboost)
*/

/* In every utility function you add that needs to use variables 
   in php_xgboost_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as XGBOOST_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define XGBOOST_G(v) TSRMG(xgboost_globals_id, zend_xgboost_globals *, v)
#else
#define XGBOOST_G(v) (xgboost_globals.v)
#endif

//Here are portability macros from php5 to php7
#if PHP_MAJOR_VERSION >= 7
#define XG_CUSTOM_STRUCT_P(cust_struct_name, obj) ((struct cust_struct_name *)((char*)obj - XtOffsetOf(struct cust_struct_name, std)))
#define XG_OBJ_FREE_GET_CUST_STRUCT_P(cust_struct_name, _obj) XG_CUSTOM_STRUCT_P(cust_struct_name, _obj)

#define XG_GET_THIS(cust_struct_name) XG_CUSTOM_STRUCT_P(cust_struct_name, Z_OBJ_P(getThis()))

#define XG_ZVAL_P_TO_CUSTOM_STRUCT_P(cust_struct_name, _zv) XG_CUSTOM_STRUCT_P(cust_struct_name, Z_OBJ_P(_zv))

#define XG_FOREACH_VAL(ht, val) ZEND_HASH_FOREACH_VAL(ht, val)
#define XG_FOREACH_END() ZEND_HASH_FOREACH_END()

#define XG_CREATE_HANDLER_RET_TYPE(_create_handler) zend_object *

#define XG_CREATE_HANDLER_INIT(cust_struct_name, _intern, class_entry)  _intern = (struct cust_struct_name *) ecalloc(1, XG_CUST_OBJ_SIZE(cust_struct_name, class_entry));

#define XG_DEFAULT_CREATE_OBJECT_HANDLER(cust_struct_name, _create_handler_name, _cust_object_handlers, _cust_free_storage)\
zend_object * _create_handler_name(zend_class_entry * class_entry TSRMLS_DC)\
{\
	cust_struct_name *intern = (cust_struct_name *)ecalloc(1,sizeof(cust_struct_name) +zend_object_properties_size(class_entry));\
         zend_object_std_init(&intern->std, class_entry TSRMLS_CC);\
         object_properties_init(&intern->std, class_entry);\
          _cust_object_handlers.offset = XtOffsetOf(struct cust_struct_name, std);\
         _cust_object_handlers.free_obj = _cust_free_storage;\
         intern->std.handlers = &_cust_object_handlers;\
         return &intern->std;\
}

#define XG_OBJ_FREE_STORAGE_ARG_TYPE zend_object *

#define XG_EXTRACT_DVAL_P(zv, _res) _res = zval_get_double(zv)

#define XG_CUST_OBJ_STRUCT_START() 

#define XG_CUST_OBJ_STRUCT_END() zend_object std;

#else
#define XG_OBJ_FREE_GET_CUST_STRUCT_P(cust_struct_name, _obj) (cust_struct_name *) _obj
#define XG_CUSTOM_STRUCT_P(cust_struct_name, obj) (cust_struct_name *) zend_object_store_get_object(obj TSRMLS_CC)
#define XG_ZVAL_P_TO_CUSTOM_STRUCT_P(cust_struct_name, _zv) XG_CUSTOM_STRUCT_P(cust_struct_name, _zv)
#define XG_GET_THIS(cust_struct_name) XG_CUSTOM_STRUCT_P(cust_struct_name, getThis())
//expects val to be zval*
#define XG_FOREACH_VAL(ht, val) do {\
	zval ** _entry; \
	HashPosition val##_pos; \
	zend_hash_internal_pointer_reset_ex(ht, &val##_pos); \
	for (; zend_hash_get_current_data_ex(ht, (void **)&_entry, &val##_pos) == SUCCESS; zend_hash_move_forward_ex(ht, &val##_pos)) { \
		val = *_entry;
	

#define XG_FOREACH_END()\
		} \
	} while(0)

#define XG_DEFAULT_CREATE_OBJECT_HANDLER(cust_struct_name, _create_handler_name, _cust_object_handlers, _cust_free_storage)\
zend_object_value _create_handler_name(zend_class_entry * class_entry TSRMLS_DC)\
{\
	zend_object_value retval;\
	cust_struct_name *obj = (cust_struct_name *)emalloc(sizeof(cust_struct_name));\
	memset(obj, 0, sizeof(cust_struct_name));\
	obj->std.ce = class_entry;\
	object_properties_init(&obj->std, class_entry);\
	retval.handle = zend_objects_store_put(obj, NULL,\
	_cust_free_storage, NULL TSRMLS_CC);\
	retval.handlers = &_cust_object_handlers;\
	return retval;\
}
	
#define XG_OBJ_FREE_STORAGE_ARG_TYPE void *

#define XG_EXTRACT_DVAL_P(zv, _res) do{\
		switch (Z_TYPE_P(zv))\
		{\
			case IS_LONG:\
				_res = (float)Z_LVAL_P(zv);\
				break;\
			case IS_DOUBLE:\
				_res = (float)Z_DVAL_P(zv);\
				break;\
			case IS_BOOL:\
				_res = (float)Z_BVAL_P(zv);\
				break;\
			case IS_STRING:\
				_res = atof(Z_STRVAL_P(zv));\
				break;\
			default:\
				break;\
		}\
	} while (0)

#define XG_CUST_OBJ_STRUCT_START() zend_object std;

#define XG_CUST_OBJ_STRUCT_END()

#endif

#endif	/* PHP_XGBOOST_H */

