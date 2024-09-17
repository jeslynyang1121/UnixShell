/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring> 
#include <regex.h>
#include <dirent.h>
#include <cassert>
#include <stdlib.h>
#include <algorithm>
#include <string.h>

#include "PipeCommand.hh"
#include "Shell.hh"

std::string question = "";
std::string uscore = "";

PipeCommand::PipeCommand() {
    // Initialize a new vector of Simple PipeCommands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
}

void PipeCommand::insertSimpleCommand( SimpleCommand * simplePipeCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simplePipeCommand);
}

void PipeCommand::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simplePipeCommand : _simpleCommands) {
        delete simplePipeCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
    _append = false;
}

void PipeCommand::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple PipeCommands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simplePipeCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simplePipeCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void PipeCommand::execute() {
    bool inLoop;
		//if (inLoop == NULL) {
			inLoop = false;
		//}
		// Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        fprintf(stderr, "here1\n");
				Shell::TheShell->prompt();
        return;
    }
		
		// check for specific built in functions
		std::string* firstComm = _simpleCommands[0]->_arguments[0];
		
		// exit
		if (strcmp(firstComm->c_str(), "exit") == 0) {
    	printf("Good bye!!\n");
      exit(0);
    }
		
		// change curr directory to A (cd)
		if (strcmp(firstComm->c_str(), "cd") == 0) {
			const char * a;
			if (_simpleCommands[0]->_arguments.size() > 1) {
				a = _simpleCommands[0]->_arguments[1]->c_str();
				if (strcmp(a, "${HOME}") == 0) {
					a = getenv("HOME");
				}
			} else {
				a = getenv("HOME");
			}
			//printf("directory: %s\n", a);
      int result = chdir(a);
			if (result < 0) {
				fprintf(stderr, "/bin/sh: 1: cd: can't cd to %s\n", a);
			}
      clear();
      Shell::TheShell->prompt();
      return;
    }

    // Print contents of PipeCommand data structure
    if(isatty(0)) {
	    //print();
    }

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
   
    // use cat_grep.cc and slides as template
    int defaultin = dup(0);
    int defaultout = dup(1);
    int defaulterr = dup(2);
    int numSimpComm = _simpleCommands.size();
    int fdin;
    int fdout;
    int fderr;
		
    // open or create error and in files
    if (_errFile) {
			//printf("errFile: %s\n", _errFile->c_str());
    	if (_append) {
				fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
			} else {
				fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
			}
    } else {
			//printf("no error file\n");
    	fderr = dup(defaulterr);
    }

    if (_inFile) {
			//printf("inFile: %s\n", _inFile->c_str());
    	fdin = open(_inFile->c_str(), O_RDONLY);
			if (fdin < 0) {
				// inFile does not exist
					fprintf(stderr, "/bin/sh: 1: cannot open %s: No such file\n", _inFile->c_str());
					Shell::TheShell->prompt();
					clear();
					return;
			}
    } else {
    	fdin = dup(defaultin);
    }

    dup2(fderr, 2);
    close(fderr);
    int pid = 0;
	
    // loop through simple commands
    for (int i = 0; i < numSimpComm; i++) {
			//printf("in the loop: %d\n", i);
			// redirect input
			dup2(fdin, 0);
			close(fdin);

			if (i == numSimpComm - 1) {

				if (_outFile) {
    			if (_append) {
						//printf("appending\n");
        		fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
		  		} else {
						//printf("not appending\n");
						//printf("outfile: %s\n", _outFile->c_str());
        		fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
     			}
	  		} else {
					//printf("no out file\n");
      		fdout = dup(defaultout);
	  		}
			} else {
  			// Use default output
	  		int fdpipe[2];
    		pipe(fdpipe);
				fdin = fdpipe[0];
				fdout = fdpipe[1];
			}

  		// Redirect output
  		dup2(fdout, 1);
			close(fdout);
			
			const char ** args = expandEnvVarsAndWildcards(i);
			if (strcmp(args[0], "setenv") == 0) {
      	//fprintf(stderr, "setting: %s from %s to %s\n", args[1], getenv(args[1]), args[2]);
    	  const char * a = args[1];
  	    const char * b = args[2];
	      setenv(a, b, 1);
      	//clear();
    	  //Shell::TheShell->prompt();
  	    return;
	    }

    	// unset environment var A
    	if (strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "unsetenv") == 0) {
      	//printf("unsetting\n");
      	const char * a = _simpleCommands[0]->_arguments[1]->c_str();
      	unsetenv(a);
      	//clear();
      	//Shell::TheShell->prompt();
      	return;
    	}
  		// Create new child process 
 		  pid = fork();
  		if (pid < 0) {
    		perror("fork\n");
      	exit(2);
    	} else if (pid == 0) {
				// check for more built in functions		
				if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv") == 0) {
        	int eIndex = 0;
        	while (environ[eIndex] != NULL) {
        	  printf("%s\n", environ[eIndex]);
          	eIndex++;
        	}
        	exit(0);
      	}
				
				if (args == NULL) {
					return;
				}
				execvp(args[0], (char * const*)(args));
				//printf("here2\n");
      	// execvp() is not suppose to return, something went wrong
      	perror("execvp");
				exit(1);
    	}
			free(args);
		}
		

    // dup2 and close 
    dup2(defaultin, 0);
    dup2(defaultout, 1);
    dup2(defaulterr, 2);
    close(defaultin);
    close(defaultout);
    close(defaulterr);
		
    if (!_background) {
			int status;
    	waitpid(pid, &status, 0);
			std::string lastCode = std::to_string(WEXITSTATUS(status));
			question = lastCode;
    } else {
			std::string lastPid = std::to_string(pid);
			setenv("bang", lastPid.c_str(), 1);
			Shell::TheShell->_bPid.push_back(pid);
		}

    // Clear to prepare for next command
    if (!inLoop) {
			//clear();
		}

    // Print new prompt
    if (isatty(0)) {
    	//Shell::TheShell->prompt();
    }
}

// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
const char ** PipeCommand::expandEnvVarsAndWildcards (int simpleCommandNumber) {
		int i = simpleCommandNumber;
		
    // convert arguments to string array
    int numArgs = _simpleCommands[i]->_arguments.size();
    const char ** args = (const char **)malloc((numArgs + 1) * sizeof(char*));
		for (int j  = 0; j < numArgs; j++) {
			// expand environment variables
			args[j] = _simpleCommands[i]->_arguments[j]->c_str();		
			std::string * presub = new std::string(args[j]);
			expandSubIfNecessary(i, j);
			//fprintf(stderr, "after sub: %s\n", args[j]);
			//fprintf(stderr, "after su: %s\n", _simpleCommands[i]->_arguments[j]->c_str());
			args[j] = _simpleCommands[i]->_arguments[j]->c_str();
			args[j] = expandEnvVarsIfNecessary(args[j]);
			expandWildcardsIfNecessary(i, j);
			if (numArgs < _simpleCommands[i]->_arguments.size()) {
				numArgs = _simpleCommands[i]->_arguments.size();
				args = (const char **)realloc(args, (numArgs + 1) * sizeof(char*));
				args[j] = (_simpleCommands[i]->_arguments[j]->c_str());	
				//fprintf(stderr, "after sub 2: %s\n", args[j]);
			}
			if (j == numArgs - 1) {
				// last argument so set uscore
        uscore = args[j];
			}
			_simpleCommands[i]->_arguments[j] = presub;
		}
    args[numArgs] = NULL;
		return args;
}

