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

  AM_CONDITIONAL(ROSE_WITH_DBGLOG, [test "x$DBGLOG_INSTALL_PATH" != "xno"])
])