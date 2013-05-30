/*
 * Main program for our spreadsheet server
 * Date: April 23, 2013
 * Authors: John Clukey, Josh Bell, Jake Guckert, Oscar Marshall
 *
 * CS 3505 Final Project
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <string>
#include <iostream>
#include "spreadsheet_server.h"


int main (int argc, char *argv[]) {

  // Check if a port was specified.
  if (argc != 2)
    {
      std::cout << "Command line argument should be port number" << std::endl;
      exit(1);
    }
  boost::asio::io_service io_service;
  // Run the server.
  SpreadSheetServer server(io_service, atoi(argv[1])); 
  
  io_service.run();
 
  return 0;
}

