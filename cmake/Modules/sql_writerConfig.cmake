INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SQL_WRITER sql_writer)

FIND_PATH(
    SQL_WRITER_INCLUDE_DIRS
    NAMES sql_writer/api.h
    HINTS $ENV{SQL_WRITER_DIR}/include
        ${PC_SQL_WRITER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SQL_WRITER_LIBRARIES
    NAMES gnuradio-sql_writer
    HINTS $ENV{SQL_WRITER_DIR}/lib
        ${PC_SQL_WRITER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQL_WRITER DEFAULT_MSG SQL_WRITER_LIBRARIES SQL_WRITER_INCLUDE_DIRS)
MARK_AS_ADVANCED(SQL_WRITER_LIBRARIES SQL_WRITER_INCLUDE_DIRS)

