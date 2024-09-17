#ifndef ifcommand_hh
#define ifcommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"
#include "ListCommands.hh"
#include "PipeCommand.hh"

// Command Data Structure

class IfCommand : public Command {
public:
  SimpleCommand * _condition;
  ListCommands * _listCommands; 
	int _loop_type;

  IfCommand();
  void insertCondition( SimpleCommand * condition );
  void insertListCommands( ListCommands * listCommands);
  static int runTest(SimpleCommand * condition);

  void clear();
  void print();
  void execute();

};

#endif
