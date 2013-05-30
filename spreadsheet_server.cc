/*
 * Spreadsheet server class
 * Date: April 23, 2013
 * Authors: John Clukey, Oscar Marshall, Jake Guckert, Josh Bell
 *
 * CS 3505 Final Project
 */

#include <boost/asio.hpp>
#include <sstream>
#include <iostream>
#include <sstream>
#include "spreadsheet_server.h"

/*
 * SpreadSheetServer constructor
 */
SpreadSheetServer::SpreadSheetServer(boost::asio::io_service& io_service, int port)
  : io_service_(io_service), 
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    spreadsheet_database_() {
  // Begin accepting connections
  start_accept();
  
}

/*
 * Accepts a new connection and begins listening.
 */
void SpreadSheetServer::start_accept() {
  std::cout << "Hello" << std::endl;
  Connection *new_connection = new Connection(io_service_, this);
  acceptor_.async_accept(new_connection->socket(),
			 boost::bind(&SpreadSheetServer::handle_accept, 
				     this, 
				     new_connection,
				     boost::asio::placeholders::error));
}

/*
 * Starts listening on the connection.
 */
void SpreadSheetServer::handle_accept(Connection *new_connection, 
				      const boost::system::error_code& error) {

  if (!error)
    {
      unjoined_clients_.insert(new_connection);
      std::cout << "New client created and inserted into unjoined_clients_" << std::endl;
      new_connection->start();
      
    }
  else
    {
      delete new_connection;
    }
  // Ready for a new connection.
  start_accept();
}

/*
 * Handles the CREATE command from a connection.
 */
void SpreadSheetServer::Create(Connection *connection, std::string filename,
			       std::string password) {
  // If the spreadsheet already exists, send fail message.	
  if (spreadsheet_database_.DoesSpreadSheetExist(filename)) {
    connection->SendMessage("CREATE FAIL\nName:" + 
			    filename + "\nThe spreadsheet already exists.\n");
    return;
  }
  // Or else create one and send the OK message.
  spreadsheet_database_.CreateSpreadSheet(filename, password);
  connection->SendMessage("CREATE OK\nName:" + filename +
			  "\nPassword:" + password + "\n");
}

/*
 * Handles the JOIN message from connection.
 */
void SpreadSheetServer::Join(Connection *connection, std::string filename, 
			     std::string password) {
  std::stringstream ss;
  // Join fail for non existant spreadsheet.
  if (!spreadsheet_database_.DoesSpreadSheetExist(filename)) {
    ss << "JOIN FAIL\nName:" << filename << "\nSpreadsheet does not exist.\n";
    connection->SendMessage(ss.str());
    return;
  }
  // Join fail for incorrect password.
  if (password.compare(spreadsheet_database_.GetSpreadSheetPassword(filename))
      != 0) {
    ss << "JOIN FAIL\nName:" << filename << "\nIncorrect password.\n";
    connection->SendMessage(ss.str());
    return;
  }
  // If we're here, the spreadsheet exists and the password is correct. Send the xml data.
  std::string xml = spreadsheet_database_.GetSpreadSheetXml(filename);
  int version = GetVersion(filename);
  ss << "JOIN OK\nName:" << filename << "\nVersion:" << version << "\nLength:";
  ss << xml.length() << "\n" << xml << "\n";
  // Add connection to spreadsheet
  connection->SendMessage(ss.str());
  // TODO: remove client from unjoined_clients_
  edit_sessions_[filename].clients.insert(connection);
}
 
/*
 * Handles CHANGE message from connection.
 */
void SpreadSheetServer::Change(Connection *connection, std::string filename, 
			       int version, std::string cell,
			       std::string contents) {
  std::stringstream ss;
  // If the client isn't in an editing session for that spreadsheet send CHANGE FAIL.
  if (!CheckName(connection, filename)) {
    ss << "CHANGE FAIL\nName:" << filename << "\nYou are not connected to that spreadsheet.\n";
    connection->SendMessage(ss.str());
    return;
  }
  // If client does not have current version send CHANGE WAIT.
  if (version != GetVersion(filename)) {
    ss << "CHANGE WAIT\nName:" << filename << "\nVersion:" << edit_sessions_[filename].version << "\n";
    connection->SendMessage(ss.str());
    return;
  }
  // Else send client CHANGE OK message.
  //edit_sessions_[filename].dones.push(spreadsheet_database_.ChangeCell(filename, cell, contents));
  ss << "CHANGE OK\nName:" << filename << "\nVersion:" << ++edit_sessions_[filename].version << "\n";
  std::string change_message = ss.str();
  connection->SendMessage(change_message);
  // Then iterate over all other clients in the editing session and send them the UPDATE message.
  ss.str("");
  ss << "UPDATE\nName:" << filename << "\nVersion:";
  ss << edit_sessions_[filename].version << "\nCell:" << cell << "\nLength:";
  ss << contents.length() << "\n" << contents << "\n";
  std::string message = ss.str();
  for (std::set<Connection*>::iterator it = edit_sessions_[filename].clients.begin(); it != edit_sessions_[filename].clients.end(); it++) {
    if (*it == connection)
      continue;
    (*it)->SendMessage(message);
  }
  edit_sessions_[filename].dones.push(spreadsheet_database_.ChangeCell(filename, cell, contents));
}
 
