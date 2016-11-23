/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_SQL_WRITER_SQL_BEACON_PARSER_IMPL_H
#define INCLUDED_SQL_WRITER_SQL_BEACON_PARSER_IMPL_H

#include <sql_writer/sql_beacon_parser.h>
#include <mysql.h>

namespace gr {
  namespace sql_writer {

    class sql_beacon_parser_impl : public sql_beacon_parser
    {
     private:
      MYSQL *d_mysql;
      std::string d_table;
      std::vector<std::string> d_symbols;
      std::string d_str_ids;
     public:
      sql_beacon_parser_impl(std::string user, std::string password, std::string database, std::string table, std::string host, unsigned int port);
      ~sql_beacon_parser_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

      void handle_msg(pmt::pmt_t pmt_msg);

    };

  } // namespace sql_writer
} // namespace gr

#endif /* INCLUDED_SQL_WRITER_SQL_BEACON_PARSER_IMPL_H */

