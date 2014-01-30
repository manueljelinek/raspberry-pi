#ifndef SHELL_H
#define SHELL_H

#define MAXNUMBEROFFUNCTIONS 32
#define COMMANDLENGTH 32

typedef void (*fcn_ptr)(void); 

extern void shell(void);
extern void addNewCommand(fcn_ptr function_pointer, char command_name[COMMANDLENGTH],char* help_text);

extern void examplemaincommandsadd(void);

#endif /* SHELL_H */
