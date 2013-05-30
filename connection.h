/*
 * Models a client connection.
 * Date: April 23, 2013
 * Authors: John Clukey, Oscar Marshall, Jake Guckert, Josh Bell
 *
 * CS 3505 Final Project
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>
#include <iostream>
#include "spreadsheet_server.h"

class SpreadSheetServer;

class Connection{

 public:
  Connection(boost::asio::io_service& io_service, SpreadSheetServer *server);
  ~Connection();
  void start();
  void SendMessage(std::string message);
  boost::asio::ip::tcp::socket& socket();

 private:
 
  void SendError();
  void Written(const boost::system::error_code& ec, std::size_t bytes_transferred);

  void primary_handler(const boost::system::error_code& ec, std::size_t bytes_transferred);

  std::string process_string(std::string s, int n);

  void WriteHandler(const boost::system::error_code& error, std::size_t bytes_transferred);
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::streambuf buffer_; // A buffer for receiving messages
  boost::asio::streambuf out_buf_; // A buffer for sending them.
  SpreadSheetServer *server_; // The server this client is on. Tracked so that we can call its methods.

};

#endif
