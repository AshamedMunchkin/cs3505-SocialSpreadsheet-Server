// done.cc
//
// Definitions for the Done class. A Done object represents an undo action on
// the stack.
//
// Author: Oscar Marshall, John Clukey, Josh Bell, Jake Guckert
//
// CS 3505 Final Project


#include <string>
#include "done.h"

/*
 * Constructor of a done object...You can't undo it until it's done.
 */
Done::Done(std::string cell, std::string old_contents) {
  cell_ = cell;
  old_contents_ = old_contents;
}

/*
 * Done destructor.
 */
Done::~Done() { }

/*
 * Getter for the cell name.
 */
std::string Done::cell() {
  return cell_;
}

/*
 * Getter for the old cell contents.
 */
std::string Done::old_contents() {
  return old_contents_;
}
