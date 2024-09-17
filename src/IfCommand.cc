
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

#include "Command.hh"
#include "SimpleCommand.hh"
#include "IfCommand.hh"
#include "PipeCommand.hh"

//PipeCommand pipe;

IfCommand::IfCommand() {
    _condition = NULL;
    _listCommands =  NULL;
		_loop_type = 0;
}


// Run condition with command "test" and return the exit value.
int
IfCommand::runTest(SimpleCommand * condition) {
		// make copy of arguments
		SimpleCommand * sc = new SimpleCommand();
		int numArgs = condition->_arguments.size();
		for (int i = 0; i < numArgs; i++) {
			char * temp = (char *) malloc(sizeof(char) * condition->_arguments[i]->length() + 1);
			strcpy(temp, condition->_arguments[i]->c_str());
			std::string * newArg = new std::string(temp);
    	sc->insertArgument(newArg);
		}
		
		// shell
		int pid = fork();
		if (pid < 0) {
			perror("fork");
      exit(2);
		} else if (pid == 0) {
			// child
			numArgs = sc->_arguments.size() + 1;
    	const char ** args = (const char **)malloc((numArgs + 1) * sizeof(char*));
    	
			args[0] = "test";
			//fprintf(stderr, "%d\n", numArgs);
    	for (int i = 0; i < numArgs - 1; i++) {
				// subshell and expand vars
    		PipeCommand pipe;
		    const char * temp = sc->_arguments[i]->c_str();
		    temp = pipe.expandEnvVarsIfNecessary(temp);
				
				std::string * tempAsString = new std::string(temp);
		    sc->_arguments[i] = tempAsString;
				args[i + 1] = (char *)(sc->_arguments[i]->c_str());

				if (numArgs < sc->_arguments.size()) {
					numArgs = sc->_arguments.size();
				}
    	}
    	args[numArgs] = NULL;
			execvp(args[0], (char * const*)(args));
			perror("execvp");
      exit(1);
		} 

		// parent
		int status;
    waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
}

void 
IfCommand::insertCondition( SimpleCommand * condition ) {
    _condition = condition;
}

void 
IfCommand::insertListCommands( ListCommands * listCommands) {
    _listCommands = listCommands;
}

void 
IfCommand::clear() {
}

void 
IfCommand::print() {
    printf("IF [ \n");
    this->_condition->print();
    printf("   ]; then\n");
    this->_listCommands->print();
}

void 
IfCommand::execute() {
		int numLoops = 0;
    // Run command if test is 0
		if (this->_loop_type == 0) {
    	if (runTest(this->_condition) == 0) {
				_listCommands->execute();
	    }
		// Run command while test is 1
		} else if (this->_loop_type == 1) {
			while (runTest(this->_condition) == 0) {
        _listCommands->execute();
				numLoops++;
      }
		}
}