/*
 * Handles UNDO message from a connection.
 */
void SpreadSheetServer::Undo(Connection *connection, std::string filename, 
		       int version) {
  std::stringstream ss;
  // If the client isn't in an editing session for that spreadsheet send UNDO FAIL.	
  if (!CheckName(connection, filename)) {
    ss << "UNDO FAIL\nName:" << filename << "\nYou are not connected to this spreadsheet.\n";
    connection->SendMessage(ss.str());
    return;
  }
  // If the client does not have the current version, send UNDO WAIT.
  if (version != GetVersion(filename)) {
    ss << "UNDO WAIT\nName:" << filename << "\nVersion:" << GetVersion(filename) << "\n";
    connection->SendMessage(ss.str());
    return;
  }
  // If there are no actions in the undo stack, send UNDO END.
  if (edit_sessions_[filename].dones.size() == 0) {
    ss << "UNDO END\nName:" << filename << "\nVersion:" << GetVersion(filename) << "\n";
    connection->SendMessage(ss.str());
    return;
  }    
  // Or else get the done item.
  Done done = edit_sessions_[filename].dones.top();
  edit_sessions_[filename].dones.pop();
  // Change the database cell.
  //spreadsheet_database_.ChangeCell(filename, done.cell(),
  //done.old_contents());
  // Send the UNDO OK message to originating client.
  ss << "UNDO OK\nName:" << filename << "\nVersion:";
  ss << ++edit_sessions_[filename].version << "\nCell:" << done.cell(); 
  ss << "\nLength:" << done.old_contents().length() << "\n";
  ss << done.old_contents() << "\n";
  connection->SendMessage(ss.str());
  // Send UPDATE message to all other clients.
  ss.str("");
  ss << "UPDATE\nName:" << filename << "\nVersion:";
  ss << edit_sessions_[filename].version << "\nCell:" << done.cell() << "\nLength:";
  ss << done.old_contents().length() << "\n" << done.old_contents() << "\n";
  std::string message = ss.str();
  for (std::set<Connection*>::iterator it = edit_sessions_[filename].clients.begin(); it != edit_sessions_[filename].clients.end(); it++) {
    if (*it == connection)
      continue;
    (*it)->SendMessage(message);
  }
  spreadsheet_database_.ChangeCell(filename, done.cell(), done.old_contents());
}
 
/*
 * Handles the SAVE command from a connection.
 */
void SpreadSheetServer::Save(Connection *connection, std::string filename) {
  std::stringstream ss;
  // If the client is not in the editing session for the filename send SAVE FAIL message.
  if (!CheckName(connection, filename)) {
    ss << "SAVE FAIL\nName:" << filename << "\nYou are not connected to that spreadsheet.\n";
    connection->SendMessage(ss.str());
    return;
  }
  // Or else send SEND OK message and empty the undo stack. Changes always persist on the database
  // so no other action is necessary for saving.
  ss << "SAVE OK\nName:" << filename << "\n";
  connection->SendMessage(ss.str());
  edit_sessions_[filename].dones = std::stack<Done>();
}
  
/*
 * Handles the LEAVE command from a connection.
 */
void SpreadSheetServer::Leave(Connection *connection, std::string filename) {
  // Remove the client from the editing session.
  std::set<Connection*> clients = edit_sessions_[filename].clients;
  clients.erase(connection);
  // TODO: delete client?
  
  // If there are no more clients in the editing session, erase the whole editing session.
  // Changes always persist on the database so no other action is necessary for saving.
  if (clients.size() == 0) {
    edit_sessions_.erase(filename);
  }
}

/*
 * A helper function to get the current version number from an editing session.
 */
int SpreadSheetServer::GetVersion(std::string filename) {
  int version = 1;
  if (edit_sessions_.find(filename) != edit_sessions_.end()) {
    version = edit_sessions_[filename].version;
  }
  else {
    edit_sessions_[filename].version = version;
  }
  return version;
}

/*
 * A helper function to check if given connection is in an editing session for the given filename.
 */
bool SpreadSheetServer::CheckName(Connection *connection, std::string filename) {
  return  edit_sessions_[filename].clients.find(connection) != edit_sessions_[filename].clients.end();
}
