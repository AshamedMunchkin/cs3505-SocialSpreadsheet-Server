// done.h
// 
// Represents an action on the undo stack.
//
// Author: Oscar Marshall, John Clukey, Josh Bell, Jake Guckert
// Date: April 21, 2013
//
// CS 3505 Final Project

#ifndef DONE_H_
#define DONE_H_

#include <string>

class Done {
 public:
  Done(std::string cell, std::string old_contents);
  ~Done();
  std::string cell();
  std::string old_contents();
 private:
  std::string cell_; // The old cell name.
  std::string old_contents_; // The old cell contents.
};

#endif
