AC_INIT([video_effects], [2025], [alexander.ziegler@st.oth-regensburg.de])

AC_CONFIG_SRCDIR([src/main.c])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CC

AC_CHECK_HEADER([argp.h], [], [
    AC_MSG_ERROR([argp.h header not found. On macOS, install with: brew install argp-standalone])
])

AC_CANONICAL_HOST
case "${host_os}" in
    darwin*)
        LIBS="$LIBS -largp"
        ;;
esac

PKG_CHECK_MODULES([FFMPEG], [libavcodec libavformat libavutil libswscale], [], [
    AC_MSG_ERROR([FFmpeg libraries are required but were not found.])
])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
