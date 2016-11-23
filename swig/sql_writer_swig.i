/* -*- c++ -*- */

#define SQL_WRITER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sql_writer_swig_doc.i"

%{
#include "sql_writer/sql_beacon_parser.h"
%}


%include "sql_writer/sql_beacon_parser.h"
GR_SWIG_BLOCK_MAGIC2(sql_writer, sql_beacon_parser);
