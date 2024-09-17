#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <regex.h>
#include <dirent.h>
#include <algorithm>

#include "Command.hh"
#include "SimpleCommand.hh"
#include "IfCommand.hh"
#include "ForCommand.hh"
#include "PipeCommand.hh"

ForCommand::ForCommand() {
    _variable = NULL;
		_listValues = NULL;
    _listCommands =  NULL;
    
}
void ForCommand::insertVariable( std::string * variable ) {
    _variable = variable->c_str();
}

void ForCommand::insertValues( SimpleCommand * listValues ) {
	_listValues = listValues;
}

void ForCommand::insertListCommands( ListCommands * listCommands) {
    _listCommands = listCommands;
}

void ForCommand::clear() {
}

void ForCommand::print() {
    printf("FOR [ \n");
    this->_listValues->print();
    printf("   ]; then\n");
    this->_listCommands->print();
}

int maxEntriesFor = 20;
int nEntriesFor = 0;
char ** arrayFor;
std::vector<std::string *> ForCommand::expandWildcardsIfNecessary(std::string * value) {
    std::vector<std::string *> args;
		char * arg = (char *)(value->c_str());
    if ((strchr(arg, '*') == NULL && strchr(arg, '?') == NULL) || (strcmp(arg, "${?}") == 0)) {
      args.push_back(value);
			return args;
    }

    maxEntriesFor = 20;
    nEntriesFor = 0;
    arrayFor = (char **) malloc(maxEntriesFor * sizeof(char *));

    expandWildcard(NULL, arg);
    std::sort(arrayFor, arrayFor + nEntriesFor, cmpfunc);

    if (nEntriesFor > 0) {
      // new arguments to add so delete * or ?
    } else {
			args.push_back(value);
		}
		// add arguments
    for (int k = 0; k < nEntriesFor; k++) {
      std::string * a = new std::string(arrayFor[k]);
      args.push_back(a);
    }

    free(arrayFor);
    return args;
}

bool ForCommand::cmpfunc(char * a, char * b) {
  return strcmp(a, b) < 0;
}

void ForCommand::expandWildcard(char * prefix, char * suffix) {
  if (suffix[0] == 0) {
    // suffix is empty.
    arrayFor[nEntriesFor] = strdup(prefix);
    nEntriesFor++;
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
      if (nEntriesFor == maxEntriesFor) {
        maxEntriesFor *= 2;
        arrayFor = (char **) realloc(arrayFor, maxEntriesFor * sizeof(char *));
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

void ForCommand::execute() {
	// iterate through each value in listValues
	for (int i = 0; i < _listValues->_arguments.size(); i++) {
		std::string * value = _listValues->_arguments[i];
		std::vector<std::string *> args = expandWildcardsIfNecessary(value);

		// iterate through arguments in value
		for (int j = 0; j < args.size(); j++) {
			std::string * temp = args[j];
			setenv(_variable, temp->c_str(), 1);
			_listCommands->execute();
			unsetenv(_variable);
		}
		//free(args);

	}
}
