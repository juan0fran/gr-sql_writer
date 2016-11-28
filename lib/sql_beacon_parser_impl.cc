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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

#include <gnuradio/io_signature.h>
#include "sql_beacon_parser_impl.h"

#include <stdint.h>

#define DB_HOST         "localhost"
#define DB_DBNAME       "nslsbp"
#define DB_TABLE_HK     "hk_data"

#define DB_USR          "carles"
#define DB_PASSWD       "96452000"

typedef struct _ambient_sensors{
  float cpu_temp;
  float gpu_temp;
  float in_temp;
  float in_pressure;
  float in_calc_alt;
  float out_temp;
  float out_pressure;
  float out_calc_alt;
}_ambient_sensors;

typedef struct _motion_sensors{
  float acc_x;
  float acc_y;
  float acc_z;

  float gyro_x;
  float gyro_y;
  float gyro_z;

  float mag_x;
  float mag_y;
  float mag_z;
}_motion_sensors;

typedef struct _gps_data{ 
  uint32_t time_local;
  uint32_t time_gps;
  float lat;
  float lng;
  float gspeed; /* this is ground speed in mm/second?? */
  float sea_alt;
  float geo_alt;
}_gps_data;

typedef struct HKData {
  _gps_data gps;
  _motion_sensors mot;
  _ambient_sensors amb;
} HKData;

