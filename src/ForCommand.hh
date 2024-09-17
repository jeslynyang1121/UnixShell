#ifndef forcommand_hh
#define forcommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"
#include "ListCommands.hh"
#include "PipeCommand.hh"

// Command Data Structure

class ForCommand : public Command {
public:
  const char * _variable;
	SimpleCommand * _listValues;
  ListCommands * _listCommands;

  ForCommand();
  void insertVariable( std::string * variable );
	void insertValues( SimpleCommand * listValues );
  void insertListCommands( ListCommands * listCommands);
	std::vector<std::string *> expandWildcardsIfNecessary(std::string * value);
	bool static cmpfunc(char * a, char * b);
	void expandWildcard(char * prefix, char * suffix);

	void clear();
  void print();
  void execute();

};

#endif