void PipeCommand::expandSubIfNecessary(int i, int j) {
	/* subshell recognizes $(command) and `command` */
	char * arg = (char *)(_simpleCommands[i]->_arguments[j]->c_str());
  std::string command = std::string(arg);
	std::string startChars = command.substr(0, 2);
  /* remove $() or `` from command */
  if (startChars == "$(") {
    command = command.substr(2, command.size() - 3);
  } else if (command.at(0) == '`') {
    command = command.substr(1, command.size() - 2);
  } else {
		return;
	}
  command += "\n";
  int defaultIn = dup(0);
  int defaultOut = dup(1);
  int fdpipeIn[2];  // pass command to child
  int fdpipeOut[2]; // read output from child
  pipe(fdpipeIn);
  pipe(fdpipeOut);

  dup2(fdpipeIn[0], 0);
  dup2(fdpipeOut[1], 1);
  close(fdpipeIn[0]);
  close(fdpipeOut[1]);

  write(fdpipeIn[1], command.c_str(), command.size());
  close(fdpipeIn[1]);

  /* fork */
  int pid = fork();
  if (pid < 0) {
    perror("fork\n");
    exit(2);
  } else if (pid == 0) {
    execvp("/proc/self/exe", NULL);
    perror("subshell execvperror");
    exit(2);
  }
	
  dup2(defaultIn, 0);
  dup2(defaultOut, 1);
  close(defaultIn);
  close(defaultOut);

  waitpid(pid, NULL, 0);

  char * buffer = new char[1024];
  char c;
  int currIndex = 0;
  while (read(fdpipeOut[0], &c, 1) > 0) {
    if (c == '\n') {
      buffer[currIndex] = ' ';
    } else {
      buffer[currIndex] = c;
    }
    currIndex++;
  }
  close(fdpipeOut[0]);
  buffer[currIndex] = '\0';

	delete _simpleCommands[i]->_arguments[j];
  _simpleCommands[i]->_arguments.erase(_simpleCommands[i]->_arguments.begin() + j);

	std::string a = "";
  for (int k = 0; k < currIndex; k++) {
		if (buffer[k] == ' ') {
			// add argument to current simple command
			char * temp = (char *)malloc(sizeof(char) * a.length() + 1);
			std::strcpy(temp, a.c_str());
			a = "";
			std::string * newArg = new std::string(temp);
			_simpleCommands[i]->insertArgument(newArg);

		} else {
			// add char from buffer to current argument
			a += buffer[k];
		}
  }
}

const char * PipeCommand::expandEnvVarsIfNecessary(const char * arg) {
	/*${var} */
  std::string s = std::string(arg);
  std::string command = "";

  int index1 = s.find("$");
	if (index1 == -1) {
		// no environemental variables to expand
		return arg;
	}
  /* find + convert all ${var} */
  while (index1 != -1) {
    command += s.substr(0, index1);
    if (s[index1 + 1] == '{') {
      std::size_t index3 = s.find("}");
      if (index3 != -1) {
        std::string temp = s.substr(index1 + 2, index3 - index1 - 2);
				char * result = getenv(temp.c_str());
				if (result != NULL && strcmp(temp.c_str(), "SHELL") != 0  && strcmp(temp.c_str(), "_") != 0) {
          /* environmental variable successfully converted */
					command += result;
        } else {
          /* parse through for specific environmental variables*/
          if (strcmp(temp.c_str(), "$") == 0) {
            /* get PID of shell process */
            pid_t currPid = getpid();
            command += std::to_string(currPid);
          } else if (strcmp(temp.c_str(), "?") == 0) {
            /* return code of last executed non-background command */
						command += question;
          } else if (strcmp(temp.c_str(), "!") == 0) {
            /* get PID of last background process */
            char * lastPid = getenv("bang");
            command += lastPid;
          } else if (strcmp(temp.c_str(), "_") == 0) {
            /* return code of last executed non-background command */
            char * lastCode = getenv("uscore");
            command += uscore;
          } else if (strcmp(temp.c_str(), "SHELL") == 0) {
            /* get path of my shell */
            char fullPath[100];
            char * result = realpath("../lab3-src/shell", fullPath);
            if (result) {
              command += fullPath;
            } else {
              perror("SHELL path failure.");
            }
          } else {
            /* environmental variable failed to convert */
            command += temp;
          }
        }
      }
      s = s.substr(index3 + 1, s.size());
    }
    index1 = s.find("$");
  }
  command += s;
	char * ans = (char *) malloc(sizeof(char) * command.length() + 1);
	std::strcpy(ans, command.c_str());
  return ans;
	
}

int maxEntries = 20;
int nEntries = 0;
char ** array;

