#ifndef pipecommand_hh
#define pipecommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"

extern bool inLoop;

// Command Data Structure

class PipeCommand : public Command {
public:
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  bool _append;

  PipeCommand();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();

  // Expands environment vars and wildcards of a SimpleCommand and
  // returns the arguments to pass to execvp.
  const char ** expandEnvVarsAndWildcards(int simpleCommandNumber);
	void expandSubIfNecessary(int i, int j);
	const char *  expandEnvVarsIfNecessary(const char * arg);
	void expandWildcardsIfNecessary(int i, int j);
	static bool cmpfunc(char * a, char * b);
	void expandWildcard(char * prefix, char * suffix);

};

#endif
