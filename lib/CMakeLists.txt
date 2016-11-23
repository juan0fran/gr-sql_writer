# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND sql_writer_sources
    sql_beacon_parser_impl.cc
)

set(sql_writer_sources "${sql_writer_sources}" PARENT_SCOPE)
if(NOT sql_writer_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT sql_writer_sources)

add_library(gnuradio-sql_writer SHARED ${sql_writer_sources})
target_link_libraries(gnuradio-sql_writer ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} ${MYSQL_LIBRARIES})
set_target_properties(gnuradio-sql_writer PROPERTIES DEFINE_SYMBOL "gnuradio_sql_writer_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-sql_writer PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-sql_writer
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_sql_writer_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sql_writer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_sql_writer.cc
)

add_executable(test-sql_writer ${test_sql_writer_sources})

target_link_libraries(
  test-sql_writer
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  ${MYSQL_LIBRARIES}
  gnuradio-sql_writer
)

GR_ADD_TEST(test_sql_writer test-sql_writer)
