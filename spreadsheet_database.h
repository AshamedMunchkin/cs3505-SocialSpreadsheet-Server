/*
 * Spreadsheet Database handler
 * Date: April 23, 2013
 * Author: Oscar Marshall
 *
 * CS 3505 Final Project
 */

#ifndef SPREADSHEET_DATABASE_H_
#define SPREADSHEET_DATABASE_H_

#include <string>
#include "include/mysql_connection.h"

#include "done.h"

class SpreadSheetDatabase {
 public:
  SpreadSheetDatabase();
  ~SpreadSheetDatabase();
  bool DoesSpreadSheetExist(std::string filename);
  bool CreateSpreadSheet(std::string filename, std::string password);
  std::string GetSpreadSheetPassword(std::string filename);
  std::string GetSpreadSheetXml(std::string filename);
  Done ChangeCell(std::string filename, std::string cell, std::string contents);
 private:
  int GetSpreadSheetId(std::string filename);
  std::string GetCellContents(int id, std::string cell);
  sql::Connection *connection_;
};

#endif