void PipeCommand::expandWildcardsIfNecessary(int i, int j) {
		char * arg = (char *)(_simpleCommands[i]->_arguments[j]->c_str());
		if ((strchr(arg, '*') == NULL && strchr(arg, '?') == NULL) || (strcmp(arg, "${?}") == 0)) {
      return;
    }

		maxEntries = 20;
		nEntries = 0;
		array = (char **) malloc(maxEntries * sizeof(char *));
		
		expandWildcard(NULL, arg);
		std::sort(array, array + nEntries, cmpfunc);

		if (nEntries > 0) {
			// new arguments to add so delete * or ?
			delete _simpleCommands[i]->_arguments[j];
			_simpleCommands[i]->_arguments.erase(_simpleCommands[i]->_arguments.begin() + j);
		}

		// add arguments
		for (int k = 0; k < nEntries; k++) {
			std::string * a = new std::string(array[k]);
			_simpleCommands[i]->insertArgument(a);
		}
		free(array);
		return;
}

bool PipeCommand::cmpfunc(char * a, char * b) {
	return strcmp(a, b) < 0;
}

void PipeCommand::expandWildcard(char * prefix, char * suffix) {
	if (suffix[0] == 0) {
		// suffix is empty.
		array[nEntries] = strdup(prefix);
    nEntries++;
		return;
	}

	// Obtain the next component in the suffix
	// Also advance suffix.
	char * s = NULL;
	if (suffix[0] == '/') {
		s = strchr((char *)(suffix + 1), '/');
	} else {
		s = strchr(suffix, '/');
	}

	char component[1024] = "";
	if (s != NULL) {
		// copy up to the first "/"
		strncpy(component, suffix, strlen(suffix) - strlen(s));
		suffix = s + 1;
	} else {
		//last parth of path. copy whole thing.
		strcpy(component, suffix);
    suffix = suffix + strlen(suffix);
	}

	// Now we need to expand the component
	char newPrefix[1024];
	if (strchr(component, '*') == NULL && strchr(component, '?') == NULL) {
		// component does not have wildcards
		if (prefix == NULL) {
			sprintf(newPrefix, "%s", component);
		} else {
			sprintf(newPrefix, "%s/%s", prefix, component);
		}
		expandWildcard(newPrefix, suffix);
		return;
	}
	// component has wildcards
	// convert component to regular expression
	// 1. Convert wildcard to regular expression
  // Convert “*” -> “.*”
  // “?” -> “.”
  // “.” -> “\.” and others you need
  // Also add ^ at the beginning and $ at the end to match
  // the begining the end of the word.
  // Allocate enough space ing ant for regular expression
  char * reg = (char *) malloc(2 * strlen(component) + 10);
  char * a = component;
  char * r = reg;
  // match beginning of line
  *r = '^';
  r++;
  while (*a) {
  	if (*a == '*') {
    	*r = '.';
      r++;
      *r ='*';
      r++;
    }
    else if (*a == '?') {
      *r = '.';
      r++;
    }
    else if (*a == '.') {
      *r = '\\';
      r++;
      *r = '.';
      r++;
		}
    else {
      *r = *a;
      r++;
    }
    a++;
	}
  // match end of line and add null char
  *r = '$';
  r++;
  *r = 0;

	// 2. compile regular expression. See lab3-src/regular.cc
  regex_t re;
  int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
  if (expbuf != 0) {
    perror("compile");
    return;
  }
  
	// 3. List directory and add as arguments the entries
  // that match the regular expression
  char * curr_dir;
	if (prefix == NULL) {
		curr_dir = strdup(".");
	} else {
		curr_dir = prefix;
	}

	DIR * dir = opendir(curr_dir);
  if (dir == NULL) {
    //perror("pendir");
    return;
  }
	struct dirent * ent;
  while ( (ent = readdir(dir) ) != NULL) {
    // Check if name matches
		regmatch_t match;
    if (regexec(&re, ent->d_name, 1, &match, 0) == 0 ) {
			if (nEntries == maxEntries) {
        maxEntries *= 2;
        array = (char **) realloc(array, maxEntries * sizeof(char *));
      }

			if (prefix == NULL || prefix[0] == 0) {
				sprintf(newPrefix, "%s", ent->d_name);
			} else {
				sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
			}
			if (ent->d_name[0] == '.') {
        if (component[0] == '.') {
					expandWildcard(newPrefix, suffix);
				}
			} else {
				expandWildcard(newPrefix, suffix);
			}
		}
  }
  closedir(dir);

}