template <typename T> std::string tostr(const T& t) { 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 

namespace gr {
  namespace sql_writer {

    sql_beacon_parser::sptr
    sql_beacon_parser::make(std::string user, std::string password, std::string database, std::string table, std::string host, unsigned int port)
    {
      return gnuradio::get_initial_sptr
        (new sql_beacon_parser_impl(user, password, database, table, host, port));
    }

    /*
     * The private constructor
     */
    sql_beacon_parser_impl::sql_beacon_parser_impl(std::string user, std::string password, std::string database, std::string table, std::string host, unsigned int port)
      : gr::block("sql_beacon_parser",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0))
    {

      d_table = table;

      d_mysql = NULL;
      d_mysql = mysql_init(d_mysql);

      if(!d_mysql) throw std::runtime_error("Initialize mysql instance failed.");

      // Connect to Mysql server
      if(!mysql_real_connect(d_mysql,      /* MYSQL structure to use */
                         host.c_str(),     /* server hostname or IP address */
                         user.c_str(),     /* mysql user */
                         password.c_str(), /* password */
                         database.c_str(), /* default database to use, NULL for none */
                         port,                /* port number, 0 for default */
                         NULL,             /* socket file or named pipe name */
                         CLIENT_FOUND_ROWS /* connection flags */ )){
          std::string error_msg = "[gr-sql] Connection to MySQL server failed: ";
          error_msg.append(mysql_error(d_mysql));
          throw std::runtime_error(error_msg.c_str());
      }
      else{
          std::cout << "[gr-sql] Connection to MySQL server established." << std::endl;
          std::cout << "[gr-sql] -> Selected user: " << user << std::endl;
          std::cout << "[gr-sql] -> Selected database: " << database << std::endl;
          std::cout << "[gr-sql] -> MySQL Server Info: " << mysql_get_server_info(d_mysql) << std::endl;
          std::cout << "[gr-sql] -> MySQL Client Info: " << mysql_get_client_info() << std::endl;
      }

      d_symbols.push_back(std::string("time_local"));
      d_symbols.push_back(std::string("time_gps"));

      d_symbols.push_back(std::string("lat"));
      d_symbols.push_back(std::string("lng"));
      d_symbols.push_back(std::string("gspeed"));
      d_symbols.push_back(std::string("sea_alt"));
      d_symbols.push_back(std::string("geo_alt"));

      d_symbols.push_back(std::string("acc_x"));
      d_symbols.push_back(std::string("acc_y"));
      d_symbols.push_back(std::string("acc_z"));

      d_symbols.push_back(std::string("gyro_x"));
      d_symbols.push_back(std::string("gyro_y"));
      d_symbols.push_back(std::string("gyro_z"));

      d_symbols.push_back(std::string("mag_x"));
      d_symbols.push_back(std::string("mag_y"));
      d_symbols.push_back(std::string("mag_z"));

      d_symbols.push_back(std::string("cpu_temp"));
      d_symbols.push_back(std::string("gpu_temp"));
      
      d_symbols.push_back(std::string("in_temp"));
      d_symbols.push_back(std::string("in_pressure"));
      d_symbols.push_back(std::string("in_calc_alt"));

      d_symbols.push_back(std::string("out_temp"));
      d_symbols.push_back(std::string("out_pressure"));
      d_symbols.push_back(std::string("out_calc_alt"));

      for(int k = 0; k < d_symbols.size(); k++){
        if(k < d_symbols.size() - 1){
          d_str_ids.append(d_symbols[k]);
          d_str_ids.append(",");
        }
        else{
          d_str_ids.append(d_symbols[k]);
        }
      }

      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),
       boost::bind(&sql_beacon_parser_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    sql_beacon_parser_impl::~sql_beacon_parser_impl()
    {
      mysql_close(d_mysql);
    }

    void
    sql_beacon_parser_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    sql_beacon_parser_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      return 0;
    }
    void
    sql_beacon_parser_impl::handle_msg(pmt::pmt_t pmt_msg)
    {
      pmt::pmt_t msg = pmt::cdr(pmt_msg);
      unsigned int msg_size = pmt::length(msg);
      uint8_t data[msg_size];
      size_t offset(0);
      std::string str_values;
      std::vector<std::string> values;
      HKData hk;
      memcpy(data, pmt::uniform_vector_elements(msg, offset), sizeof(data));
      /* Check if the first byte corresponds to the length of the data and to the sizeof(HKData) */
      if (( (int)data[0] == (msg_size - 1) ) && ( (int)data[0] == sizeof(HKData) )){
        /* in case of that, the message is HKData struct sized!!! Heureka */
        /* now just put into ''std::vector<std::string> values'' the string corresponding to the data */
        memcpy(&hk, data+1, sizeof(HKData));
        if (hk.gps.time_local >= 2147483647){
            hk.gps.time_local = 0;
        }
        if (hk.gps.time_gps >= 2147483647){
            hk.gps.time_gps = 0;
        }
        str_values.append(tostr(hk.gps.time_local)); str_values.append(",");
        str_values.append(tostr(hk.gps.time_gps)); str_values.append(",");
        str_values.append(tostr(hk.gps.lat)); str_values.append(",");
        str_values.append(tostr(hk.gps.lng)); str_values.append(",");
        str_values.append(tostr(hk.gps.gspeed)); str_values.append(",");
        str_values.append(tostr(hk.gps.sea_alt)); str_values.append(",");
        str_values.append(tostr(hk.gps.geo_alt)); str_values.append(",");
        str_values.append(tostr(hk.mot.acc_x)); str_values.append(",");
        str_values.append(tostr(hk.mot.acc_y)); str_values.append(",");
        str_values.append(tostr(hk.mot.acc_z)); str_values.append(",");
        str_values.append(tostr(hk.mot.gyro_x)); str_values.append(",");
        str_values.append(tostr(hk.mot.gyro_y)); str_values.append(",");
        str_values.append(tostr(hk.mot.gyro_z)); str_values.append(",");
        str_values.append(tostr(hk.mot.mag_x)); str_values.append(",");
        str_values.append(tostr(hk.mot.mag_y)); str_values.append(",");
        str_values.append(tostr(hk.mot.mag_z)); str_values.append(",");
        str_values.append(tostr(hk.amb.cpu_temp)); str_values.append(",");
        str_values.append(tostr(hk.amb.gpu_temp)); str_values.append(",");
        str_values.append(tostr(hk.amb.in_temp)); str_values.append(",");
        str_values.append(tostr(hk.amb.in_pressure)); str_values.append(",");
        str_values.append(tostr(hk.amb.in_calc_alt)); str_values.append(",");
        str_values.append(tostr(hk.amb.out_temp)); str_values.append(",");
        str_values.append(tostr(hk.amb.out_pressure)); str_values.append(",");
        str_values.append(tostr(hk.amb.out_calc_alt));

      }else{
        return;
      }
      // Send msg to SQL database
      MYSQL_STMT* stmt = mysql_stmt_init(d_mysql);
      std::string query = "INSERT INTO ";
      query.append(d_table);
      query.append(" (");
      query.append(d_str_ids);
      query.append(") VALUES (");
      query.append(str_values);
      query.append(");");
      if(mysql_stmt_prepare(stmt, query.c_str(), query.size())){
          std::string error_msg = "SQL query init failed: ";
          error_msg.append(mysql_stmt_error(stmt));
          error_msg.append(" : ");
          error_msg.append(query);
          std::printf("error at MYSQL Query: %s\n", error_msg.c_str());
      }
      if(mysql_stmt_execute(stmt)){
          std::string error_msg = "SQL query execution failed: ";
          error_msg.append(mysql_stmt_error(stmt));
          error_msg.append(" : ");
          error_msg.append(query);
          std::printf("error at MYSQL Query: %s\n", error_msg.c_str());
      }
      mysql_stmt_close(stmt);
      
    }
  } /* namespace sql_writer */
} /* namespace gr */

