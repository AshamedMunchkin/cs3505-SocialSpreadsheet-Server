/*
 * Spreadsheet Database handler
 * Date: April 23, 2013
 * Authors: Oscar Marshall
 *
 * CS 3505 Final Project
 */

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "mysql_connection.h"
#include "mysql_driver.h"
#include "cppconn/driver.h"
#include "cppconn/connection.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/exception.h"
#include "spreadsheet_database.h"
#include "done.h"

// Constructor which creates a connection to the database.
SpreadSheetDatabase::SpreadSheetDatabase() {
  sql::Driver *driver;

  driver = get_driver_instance();
  connection_ = driver->connect("tcp://mysql.oscarmarshall.com", "socialss",
				"cs3505");
  
  connection_->setSchema("socialspreadsheet");
}

// Destructor which cleans up the connection.
SpreadSheetDatabase::~SpreadSheetDatabase() {
  delete connection_;
}

// Whether the spreadsheet currently exists in the database.
bool SpreadSheetDatabase::DoesSpreadSheetExist(std::string filename) {
  sql::PreparedStatement *statement;
  sql::ResultSet *result_set;

  // Prepare and execute the database query asking for a spreadsheet with the
  // filename.
  statement = connection_->prepareStatement("SELECT id FROM Spreadsheets WHERE filename = ?");
  statement->setString(1, filename);
  result_set = statement->executeQuery();

  bool result = result_set->next();

  delete result_set;
  delete statement;

  return result;
}

// Creates a spreadsheet with the given filename and password.
bool SpreadSheetDatabase::CreateSpreadSheet(std::string filename,
					    std::string password) {
  sql::PreparedStatement *statement;
  
  statement = connection_->prepareStatement("INSERT INTO Spreadsheets (filename, password) VALUES (?, ?)");
  statement->setString(1, filename);
  statement->setString(2, password);

  // Execute the insert. If it returns saying that it affected 1 row, then we
  // successfully added our spreadsheet. 
  bool result = statement->executeUpdate() == 1;

  delete statement;

  return result;
}

// Gets the password of a specified file. Assumes that the spreadsheet exists.
// Check to see if a spreadsheet exists by using the DoesSpreadSheetExist
// method.
std::string SpreadSheetDatabase::GetSpreadSheetPassword(std::string filename) {
  sql::PreparedStatement *statement;
  sql::ResultSet *result_set;

  statement = connection_->prepareStatement("SELECT password FROM Spreadsheets WHERE filename = ?");
  statement->setString(1, filename);
  result_set = statement->executeQuery();

  result_set->next();
  std::string result = result_set->getString(1);
  delete result_set;
  delete statement;

  return result;
}

// Gets an entire spreadsheet and packages it up into XML.
std::string SpreadSheetDatabase::GetSpreadSheetXml(std::string filename) {
  sql::PreparedStatement *statement;
  sql::ResultSet *result_set;

  int id = GetSpreadSheetId(filename);

  statement = connection_->prepareStatement("SELECT name, contents FROM Cells WHERE id = ?");
  statement->setInt(1, id);
  result_set = statement->executeQuery();

  std::stringstream result;

  result << "<?xml version=\"1.0\" encoding=\"utf-8\"?><spreadsheet version=\"ps6\">";

  while(result_set->next()) {
    result << "<cell><name>" << result_set->getString(1) << "</name><contents>" << result_set->getString(2) << "</contents></cell>";
  }

  result << "</spreadsheet>";

  delete result_set;
  delete statement;

  return result.str();
}

Done SpreadSheetDatabase::ChangeCell(std::string filename, std::string cell,
				     std::string contents) {
  sql::PreparedStatement *statement;
  boost::to_upper(cell);
  int id = GetSpreadSheetId(filename);
  std::string old_contents = GetCellContents(id, cell);
   
  statement = connection_->prepareStatement("DELETE FROM Cells WHERE id = ? AND name = ?");
  statement->setInt(1, id);
  statement->setString(2, cell);
  statement->executeUpdate();

  delete statement;

  if (contents.length() > 0) {
    statement = connection_->prepareStatement("INSERT INTO Cells (id, name, contents) VALUES (?, ?, ?)");
    statement->setInt(1, id);
    statement->setString(2, cell);
    statement->setString(3, contents);
    statement->executeUpdate();

    delete statement;
  }
  
  return Done(cell, old_contents);
}

// Get the id of a SpreadSheet. Assumes that the spreadsheet exists. Check to
// see if a spreadsheet exists by using the DoesSpreadSheetExist method.
int SpreadSheetDatabase::GetSpreadSheetId(std::string filename) {
  sql::PreparedStatement *statement;
  sql::ResultSet *result_set;

  statement = connection_->prepareStatement("SELECT id FROM Spreadsheets WHERE filename = ?");
  statement->setString(1, filename);
  result_set = statement->executeQuery();
  result_set->next();
  int id = result_set->getInt(1);

  delete result_set;
  delete statement;

  return id;
}

// Get the current contents of a cell. If the cell does not exist, returns the
// empty string.
std::string SpreadSheetDatabase::GetCellContents(int id, std::string cell) {
  sql::PreparedStatement *statement;
  sql::ResultSet *result_set;

  statement = connection_->prepareStatement("SELECT contents FROM Cells WHERE id = ? AND name = ?");
  statement->setInt(1, id);
  statement->setString(2, cell);
  result_set = statement->executeQuery();
  
  std::string result;

  if (result_set->next()) {
    result = result_set->getString(1);
  } else {
    result = "";
  }

  delete result_set;
  delete statement;

  return result;
}
