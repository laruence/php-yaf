 /*
  +----------------------------------------------------------------------+
  | Yet Another Framework                                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Xinchen Hui  <laruence@php.net>                              |
  +----------------------------------------------------------------------+
*/

/* $Id: supervar.c 326905 2012-07-31 15:18:27Z laruence $ */

#define YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR "_var_name"

zend_class_entry *yaf_route_supervar_ce;

/** {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(yaf_route_supervar_construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, supervar_name)
ZEND_END_ARG_INFO()
/* }}} */

/** {{{ int yaf_route_supervar_route(yaf_route_t *route, yaf_request_t *request TSRMLS_DC)
 */
int yaf_route_supervar_route(yaf_route_t *route, yaf_request_t *request TSRMLS_DC) {
	zval *varname, *zuri;
	char *req_uri;

	varname = zend_read_property(yaf_route_supervar_ce, route, ZEND_STRL(YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR), 1 TSRMLS_CC);

	zuri = yaf_request_query(YAF_GLOBAL_VARS_GET, Z_STRVAL_P(varname), Z_STRLEN_P(varname) TSRMLS_CC);

	if (!zuri || ZVAL_IS_NULL(zuri)) {
		return 0;
	}

	req_uri = estrndup(Z_STRVAL_P(zuri), Z_STRLEN_P(zuri));
    yaf_route_pathinfo_route(request, req_uri, Z_STRLEN_P(zuri) TSRMLS_CC);
	efree(req_uri);
	return 1;
}
/* }}} */

/** {{{ yaf_route_t * yaf_route_supervar_instance(yaf_route_t *this_ptr, zval *name TSRMLS_DC)
 */
yaf_route_t * yaf_route_supervar_instance(yaf_route_t *this_ptr, zval *name TSRMLS_DC) {
	yaf_route_t *instance;

	if (!name || IS_STRING != Z_TYPE_P(name) || !Z_STRLEN_P(name)) {
		return NULL;
	}

	if (this_ptr) {
		instance  = this_ptr;
	} else {
		MAKE_STD_ZVAL(instance);
		object_init_ex(instance, yaf_route_supervar_ce);
	}

	zend_update_property(yaf_route_supervar_ce, instance, ZEND_STRL(YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR), name TSRMLS_CC);

	return instance;
}
/* }}} */

/** {{{ zval * yaf_route_supervar_assemble(zval *mvc, zval *query TSRMLS_DC)
 */
zval * yaf_route_supervar_assemble(yaf_route_t *this_ptr, zval *mvc, zval *query TSRMLS_DC) {
	char tvalue[1024];
	uint tvalue_len = 0;
	zval *pname;
	zval *uri;

	MAKE_STD_ZVAL(uri);

	pname		= zend_read_property(yaf_route_supervar_ce, this_ptr, ZEND_STRL(YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR), 1 TSRMLS_CC);

	do {
		zval **tmp;
		char tsprintf[1024];
		uint tlen;
	
		tlen = strlen("?=") + Z_STRLEN_P(pname);
		tlen = snprintf(tsprintf, tlen + 1, "?%s=", Z_STRVAL_P(pname));

		if (tlen) {
			memcpy(&tvalue[tvalue_len], tsprintf, strlen(tsprintf));
			tvalue_len += tlen;
		}

		if (zend_hash_find(Z_ARRVAL_P(mvc), ZEND_STRS(YAF_ROUTE_VAR_NAME_MODULE), (void **)&tmp) == SUCCESS) {
			tlen = strlen("/") + Z_STRLEN_PP(tmp);
			tlen = snprintf(tsprintf, tlen + 1, "/%s", Z_STRVAL_PP(tmp));
			if (tlen) {
				memcpy(&tvalue[tvalue_len], tsprintf, strlen(tsprintf));
				tvalue_len += tlen;
			}
		}

		if (zend_hash_find(Z_ARRVAL_P(mvc), ZEND_STRS(YAF_ROUTE_VAR_NAME_CONTROLLER), (void **)&tmp) == FAILURE) {
			yaf_trigger_error(YAF_ERR_TYPE_ERROR TSRMLS_CC, "%s", "You need to specify the controller");
			break;
		}

		tlen = strlen("/") + Z_STRLEN_PP(tmp);
		tlen = snprintf(tsprintf, tlen + 1, "/%s", Z_STRVAL_PP(tmp));
		if (tlen) {
			memcpy(&tvalue[tvalue_len], tsprintf, strlen(tsprintf));
			tvalue_len += tlen;
		}

		if(zend_hash_find(Z_ARRVAL_P(mvc), ZEND_STRS(YAF_ROUTE_VAR_NAME_ACTION), (void **)&tmp) == FAILURE) {
			yaf_trigger_error(YAF_ERR_TYPE_ERROR TSRMLS_CC, "%s", "You need to specify the action");
			break;
		}

		tlen = strlen("/") + Z_STRLEN_PP(tmp);
		tlen = snprintf(tsprintf, tlen + 1, "/%s", Z_STRVAL_PP(tmp));
		if (tlen) {
			memcpy(&tvalue[tvalue_len], tsprintf, strlen(tsprintf));
			tvalue_len += tlen;
		}

		if ( IS_ARRAY == Z_TYPE_P(query)) {
                        uint key_type, key_len;
                        char *key;
                        ulong key_idx;

			for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(query));
				zend_hash_get_current_data(Z_ARRVAL_P(query), (void **)&tmp) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(query))) {

				if (IS_STRING == Z_TYPE_PP(tmp)
						&& HASH_KEY_IS_STRING == zend_hash_get_current_key_ex(Z_ARRVAL_P(query), &key, &key_len, &key_idx, 0, NULL)) {

					tlen = strlen("=&") + key_len + Z_STRLEN_PP(tmp);
					tlen = snprintf(tsprintf, tlen + 1, "&%s=%s", key, Z_STRVAL_PP(tmp));
					if (tlen) {
						memcpy(&tvalue[tvalue_len], tsprintf, strlen(tsprintf));
						tvalue_len += tlen;
					}
				}
			}
		}

		tvalue[tvalue_len] = '\0';
		ZVAL_STRING(uri, tvalue, 0);
		return uri;
	} while (0);

	ZVAL_NULL(uri);
	return uri;
}
/* }}} */

