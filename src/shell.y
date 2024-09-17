
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <cstring>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT
%token AMPERSAND PIPE LESS NEWLINE IF FI THEN LBRACKET RBRACKET SEMI
%token DO DONE WHILE FOR IN

%{
//#define yylex yylex
#include <cstdio>
#include "Shell.hh"

void yyerror(const char * s);
int yylex();

%}

%% //start of rules section

goal: command_list;

arg_list:
        arg_list WORD { 
          Shell::TheShell->_simpleCommand->insertArgument( $2 ); }
        | /*empty string*/
	;

cmd_and_args:
  	WORD {
					Shell::TheShell->_simpleCommand = new SimpleCommand();
					Shell::TheShell->_simpleCommand->insertArgument( $1 );
        } 
        arg_list
	;

pipe_list:
        cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( 
		    Shell::TheShell->_simpleCommand ); 
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	| pipe_list PIPE cmd_and_args 
	    {
		Shell::TheShell->_pipeCommand->insertSimpleCommand(
                    Shell::TheShell->_simpleCommand );
                Shell::TheShell->_simpleCommand = new SimpleCommand(); 
	    }
	;

io_modifier:
	   GREATGREAT WORD
	    {
				if (Shell::TheShell->_pipeCommand->_outFile != NULL) {
					yyerror("Ambiguous output redirect.\n");
				}
				Shell::TheShell->_pipeCommand->_append = true;
				Shell::TheShell->_pipeCommand->_outFile = $2;
	    }
	 | GREAT WORD 
	    {
				if (Shell::TheShell->_pipeCommand->_outFile != NULL) {
					yyerror("Ambiguous output redirect.\n");
					exit(1);
				}
				Shell::TheShell->_pipeCommand->_outFile = $2;
	    }
	 | GREATGREATAMPERSAND WORD
	    {
				if (Shell::TheShell->_pipeCommand->_outFile != NULL) {
					yyerror("Ambiguous output redirect.\n");
					exit(1);
				}
				Shell::TheShell->_pipeCommand->_append = true;
				Shell::TheShell->_pipeCommand->_outFile = $2;
				Shell::TheShell->_pipeCommand->_errFile = new std::string($2->c_str());
	    }
	 | GREATAMPERSAND WORD
	    {
				if (Shell::TheShell->_pipeCommand->_outFile != NULL) {
					yyerror("Ambiguous output redirect.\n");
					exit(1);
				}
				Shell::TheShell->_pipeCommand->_outFile = $2;
				Shell::TheShell->_pipeCommand->_errFile = new std::string($2->c_str());
	    }
	 | LESS WORD
	    {
      	Shell::TheShell->_pipeCommand->_inFile = $2;
      }
	 | TWOGREAT WORD
	    {
	    	Shell::TheShell->_pipeCommand->_errFile = $2;
	    }
	;

io_modifier_list:
	io_modifier_list io_modifier 
	| /*empty*/
	;

background_optional: 
	AMPERSAND 
	    {
		Shell::TheShell->_pipeCommand->_background = true;
	    }
	| /*empty*/
	;

SEPARATOR:
	NEWLINE
	| SEMI
	;

command_line:
	 pipe_list io_modifier_list background_optional SEPARATOR 
         { 
	    Shell::TheShell->_listCommands->
		insertCommand(Shell::TheShell->_pipeCommand);
	    Shell::TheShell->_pipeCommand = new PipeCommand(); 
         }
        | if_command SEPARATOR 
         {
	    				Shell::TheShell->_listCommands->
						insertCommand(Shell::TheShell->_ifCommand);
         }
        | while_command SEPARATOR 
				{
					if (Shell::TheShell->_level == 0) {
							Shell::TheShell->_listCommands->
    				insertCommand(Shell::TheShell->_ifCommand);
					}
				}
        | for_command SEPARATOR 
				{
					if (Shell::TheShell->_level == 0) {
							Shell::TheShell->_listCommands->
            insertCommand(Shell::TheShell->_forCommand); 
					}
				}
        | SEPARATOR /*accept empty cmd line*/
        | error SEPARATOR {yyerrok; Shell::TheShell->clear(); }
	;          /*error recovery*/

