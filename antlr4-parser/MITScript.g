grammar MITScript;
 
// Lexer Rules

INT : [0-9]+ ;

COMMENT: '//'~( '\r' | '\n' )* -> skip;

WHITESPACE : ( '\t' | ' ' | '\r' | '\n'| '\u000C' )+ -> skip ;

// The rest of your lexical rules go here

// Parser Rules

program 
  :  statement_list
  ;

statement_list 
  : // matches epislon  
  ;
