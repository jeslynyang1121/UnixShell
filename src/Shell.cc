
#include <unistd.h>
#include <cstdio>
#include <sys/wait.h>
#include "Command.hh"
#include "Shell.hh"

//added
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int yyparse(void);

Shell * Shell::TheShell;

Shell::Shell() {
    this->_level = 0;
    this->_enablePrompt = true;
    this->_listCommands = new ListCommands(); 
    this->_simpleCommand = new SimpleCommand();
    this->_pipeCommand = new PipeCommand();
    this->_currentCommand = this->_pipeCommand;
    
		if ( !isatty(0)) {
			this->_enablePrompt = false;
    }
}

void Shell::prompt() {
    if (_enablePrompt) {
			printf("myshell>");
			fflush(stdout);
    }
}

void Shell::print() {
    printf("\n--------------- Command Table ---------------\n");
    this->_listCommands->print();
}

void Shell::clear() {
    this->_listCommands->clear();
    this->_simpleCommand->clear();
    this->_pipeCommand->clear();
    this->_currentCommand->clear();
    this->_level = 0;
}

void Shell::execute() {
  if (this->_level == 0 ) {
    //this->print();
    this->_listCommands->execute();
    this->_listCommands->clear();
    this->prompt();
  }
}

// ctrl-c and zombie function
extern "C" void sigIntHandler (int sig) {
  if (sig == SIGINT) {
		printf("\n");
		Shell::TheShell->prompt();
	} else if (sig == SIGCHLD) {
		int pid = waitpid(-1, NULL, WNOHANG);
		while (pid != -1) {
			pid = waitpid(-1, NULL, WNOHANG);
		}
		
		for (int i = 0; i < (int)(Shell::TheShell->_bPid.size()); i++) {
			if (pid == Shell::TheShell->_bPid[i]) {
				kill(pid, sig);
				if (isatty(0)) {
      		printf("[%d] exited\n", pid);
    		}
				Shell::TheShell->_bPid.erase(Shell::TheShell->_bPid.begin() + i);
				break;
			}
		}
	}	
}


void yyset_in (FILE *  in_str );

int main (int argc, char **argv) {
  //fprintf(stderr, "path to shell: %s\n", argv[0]);

	char * input_file = NULL;
  if ( argc > 1 ) {
    input_file = argv[1];
    FILE * f = fopen(input_file, "r");
    if (f==NULL) {
			fprintf(stderr, "Cannot open file %s\n", input_file);
      perror("fopen");
      exit(1);
    }
    yyset_in(f);
  }  

  Shell::TheShell = new Shell();


  //handle ctrl-c signalling and zombie processes
  struct sigaction sigAction;
  sigAction.sa_handler = sigIntHandler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &sigAction, NULL)) {
  	perror("sigaction\n");
  	exit(-1);
  }
	if (sigaction(SIGCHLD, &sigAction, NULL)) {
    perror("sigaction\n");
    exit(-1);
  }

	int i = 0;
	while(i < argc - 1) {
		const char * a = std::to_string(i).c_str();
		setenv(a, argv[i + 1], 1);
		i++;
	}
	setenv("#", std::to_string(argc - 2).c_str(), 1);


	if (input_file != NULL) {
    // No prompt if running a script
    Shell::TheShell->_enablePrompt = false;
  }
  else {
    Shell::TheShell->prompt();
  }

  yyparse();
}


