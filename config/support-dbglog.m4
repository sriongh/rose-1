##### http://www.rosecompiler.org
#
# SYNOPSIS
#
#   ROSE_SUPPORT_DBGLOG([])
#
# DESCRIPTION
#
#   Determine if support is requested for the dbglog
#
#   author: Sriram Aananthakrishnan

AC_DEFUN([ROSE_SUPPORT_DBGLOG],
[
  ROSE_CONFIGURE_SECTION([DBGLOG])
  dnl --with-dbglog=<path>
  dnl  
  ROSE_ARG_WITH(
    [dbglog],
    [for the dbglog interface],
    [use the dbglog interface]
    [],
    []
  )
  if test "x$CONFIG_HAS_ROSE_WITH_DBGLOG" != "xno"; then
    DBGLOG_INSTALL_PATH="$ROSE_WITH_DBGLOG"    
    DBGLOG_INCLUDE_PATH="$ROSE_WITH_DBGLOG"
    DBGLOG_LIBRARY_PATH="$ROSE_WITH_DBGLOG"
  else
    DBGLOG_INSTALL_PATH=
    DBGLOG_INCLUDE_PATH=
    DBGLOG_LIBRARY_PATH=
  fi

  if test "x$DBGLOG_INSTALL_PATH" != "x"; then
    dbglog_include_path=-I"$DBGLOG_INSTALL_PATH"
    dbglog_libs_path=-L"$DBGLOG_INSTALL_PATH"
    dbglog_libs=-ldbglog
  else
    dbglog_include_path=
    dbglog_libs_path=
    dbglog_libs=
  fi
  
  if test "x$DBGLOG_INSTALL_PATH" != "x"; then
    ROSE_WITH_DBGLOG_CPPFLAGS="$dbglog_include_path $dbglog_libs_path"
    ROSE_WITH_DBGLOG_LDFLAGS="$dbglog_libs"
    #AC_MSG_RESULT([$ROSE_WITH_DBGLOG_CPPFLAGS])
    OLD_CPPFLAGS=$CPPFLAGS
    OLD_LIBS=$LIBS
    CPPFLAGS="$OLD_CPPFLAGS $ROSE_WITH_DBGLOG_CPPFLAGS"
    LIBS=$ROSE_WITH_DBGLOG_LDFLAGS
    AC_LANG(C++)
    dnl SA 10/6/2013
    dnl AC_TRY_LINK is deprecated ??
    dnl if so should consider using AC_LINK_IFELSE instead
    AC_TRY_LINK([#include <dbglog.h>],
                          [dbglog::dbg << "hello\n";],
                          [ 
                              have_dbglog=yes
                              AC_MSG_NOTICE([dbglog library found])],
                          [
                              ROSE_MSG_ERROR([cannot detect dbglog library])
                              ROSE_WITH_DBGLOG_CPPFLAGS=""
                              ROSE_WITH_DBGLOG_LDFLAGS=""
                              have_dbglog=])
  fi

  LIBS=$OLD_LIBS
  CPPFLAGS=$OLD_CPPFLAGS

  AC_SUBST(ROSE_WITH_DBGLOG_CPPFLAGS)
  AC_SUBST(ROSE_WITH_DBGLOG_LDFLAGS)
  AM_CONDITIONAL(ROSE_WITH_DBGLOG, [test "x$DBGLOG_INSTALL_PATH" != "xno"])
])