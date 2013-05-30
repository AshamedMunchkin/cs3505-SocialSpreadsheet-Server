/*
 * Spreadsheet Server header file
 * Date: April 23, 2013
 * Authors: John Clukey, Oscar Marshall, Jake Guckert, Josh Bell
 *
 * CS 3505 Final Project
 */

#ifndef SPREADSHEET_SERVER_H_
#define SPREADSHEET_SERVER_H_

#include <string>
#include <map>
#include <set>
#include <stack>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "connection.h"
#include "spreadsheet_database.h"
#include "done.h"

class Connection;

// A struct to hold the variables for an editing session.
typedef struct {
  int version;
  std::stack<Done> dones;
  std::set<Connection*> clients;
} edit_session;

class SpreadSheetServer{

 public:
  SpreadSheetServer(boost::asio::io_service& io_service, int port);
  void Create(Connection *connection, std::string filename,
	      std::string password);
  void Join(Connection *connection, std::string filename, std::string password);
  void Change(Connection *connection, std::string filename, int version, 
	      std::string cell, std::string contents);
  void Undo(Connection *connection, std::string filename, int version);
  void Save(Connection *connection, std::string filename);
  void Leave(Connection *connection, std::string filename);

  friend class Connection;
  
 private:
  void start_accept();
  void handle_accept(Connection *new_connection, const boost::system::error_code& error);
  int GetVersion(std::string filename);
  bool CheckName(Connection *connection, std::string filename);
  
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::map<std::string, edit_session> edit_sessions_; // Keeps track of all editing sessions. The string is the filename for a spreadsheet.
  std::set<Connection*> unjoined_clients_; // holds clients yet to be added to a session.
  SpreadSheetDatabase spreadsheet_database_; // A class that administers the database connection.
  

};


#endif
