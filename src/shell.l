
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%% // start of rules section

"#".*\n {
  /* Discard comments starting with # */
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

"2>" {
  return TWOGREAT;
}

">>" {
  return GREATGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"&" {
  return AMPERSAND;
}

"[" {
  return LBRACKET;
}

"]" {
  return RBRACKET;
}

"if" {
  return IF;
}

"then" {
  return THEN;
}

"fi" {
  return FI;
}

"while" {
  return WHILE;
}

"do" {
  return DO;
}

"done" {
  return DONE;
}

"for" {
  return FOR;
}

"in" {
  return IN;
}

";" {
  return SEMI;
}

\".*\" {
	/* implement quotes */
	std::string s = std::string(yytext);
	s = s.substr(1, s.size() - 2);
	yylval.cpp_string = new std::string(s);
	return WORD;
}

[^ \t\n]*\\[^ \t\n]* {
  /* implement escape */
  std::string s = std::string(yytext);
  std::size_t escIndex = s.find("\\");
	/* find + convert all escapes */
	while (escIndex != -1) {
		s = s.substr(0, escIndex) + s.substr(escIndex + 1, s.size());
		escIndex = s.find("\\", escIndex + 1);
	}
  yylval.cpp_string = new std::string(s);
  return WORD;
}

"source"[ \t\n]*[^ \t\n]* {
	/* source A */
	char * file = yytext + strlen("source ");
	FILE * inFile = fopen(file, "r+");
	if (inFile == NULL) {
		perror("could not open inFile for source A");
	} else {
		/* parse through file inFile */
		YY_BUFFER_STATE bState = yy_create_buffer(inFile, YY_BUF_SIZE);
		yypush_buffer_state(bState);
		BEGIN(INITIAL);
		yyparse();
		yypop_buffer_state();
		fclose(inFile);
	}
}

$\([^\t\n]*\)|\`[^\t\n]*\` {
	yylval.cpp_string = new std::string(yytext);
  return WORD;
}

~[a-zA-Z ][^ \t\n]* {
	/* Tilde expansion */
	std::string dir = std::string(yytext);
	std::string home = getenv("HOME");
	if (dir[1] == ' ') {
		yylval.cpp_string = new std::string(home);
	} else {
		std::string newHome = std::string("/homes/");
		dir = dir.substr(1, dir.size() - 1);
		dir = newHome + dir;
		yylval.cpp_string = new std::string(dir);
  }	
	return WORD;
}

[^ \t\n|<>;][^ \t\n|<>;]*  {
  /* Assume that file names have only alpha chars */
	std::string temp = std::string(yytext);
	//fprintf(stderr, "huh\n");
	std::string firstChars = temp.substr(0, 2);
	//fprintf(stderr, "here?\n");
	if (firstChars != "$(" && temp.at(0) != '`' ) {
		//fprintf(stderr, "here??\n");
		yylval.cpp_string = new std::string(temp);
		//fprintf(stderr, "here???\n");
  	return WORD;
	}
	//fprintf(stderr, "omfg\n");
}

%% // end of rules section

