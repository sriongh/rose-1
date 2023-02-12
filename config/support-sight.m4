##### http://www.rosecompiler.org
#
# SYNOPSIS
#
#   ROSE_SUPPORT_SIGHT([])
#
# DESCRIPTION
#
#   Determine if support is requested for the sight
#
#   author: Sriram Aananthakrishnan

AC_DEFUN([ROSE_SUPPORT_SIGHT],
[
  ROSE_CONFIGURE_SECTION([SIGHT])
  dnl --with-sight=<path>
  dnl  
  ROSE_ARG_WITH(
    [sight],
    [for the sight interface],
    [use the sight interface]
    [],
    []
  )
  if test "x$CONFIG_HAS_ROSE_WITH_SIGHT" != "xno"; then
    ROSE_WITH_SIGHT_INSTALL_PATH="$ROSE_WITH_SIGHT"
    sight_install=yes
  else
    ROSE_WITH_SIGHT_INSTALL_PATH=
    sight_install=no
  fi
  AC_SUBST(ROSE_WITH_SIGHT_INSTALL_PATH)
  AM_CONDITIONAL(ROSE_WITH_SIGHT, [test "x$sight_install" = "xyes"])
])
