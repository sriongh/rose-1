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
    # SIGHT_INCLUDE_PATH="$ROSE_WITH_SIGHT"
    # SIGHT_LIBRARY_PATH="$ROSE_WITH_SIGHT"
  else
    ROSE_WITH_SIGHT_INSTALL_PATH=
    # SIGHT_INCLUDE_PATH=
    # SIGHT_LIBRARY_PATH=
  fi

  # if test "x$SIGHT_INSTALL_PATH" != "x"; then
  #   sight_include_path=-I"$SIGHT_INSTALL_PATH"
  #   sight_libs_path=-L"$SIGHT_INSTALL_PATH"
  #   sight_libs=-lsight_structure
  #   sight_layout_libs=-Wl,--whole-archive,-lsight_layout,-no-whole-archive
  # else
  #   sight_include_path=
  #   sight_libs_path=
  #   sight_libs=
  #   sight_layout_libs=
  # fi
  
  # if test "x$SIGHT_INSTALL_PATH" != "x"; then
  #   ROSE_WITH_SIGHT_CPPFLAGS="$sight_include_path $sight_libs_path"
  #   ROSE_WITH_SIGHT_LDFLAGS="$sight_libs"
  #   ROSE_WITH_SIGHT_LAYOUT_LDFLAGS="$sight_layout_libs"
  #   AC_MSG_RESULT([$ROSE_WITH_SIGHT_CPPFLAGS])
  #   AC_MSG_RESULT([$ROSE_WITH_SIGHT_LDFLAGS])
  #   AC_MSG_RESULT([$ROSE_WITH_SIGHT_LAYOUT_LDFLAGS])
  #   OLD_CPPFLAGS=$CPPFLAGS
  #   OLD_LIBS=$LIBS
  #   CPPFLAGS="$OLD_CPPFLAGS $ROSE_WITH_SIGHT_CPPFLAGS"
  #   LIBS=$ROSE_WITH_SIGHT_LDFLAGS
  #   AC_MSG_RESULT([$CPPFLAGS])
  #   AC_MSG_RESULT([$LIBS])
  #   AC_LANG(C++)
    dnl SA 10/6/2013
    dnl AC_TRY_LINK is deprecated ??
    dnl if so should consider using AC_LINK_IFELSE instead
  #   AC_TRY_LINK([#include <sight.h>],
  #                         [dbg << "hello\n";],
  #                         [ 
  #                             have_sight=yes
  #                             AC_MSG_NOTICE([sight library found])],
  #                         [
  #                             ROSE_MSG_ERROR([cannot detect sight library])
  #                             ROSE_WITH_SIGHT_CPPFLAGS=""
  #                             ROSE_WITH_SIGHT_LDFLAGS=""
  #                             ROSE_WITH_SIGHT_LAYOUT_LDFLAGS=""
  #                             have_sight=])
  # fi

  # LIBS=$OLD_LIBS
  # CPPFLAGS=$OLD_CPPFLAGS

  # AC_SUBST(ROSE_WITH_SIGHT_CPPFLAGS)
  # AC_SUBST(ROSE_WITH_SIGHT_LDFLAGS)
  # AC_SUBST(ROSE_WITH_SIGHT_LAYOUT_LDFLAGS)
  AC_SUBST(ROSE_WITH_SIGHT_INSTALL_PATH)
  AM_CONDITIONAL(ROSE_WITH_SIGHT, [test "x$SIGHT_INSTALL_PATH" != "xno"])
])
