#ifndef shell_hh
#define shell_hh

#include "ListCommands.hh"
#include "PipeCommand.hh"
#include "IfCommand.hh"
#include "ForCommand.hh"
#include <vector>
#include <stack>

class Shell {

public:
  int _level; // Only outer level executes.
  bool _enablePrompt;
  ListCommands * _listCommands; 
  SimpleCommand *_simpleCommand;
  PipeCommand * _pipeCommand;
  IfCommand * _ifCommand;
  ForCommand * _forCommand;
	Command * _currentCommand;
  static Shell * TheShell;
	std::vector<int> _bPid;
	

  Shell();
	std::stack<IfCommand *> if_comm;
	std::stack<ForCommand *> for_comm;
	std::stack<ListCommands *> list_comm;
  void execute();
  void print();
  void clear();
  void prompt();

};

#endif
