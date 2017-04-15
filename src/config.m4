dnl $Id$
dnl config.m4 for extension xgboost

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(xgboost, for xgboost support,
dnl Make sure that the comment is aligned:
dnl [  --with-xgboost             Include xgboost support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(xgboost, whether to enable xgboost support,
[  --enable-xgboost           Enable xgboost support])

PHP_ARG_ENABLE(xgboost-debug, whether to enable debugging support in xgboost,
[  --enable-xgboost-debug        xgboost: Enable debugging support in xgboost], yes, yes)

if test "$PHP_XGBOOST" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-xgboost -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/xgboost.h"  # you most likely want to change this
  dnl if test -r $PHP_XGBOOST/$SEARCH_FOR; then # path given as parameter
  dnl   XGBOOST_DIR=$PHP_XGBOOST
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for xgboost files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       XGBOOST_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$XGBOOST_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the xgboost distribution])
  dnl fi

  dnl # --with-xgboost -> add include path
  dnl PHP_ADD_INCLUDE($XGBOOST_DIR/include)

  dnl # --with-xgboost -> check for lib and symbol presence
  dnl LIBNAME=xgboost # you may want to change this
  dnl LIBSYMBOL=xgboost # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $XGBOOST_DIR/lib, XGBOOST_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_XGBOOSTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong xgboost lib version or lib not found])
  dnl ],[
  dnl   -L$XGBOOST_DIR/lib -lm
  dnl ])
  dnl
  XGBOOST_SOURCE_DIR=../xgboost
  if test -z "$XGBOOST_INSTALL_DIR"; then
    XGBOOST_INSTALL_DIR=../$XGBOOST_SOURCE_DIR/lib
    AC_MSG_ERROR($XGBOOST_SOURCE_DIR)
  fi
  PHP_ADD_INCLUDE($XGBOOST_SOURCE_DIR/include)
  PHP_ADD_INCLUDE($XGBOOST_SOURCE_DIR/rabit/include)
  PHP_ADD_INCLUDE($XGBOOST_SOURCE_DIR/dmlc-core/include)
  PHP_ADD_LIBRARY_WITH_PATH(xgboost,$XGBOOST_INSTALL_DIR, XGBOOST_SHARED_LIBADD)
  PHP_SUBST(XGBOOST_SHARED_LIBADD)
  PHP_REQUIRE_CXX
  PHP_NEW_EXTENSION(xgboost, xgboost.cpp, $ext_shared)
fi