/** {{{ proto public Yaf_Route_Supervar::route(string $uri)
 */
PHP_METHOD(yaf_route_supervar, route) {
	yaf_request_t *request;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &request) == FAILURE) {
		return;
	} else {
		RETURN_BOOL(yaf_route_supervar_route(getThis(), request TSRMLS_CC));
	}
}
/** }}} */

/** {{{ proto public Yaf_Route_Supervar::__construct(string $varname)
 */
PHP_METHOD(yaf_route_supervar, __construct) {
	zval *var;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &var) ==   FAILURE) {
		return;
	}

	if (Z_TYPE_P(var) != IS_STRING || !Z_STRLEN_P(var)) {
		yaf_trigger_error(YAF_ERR_TYPE_ERROR TSRMLS_CC, "Expects a string super var name", yaf_route_supervar_ce->name);
		RETURN_FALSE;
	}

	zend_update_property(yaf_route_supervar_ce, getThis(), ZEND_STRL(YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR), var TSRMLS_CC);
}
/** }}} */

/** {{{ proto public Yaf_Route_Supervar::assemble(array $mvc[, array $query = NULL])
*/
PHP_METHOD(yaf_route_supervar, assemble) {
	zval *mvc, *query;
	zval *return_uri;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &mvc, &query) == FAILURE) {
		return;
	} else {
		return_uri = yaf_route_supervar_assemble(getThis(), mvc, query TSRMLS_CC);
		RETURN_ZVAL(return_uri, 1, 0);
	}

}
/* }}} */

/** {{{ yaf_route_supervar_methods
 */
zend_function_entry yaf_route_supervar_methods[] = {
	PHP_ME(yaf_route_supervar, __construct, yaf_route_supervar_construct_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(yaf_route_supervar, route, yaf_route_route_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(yaf_route_supervar, assemble, yaf_route_assemble_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/** {{{ YAF_STARTUP_FUNCTION
 */
YAF_STARTUP_FUNCTION(route_supervar) {
	zend_class_entry ce;
	YAF_INIT_CLASS_ENTRY(ce, "Yaf_Route_Supervar", "Yaf\\Route\\Supervar", yaf_route_supervar_methods);
	yaf_route_supervar_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_class_implements(yaf_route_supervar_ce TSRMLS_CC, 1, yaf_route_ce);
	yaf_route_supervar_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_declare_property_null(yaf_route_supervar_ce, ZEND_STRL(YAF_ROUTE_SUPERVAR_PROPETY_NAME_VAR),  ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
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

