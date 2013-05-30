/*
 * Connection Class
 * Date: April 23, 2013
 * Authors: John Clukey, Josh Bell, Jake Gucker, Oscar Marshall
 *
 * CS 3505 Final Project
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include "connection.h"
#include "spreadsheet_server.h"

/*
 * Connection Constructor
 */
Connection::Connection(boost::asio::io_service& io_service, SpreadSheetServer *server) 
  : io_service_(io_service), socket_(io_service), server_(server) { 
}

/*
 * Connection destructor
 */
Connection::~Connection() { }

/*
 * Socket getter.
 */
boost::asio::ip::tcp::socket& Connection::socket() {
  
  return socket_;

}

/*
 * Starts listening on the socket for message from the client.
 */
void Connection::start() {
  
  boost::asio::async_read_until(socket_, 
				buffer_, 
				'\n', 
				boost::bind(&Connection::primary_handler,
					    this,
					    boost::asio::placeholders::error,
					    boost::asio::placeholders::bytes_transferred));
  
}

/*
 * Receives commands from the client, parses the information from the command,
 * and calls the server methods to handle them.
 */
void Connection::primary_handler(const boost::system::error_code& ec,
		     std::size_t bytes_transferred) {
  // Check for error message.
  if (!ec) {
    boost::system::error_code error;
    std::cout << "Reached primary handler" << std::endl;
    // Get a istream to handle the buffer.
    std::istream is(&buffer_);
    char cstr[bytes_transferred];
    // Get a line from the buffer.
    is.getline(cstr, bytes_transferred);
    std::string command(cstr);
    std::string name, password, cell, content;
    int version, length;
    
    // Trims and substrings (if the second argument is greater than 0);
    command = process_string(command, 0);

    std::cout << command << ": Length: " << command.length() << std::endl;
    // Find out what the first line of the command is.
    // Then we can read synchronously since we know the structure of the message.
    if (command.compare("CREATE") == 0) {
      std::cout << "You typed CREATE" << std::endl;
      // Get the next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char name_cstr[bytes_transferred];
      is.getline(name_cstr, bytes_transferred);
      name = name_cstr;
      name = process_string(name ,5);
      std::cout << "Name is " << name << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char password_cstr[bytes_transferred];
      is.getline(password_cstr, bytes_transferred);
      password = password_cstr;
      password = process_string(password ,9);
      std::cout << "Password is " << password << std::endl;
      // Now send call the server's create method.
      server_->Create(this, name, password);
    }
    else if (command.compare("JOIN") == 0) {
      std::cout << "You typed JOIN" << std::endl;
      // Get the next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      if (!error) {
	char name_cstr[bytes_transferred];
	is.getline(name_cstr, bytes_transferred);
	name = name_cstr;
	name = process_string(name, 5);
	std::cout << "Name is " << name << std::endl;
      }
      // Next line
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char password_cstr[bytes_transferred];
      is.getline(password_cstr, bytes_transferred);
      std::string password(password_cstr);
      password = process_string(password, 9);
      std::cout << "Password is " << password << std::endl;
      // Call the server's join method.
      server_->Join(this, name, password);
    }
    else if (command.compare("CHANGE") == 0) {
      std::cout << "You typed CHANGE" << std::endl;
      // Next line
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      if (!error) {
	char name_cstr[bytes_transferred];
	is.getline(name_cstr, bytes_transferred);
	name = name_cstr;
	name = process_string(name ,5);
	std::cout << "Name is " << name << std::endl;
      }
      // Next line
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      if (error){;}
      char version_cstr[bytes_transferred];
      is.getline(version_cstr, bytes_transferred);
      version = atoi(version_cstr + 8);
      std::cout << "Version is " << version << std::endl;
      //Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char cell_cstr[bytes_transferred];
      is.getline(cell_cstr, bytes_transferred);
      cell = cell_cstr;
      cell = process_string(cell, 5);
      std::cout << "Cell is " << cell << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char length_cstr[bytes_transferred];
      is.getline(length_cstr, bytes_transferred);
      length = atoi(length_cstr + 7);
      std::cout << "Length is " << length << std::endl;      
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char content_cstr[bytes_transferred];
      is.getline(content_cstr, bytes_transferred);
      content = content_cstr;
      content = process_string(content, 0);
      std::cout << "Content is " << content << " content length is "<< content.length() <<  std::endl;
      // Call the server's change method.
      server_->Change(this, name, version, cell, content);
    }
    else if (command.compare("UNDO") == 0) {
      std::cout << "You typed UNDO" << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char name_cstr[bytes_transferred];
      is.getline(name_cstr, bytes_transferred);
      name = name_cstr;
      name = process_string(name ,5);
      std::cout << "Name is " << name << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char version_cstr[bytes_transferred];
      is.getline(version_cstr, bytes_transferred);
      version = atoi(version_cstr + 8);
      std::cout << "Version is " << version << std::endl;
      // Call the server's undo method.
      server_->Undo(this, name, version);
    }
    else if (command.compare("SAVE") == 0) {
      std::cout << "You typed SAVE" << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char name_cstr[bytes_transferred];
      is.getline(name_cstr, bytes_transferred);
      name = name_cstr;
      name = process_string(name ,5);
      std::cout << "Name is " << name << std::endl;      
      // Call the server's save method.
      server_->Save(this, name);
    }
    else if (command.compare("LEAVE") == 0) {
      std::cout << "You typed LEAVE" << std::endl;
      // Next line.
      bytes_transferred = read_until(socket_, buffer_, '\n', error);
      char name_cstr[bytes_transferred];
      is.getline(name_cstr, bytes_transferred);
      name = name_cstr;
      name = process_string(name ,5);
      std::cout << "Name is " << name << std::endl;
      // Call the server's leave method.
      server_->Leave(this, name);
    }
    else {
      // If something else was received, send the Error message.
      SendMessage("ERROR\n");
    }
    // And begin listening asynchronously again.
    start();
  }
  else {
    // If error code was set, close the socket.
    socket_.close();
  }
}

/*
 * A helper method for processing strings.
 * Trims the string then returns a substring starting at character index
 * `n' until the end of the string.
 */
std::string Connection::process_string(std::string s, int n) {
  // Trims in case there was a newline or carriage return or whitespace.
  boost::trim(s);
  return s.substr(n, s.length());
}

/*
 * Sends a message to the client.
 */
void Connection::SendMessage(std::string message) {

  std::ostream outstream(&out_buf_);
  outstream << message;
  std::cout << message;
  boost::asio::async_write(socket_, out_buf_,
			   boost::bind(&Connection::WriteHandler,
				       this,
				       boost::asio::placeholders::error,
				       boost::asio::placeholders::bytes_transferred));
}

/*
 * This is here only because it is required for the async_write method. It doesn't do anything.
 */
void Connection::WriteHandler(const boost::system::error_code& error, std::size_t bytes_transferred) {

}