command_list :
     command_line 
	{ 
	   Shell::TheShell->execute();
	}
     | 
     command_list command_line 
	{
	    Shell::TheShell->execute();
	}
     ;  /* command loop*/

if_command:
    IF LBRACKET 
	{ 
	    Shell::TheShell->_level++; 
	    Shell::TheShell->_ifCommand = new IfCommand();
			Shell::TheShell->_ifCommand->_loop_type = 0;
	} 
    arg_list RBRACKET SEMI THEN 
	{
	    Shell::TheShell->_ifCommand->insertCondition( 
		    Shell::TheShell->_simpleCommand);
	    Shell::TheShell->_simpleCommand = new SimpleCommand();
	}
    command_list FI 
	{ 
	    Shell::TheShell->_level--; 
	    Shell::TheShell->_ifCommand->insertListCommands( 
		    Shell::TheShell->_listCommands);
	    Shell::TheShell->_listCommands = new ListCommands();
	}
    ;

while_command:
    WHILE LBRACKET 
	{
      if (Shell::TheShell->_level > 0) {
				Shell::TheShell->if_comm.push(Shell::TheShell->_ifCommand);
				Shell::TheShell->list_comm.push(Shell::TheShell->_listCommands);
			}
      Shell::TheShell->_ifCommand = new IfCommand();
			Shell::TheShell->_listCommands = new ListCommands();
  		Shell::TheShell->_ifCommand->_loop_type = 1;
			Shell::TheShell->_level++;
	}
		arg_list RBRACKET SEMI DO 
	{
      Shell::TheShell->_ifCommand->insertCondition(
        Shell::TheShell->_simpleCommand);
      Shell::TheShell->_simpleCommand = new SimpleCommand();
  }
		command_list DONE 
  {
      Shell::TheShell->_level--;
      Shell::TheShell->_ifCommand->insertListCommands(
        Shell::TheShell->_listCommands);
      Shell::TheShell->_listCommands = new ListCommands();

			if (Shell::TheShell->_level == 0) {
				Shell::TheShell->_listCommands = new ListCommands();
			} else {
				// remove outer loop command from listComm
				Shell::TheShell->_listCommands = Shell::TheShell->list_comm.top();
				Shell::TheShell->list_comm.pop();

				// insert inner loop command + remove outer loop from ifComm
				Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_ifCommand);
				Shell::TheShell->_ifCommand = Shell::TheShell->if_comm.top();
        Shell::TheShell->if_comm.pop();
			}
  }
		;

for_command:
    FOR WORD IN 
  {
      if (Shell::TheShell->_level > 0) {
        Shell::TheShell->for_comm.push(Shell::TheShell->_forCommand);
        Shell::TheShell->list_comm.push(Shell::TheShell->_listCommands);
      }
      Shell::TheShell->_forCommand = new ForCommand();
			Shell::TheShell->_forCommand->insertVariable($2);
      Shell::TheShell->_listCommands = new ListCommands();
      Shell::TheShell->_level++;
  }
		arg_list SEMI DO 
  {
      Shell::TheShell->_forCommand->insertValues(
        Shell::TheShell->_simpleCommand);
      Shell::TheShell->_simpleCommand = new SimpleCommand();
  }
		command_list DONE
  {
      Shell::TheShell->_level--;
      Shell::TheShell->_forCommand->insertListCommands(
        Shell::TheShell->_listCommands);
      Shell::TheShell->_listCommands = new ListCommands();

      if (Shell::TheShell->_level == 0) {
        Shell::TheShell->_listCommands = new ListCommands();
      } else {
        // remove outer loop command from listComm
        Shell::TheShell->_listCommands = Shell::TheShell->list_comm.top();
        Shell::TheShell->list_comm.pop();

        // insert inner loop command + remove outer loop from ifComm
        Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_forCommand);
        Shell::TheShell->_forCommand = Shell::TheShell->for_comm.top();
        Shell::TheShell->for_comm.pop();
      }
  }
    ;

%% // end of rules section

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
