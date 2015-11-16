#include "../includes/cfrtil.h"

#if RETRO
namespace retro
{
    void retro ( void ) ;
} ;
#endif

#if 0 //SL5
namespace Sl5 {
    void sl5 ( void ) ;
    void isl5 ( void ) ;
} ;
#endif

CPrimitive CPrimitives [] = {
    { "(", ( block ) LO_ReadEvalPrint1, IMMEDIATE | KEYWORD, 0, "Lisp", "Root" },
    { ")", CfrTil_NoOp, IMMEDIATE | KEYWORD, 0, "Lisp", "Root" },
    { "Printf", ( block ) Printf, 0, LISP_C_RTL_ARGS | T_LISP_SPECIAL | LISP_VOID_RETURN, "Lisp", "Root" },
    //{ "_define", ( block ) _LO__Define, 0, T_LISP__DEFINE | T_LISP_SPECIAL, "Lisp", "Root" }, // nb. 2 underscores
    { "define", ( block ) LO_Define, 0, T_LISP_DEFINE | T_LISP_SPECIAL, "Lisp", "Root" },
    { "compile", ( block ) _LO_Compile, 0, T_LISP_COMPILE | T_LISP_SPECIAL, "Lisp", "Root" },
    { "_lambda", ( block ) LO_MakeLambda, 0, T_LAMBDA | T_LISP_SPECIAL, "Lisp", "Root" },
    { "begin", ( block ) LO_Begin, 0, T_LISP_BEGIN | T_LISP_SPECIAL, "Lisp", "Root" },
    { "set", ( block ) LO_Set, 0, T_LISP_SET | T_LISP_SPECIAL, "Lisp", "Root" },
    { "let", ( block ) LO_Let, 0, T_LISP_LET | T_LISP_SPECIAL, "Lisp", "Root" },
    { "if*", ( block ) LO_Cond, 0, T_LISP_IF | T_LISP_SPECIAL, "Lisp", "Root" },
    { "macro", ( block ) _LO_Macro, 0, T_LISP_MACRO | T_LISP_SPECIAL, "Lisp", "Root" }, // nb. too many clashes with other 'if's
    { ".if", ( block ) LO_Cond, 0, T_LISP_IF | T_LISP_SPECIAL, "Lisp", "Root" },
    { "cond", ( block ) LO_Cond, 0, T_LISP_IF | T_LISP_SPECIAL, "Lisp", "Root" },
    { "quote", ( block ) LO_Quote, 0, T_LISP_READ_MACRO, "Lisp", "Root" },
    { "'", ( block ) LO_Quote, KEYWORD, T_LISP_READ_MACRO, "Lisp", "Root" },
    //{ "@", ( block ) LO_Splice, 0, T_LISP_READ_MACRO, "Lisp", "Root" },
    { "quasiquote", ( block ) LO_QuasiQuote, 0, T_LISP_READ_MACRO, "Lisp", "Root" },
    { "`", ( block ) LO_QuasiQuote, KEYWORD, T_LISP_READ_MACRO, "Lisp", "Root" },
    { "unquote", ( block ) LO_UnQuote, 0, T_LISP_READ_MACRO, "Lisp", "Root" },
    { ",", ( block ) LO_UnQuote, 0, T_LISP_READ_MACRO, "Lisp", "Root" },
    { "unquoteSplicing", ( block ) LO_UnQuoteSplicing, 0, T_LISP_UNQUOTE_SPLICING | T_LISP_READ_MACRO, "Lisp", "Root" },
    { ",@", ( block ) LO_UnQuoteSplicing, 0, T_LISP_UNQUOTE_SPLICING | T_LISP_READ_MACRO, "Lisp", "Root" },
    //{ "\"", LO_DoubleQuote, 0, T_LISP_TERMINATING_MACRO, "Lisp", "Root" },
    { "list", ( block ) LO_List, 0, LIST_FUNCTION, "Lisp", "Root" },
    //{ "cfrTil", ( block ) LispCfrTil, IMMEDIATE | CPRIMITIVE | LISP_CFRTIL | CFRTIL_WORD, 0, "Lisp", "Root" },
    //{ "::", ( block ) _LO_CfrTil, 0, T_LISP_CFRTIL | T_LISP_SPECIAL, "Lisp", "Root" },
    { ":", ( block ) _LO_Colon, 0, T_LISP_COLON | T_LISP_SPECIAL, "Lisp", "Root" },
    //{ ";", CfrTil_SemiColon, IMMEDIATE|KEYWORD, 0, "Lisp", "Root" }, // nb. in Lisp we want this not the one compiled in .init.cft

    { "'", CfrTil_Tick, IMMEDIATE | KEYWORD, 0, "Forth", "Root" },

    { "if", CfrTil_If, IMMEDIATE, 0, "Shell", "Root" },
    { "else", CfrTil_Else, IMMEDIATE, 0, "Shell", "Root" },
    { "endif", CfrTil_EndIf, IMMEDIATE, 0, "Shell", "Root" },
    { "$", ShellEscape, 0, 0, "Shell", "Root" },

    { "Printf", ( block ) Printf, LISP_C_RTL_ARGS | LISP_VOID_RETURN, 0, "C", "Root" },
    //{ ")", CfrTil_NoOp, IMMEDIATE, 0, "C", "Root" },
    { "()", CfrTil_NoOp, IMMEDIATE, 0, "C", "Root" },
    { "(", CfrTil_LocalsAndStackVariablesBegin, IMMEDIATE | KEYWORD, 0, "C", "Root" },
    { "(|", CfrTil_LocalVariablesBegin, IMMEDIATE, 0, "C", "Root" },
    //{ "|-", CfrTil_NoOp, IMMEDIATE, 0, "C", "Root" },
    { "if{", CfrTil_If, IMMEDIATE, 0, "C", "Root" },
    { "}else{", CfrTil_Else, IMMEDIATE, 0, "C", "Root" },
    { "}fi", CfrTil_EndIf, IMMEDIATE, 0, "C", "Root" },
    { "case", CfrTil_Case, IMMEDIATE, 0, "C", "Root" },
    { "switch", CfrTil_Switch, IMMEDIATE, 0, "C", "Root" },
    { ">?", CfrTil_EndIf, IMMEDIATE, 0, "C", "Root" },
    // { ">;", CfrTil_EndIf, IMMEDIATE, 0, "C", "Root" },
    { ">:<", CfrTil_Else, IMMEDIATE, 0, "C", "Root" },
    { "?<", CfrTil_If, IMMEDIATE, 0, "C", "Root" },
    { "++", CfrTil_PlusPlusLValue, IMMEDIATE | CATEGORY_OP_1_ARG, 0, "C", "Root" },
    { "--", CfrTil_MinusMinusLValue, IMMEDIATE | CATEGORY_OP_1_ARG, 0, "C", "Root" },
    //{ "lReturnTOS", CfrTil_Locals_ReturnTOS, IMMEDIATE, 0, "C", "Root" },
    //{ "lReturnEAX", CfrTil_Locals_ReturnEAX, IMMEDIATE, 0, "C", "Root" },
    //{ "cReturnEAX", CfrTil_CReturnEAX, IMMEDIATE, 0, "C", "Root" },
    //{ "cstackPush", Compile_DspPop_EspPush, IMMEDIATE, 0, "C", "Root" },
    { "cfactorial", CFactorial, 0, 0, "C", "Root" },
    { "cfactorial2", CFactorial2, 0, 0, "C", "Root" },
    { "cfactorial3", CFactorial3, 0, 0, "C", "Root" },
    { "cfib", CFib, 0, 0, "C", "Root" },
    { "'", CfrTil_Tick, IMMEDIATE, 0, "C", "Root" },
    { "c_rtl", CfrTil_C_Prefix, IMMEDIATE, 0, "C", "Root" },
    { "c_prefix", CfrTil_C_Prefix, IMMEDIATE, 0, "C", "Root" },
    { "c_return", CfrTil_C_Return, IMMEDIATE, 0, "C", "Root" },

    { "c_syntaxOn", CfrTil_C_Syntax_On, 0, 0, "Compiler", "Root" }, // put this here so Compiler will be in Root namespace and Compiler will close to the top
    { "c_syntaxOff", CfrTil_C_Syntax_Off, 0, 0, "Compiler", "Root" },

    { "}", CfrTil_End_C_Block, IMMEDIATE | KEYWORD, 0, "C_Syntax", "C" },
    { ";", CfrTil_C_Semi, IMMEDIATE | KEYWORD, 0, "C_Syntax", "C" },
    { ",", CfrTil_NoOp, IMMEDIATE | KEYWORD, 0, "C_Syntax", "C" },
    { "!", CfrTil_LogicalNot, IMMEDIATE | CATEGORY_OP_1_ARG | CATEGORY_LOGIC | PREFIX, 0, "C_Syntax", "C" },
    { "c_class", ( block ) CfrTil_C_Class_New, 0, 0, "C_Syntax", "C" },
    { "type", ( block ) CfrTil_Type_New, 0, 0, "C_Syntax", "C" },
    { "typedef", CfrTil_Typedef, 0, 0, "C_Syntax", "C" },
    { "&", CfrTil_AddressOf, IMMEDIATE, 0, "C_Syntax", "C" }, // avoid name clash with '&&' and '&' 

    { "}", CfrTil_TypedefStructEnd, IMMEDIATE, 0, "C_Typedef", "C_Syntax" },
    { "{", CfrTil_TypedefStructBegin, IMMEDIATE, 0, "C_Typedef", "C_Syntax" },

    { ")", CfrTil_EndBlock, IMMEDIATE, 0, "C_Combinators", "C" },
    //{ "(", CfrTil_BeginBlock, IMMEDIATE, 0, "C_Combinators", "Combinators" },
    { "if", CfrTil_If_C_Combinator, KEYWORD | COMBINATOR | IMMEDIATE, 0, "C_Combinators", "C" },
    { "while", CfrTil_While_C_Combinator, KEYWORD | COMBINATOR | IMMEDIATE, 0, "C_Combinators", "C" },
    { "do", CfrTil_DoWhile_C_Combinator, KEYWORD | COMBINATOR | IMMEDIATE, 0, "C_Combinators", "C" },
    { "for", CfrTil_For_C_Combinator, KEYWORD | COMBINATOR | IMMEDIATE, 0, "C_Combinators", "C" },
    { "loop", CfrTil_Loop_C_Combinator, KEYWORD | COMBINATOR | IMMEDIATE, 0, "C_Combinators", "C" },

    //{ "!", CfrTil_LogicalNot, IMMEDIATE | CATEGORY_OP_EQUAL, 0, "Infix", "Compiler" },
    { "+", ( block ) CfrTil_Plus, IMMEDIATE | CATEGORY_OP_UNORDERED | PREFIXABLE | INFIXABLE, 0, "Infix", "Compiler" },
    { "*", ( block ) CfrTil_Multiply, IMMEDIATE | CATEGORY_OP_UNORDERED | PREFIXABLE | INFIXABLE, 0, "Infix", "Compiler" },
    { "=", CfrTil_C_Infix_Equal, IMMEDIATE | CATEGORY_OP_EQUAL | KEYWORD, 0, "Infix", "Compiler" },
    { "(", Interpret_DoParenthesizedRValue, IMMEDIATE | KEYWORD, 0, "Infix", "Compiler" },
    { ")", CfrTil_EndBlock, IMMEDIATE | KEYWORD, 0, "Infix", "Compiler" },

    { "(", CfrTil_LocalsAndStackVariablesBegin, IMMEDIATE, 0, "Locals", "Root" },
    { "(|", CfrTil_LocalVariablesBegin, IMMEDIATE, 0, "Locals", "Root" },

    //{ "linrec", CfrTil_Combinator_LinRec, COMBINATOR|IMMEDIATE, 0, "Combinators", "Root" },
    //{ "ifte", CfrTil_TrueFalseCombinator3, COMBINATOR|IMMEDIATE|RT_STACK_OP, 0, "Combinators", "Root" },
    //{ "ifElse", CfrTil_TrueFalseCombinator3, COMBINATOR|IMMEDIATE|RT_STACK_OP, 0, "Combinators", "Root" },
    { "tf2", CfrTil_TrueFalseCombinator2, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "tf3", CfrTil_TrueFalseCombinator3, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "if1", CfrTil_If1Combinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "if", CfrTil_If2Combinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "if2", CfrTil_If2Combinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    //{ "cond", CfrTil_If2Combinator, COMBINATOR|IMMEDIATE|RT_STACK_OP, 0, "Combinators", "Root" },
    { "while", ( block ) CfrTil_WhileCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "doWhile", ( block ) CfrTil_DoWhileCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "doWhileDo", CfrTil_DoWhileDoCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "call", CfrTil_BlockRun, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "run", CfrTil_BlockRun, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "_", CfrTil_DropBlock, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "loop", CfrTil_LoopCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "nloop", CfrTil_NLoopCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },
    { "for", CfrTil_ForCombinator, COMBINATOR | IMMEDIATE, 0, "Combinators", "Root" },

    { "in", CfrTil_PrintInNamespace, 0, 0, "Namespace", "Root" },
    { "using", CfrTil_Using, 0, 0, "Namespace", "Root" },
    { "usingFirst", CfrTil_Namespace_UsingFirst, 0, 0, "Namespace", "Root" },
    { "usingLast", CfrTil_Namespace_UsingLast, 0, 0, "Namespace", "Root" },
    { "notUsing", CfrTil_Namespace_NotUsing, 0, 0, "Namespace", "Root" },
    { "namespaces", CfrTil_Namespaces, 0, 0, "Namespace", "Root" },
    { "namespaceTree", CfrTil_Namespaces_PrettyPrintTree, 0, 0, "Namespace", "Root" },
    { "namespaceTreeWithWords", CfrTil_Namespaces_PrettyPrintTreeWithWords, 0, 0, "Namespace", "Root" },
    { "words", CfrTil_Words, 0, 0, "Namespace", "Root" },
    { "allWords", CfrTil_AllWords, 0, 0, "Namespace", "Root" },
    { "_namespaceWords", _CfrTil_NamespaceWords, 0, 0, "Namespace", "Root" },
    { "namespaceWords", CfrTil_NamespaceWords, 0, 0, "Namespace", "Root" },
    { "namespace", CfrTil_Namespace_New, 0, 0, "Namespace", "Root" },
    //{ "sealed", CfrTil_NamespaceSealed, 0, 0, "Namespace", "Root" },
    { "find", CfrTil_Postfix_Find, 0, 0, "Namespace", "Root" },

    { "};", CfrTil_ClassStructureEnd, 0, 0, "Class", "Root" },
    { "+:{", CfrTil_CloneStructureBegin, 0, 0, "Class", "Root" },
    { ":{", CfrTil_ClassStructureBegin, 0, 0, "Class", "Root" },
    { "class", CfrTil_Class_New, 0, 0, "Class", "Root" },
    { "new", CfrTil_Class_Value_New, 0, 0, "Class", "Root" },
    { "clone", CfrTil_Class_Clone, 0, 0, "Class", "Root" },
    { "]", CfrTil_ArrayEnd, ( IMMEDIATE | OBJECT_FIELD ), 0, "Class", "Root" },
    { "[", CfrTil_ArrayBegin, ( IMMEDIATE | OBJECT_FIELD ), 0, "Class", "Root" },
    { ".", CfrTil_Dot, ( IMMEDIATE | OBJECT_OPERATOR ), 0, "Class", "Root" },

    { "intToBigInt", CfrTil_BigInt_Init, 0, 0, "BigInt", "Class" },
    { "+", CfrTil_BigInt_Add, 0, 0, "BigInt", "Class" },
    { "+=", CfrTil_BigInt_PlusEqual, 0, 0, "BigInt", "Class" },
    { "++", CfrTil_BigInt_PlusPlus, 0, 0, "BigInt", "Class" },
    { "-", CfrTil_BigInt_Subtract, 0, 0, "BigInt", "Class" },
    { "-=", CfrTil_BigInt_MinusEqual, 0, 0, "BigInt", "Class" },
    { "--", CfrTil_BigInt_MinusMinus, 0, 0, "BigInt", "Class" },
    { "*", CfrTil_BigInt_Multiply, 0, 0, "BigInt", "Class" },
    { "^", CfrTil_BigInt_Power, 0, 0, "BigInt", "Class" },
    { "*=", CfrTil_BigInt_MultiplyEqual, 0, 0, "BigInt", "Class" },
    { "/", CfrTil_BigInt_DivideWithRemainder, 0, 0, "BigInt", "Class" },
    { "/=", CfrTil_BigInt_DivideEqual_RemainderDiscarded, 0, 0, "BigInt", "Class" },
    { "==", ( block ) CfrTil_BigInt_LogicalEquals, 0, 0, "BigInt", "Class" },
    { "!=", ( block ) CfrTil_BigInt_LogicalDoesNotEqual, 0, 0, "BigInt", "Class" },
    { "<=", ( block ) CfrTil_BigInt_LessThanOrEqual, 0, 0, "BigInt", "Class" },
    { "<", ( block ) CfrTil_BigInt_LessThan, 0, 0, "BigInt", "Class" },
    { ">=", ( block ) CfrTil_BigInt_GreaterThanOrEqual, 0, 0, "BigInt", "Class" },
    { ">", ( block ) CfrTil_BigInt_GreaterThan, 0, 0, "BigInt", "Class" },
    { "bprint", CfrTil_BigInt_Print, 0, 0, "BigInt", "Class" },

    { "intToBigFloat", CfrTil_BigFloat_Init, 0, 0, "BigInt", "Class" },
    { "precision", CfrTil_BigFloat_Precision, 0, 0, "BigFloat", "Class" },
    { "width", CfrTil_BigFloat_Width, 0, 0, "BigFloat", "Class" },
    { "+", CfrTil_BigFloat_Add, 0, 0, "BigFloat", "Class" },
    { "-", CfrTil_BigFloat_Subtract, 0, 0, "BigFloat", "Class" },
    { "*", CfrTil_BigFloat_Multiply, 0, 0, "BigFloat", "Class" },
    { "^", CfrTil_BigFloat_Power, 0, 0, "BigFloat", "Class" },
    { "/", CfrTil_BigFloat_Divide, 0, 0, "BigFloat", "Class" },
    { "==", ( block ) CfrTil_BigFloat_Equal, 0, 0, "BigFloat", "Class" },
    { "<=", ( block ) CfrTil_BigFloat_LessThanOrEqual, 0, 0, "BigFloat", "Class" },
    { "<", ( block ) CfrTil_BigFloat_LessThan, 0, 0, "BigFloat", "Class" },
    { ">=", ( block ) CfrTil_BigFloat_GreaterThanOrEqual, 0, 0, "BigFloat", "Class" },
    { ">", ( block ) CfrTil_BigFloat_GreaterThan, 0, 0, "BigFloat", "Class" },
    { "eprint", CfrTil_BigFloat_EPrint, 0, 0, "BigFloat", "Class" },
    { "fprint", CfrTil_BigFloat_FPrint, 0, 0, "BigFloat", "Class" },

    { "||", CfrTil_LogicalOr, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "or", CfrTil_LogicalOr, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "xor", CfrTil_LogicalXor, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "^^", CfrTil_LogicalXor, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "!", CfrTil_LogicalNot, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG | CATEGORY_LOGIC, 0, "Logic", "Root" },
    { "not", CfrTil_LogicalNot, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG | CATEGORY_LOGIC, 0, "Logic", "Root" },
    { "and", CfrTil_LogicalAnd, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "&&", CfrTil_LogicalAnd, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "<", CfrTil_LessThan, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { ">", CfrTil_GreaterThan, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "==", CfrTil_Equals, IMMEDIATE | STACKING | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "equal", CfrTil_Equals, IMMEDIATE | STACKING | CATEGORY_LOGIC, 0, "Logic", "Root" },
    { "!=", CfrTil_DoesNotEqual, IMMEDIATE | STACKING | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { "<=", CfrTil_LessThanOrEqual, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },
    { ">=", CfrTil_GreaterThanOrEqual, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | CATEGORY_LOGIC | INFIXABLE | KEYWORD, 0, "Logic", "Root" },

    { "+", CfrTil_Plus, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "-", CfrTil_Minus, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "+=", CfrTil_PlusEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "-=", CfrTil_MinusEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "*", CfrTil_Multiply, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "/", CfrTil_Divide, IMMEDIATE | STACKING | CATEGORY_OP_DIVIDE | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "modulo", CfrTil_Mod, IMMEDIATE | STACKING | CATEGORY_OP_DIVIDE | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "%", CfrTil_Mod, IMMEDIATE | STACKING | CATEGORY_OP_DIVIDE | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "*=", CfrTil_MultiplyEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "/=", CfrTil_DivideEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Int", "Root" },
    { "++", CfrTil_PlusPlusRValue, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG, 0, "Int", "Root" },
    { "--", CfrTil_MinusMinusRValue, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG, 0, "Int", "Root" },
    { "intPrint", CfrTil_PrintInt, 0, 0, "Int", "Root" },

    { "<<", CfrTil_ShiftLeft, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { ">>", CfrTil_ShiftRight, IMMEDIATE | STACKING | CATEGORY_OP_ORDERED | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { "&", CfrTil_BitWise_AND, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { "|", CfrTil_BitWise_OR, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    //{ "|", CfrTil_BitWise_OR, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { "^", CfrTil_BitWise_XOR, IMMEDIATE | STACKING | CATEGORY_OP_UNORDERED, 0, "Bits", "Logic" },
    { "~", CfrTil_BitWise_NOT, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG, 0, "Bits", "Logic" },
    { "neg", CfrTil_BitWise_NEG, IMMEDIATE | STACKING | CATEGORY_OP_1_ARG, 0, "Bits", "Logic" },
    { "<<=", CfrTil_ShiftLeft_Equal, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { ">>=", CfrTil_ShiftRight_Equal, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { "&=", CfrTil_BitWise_AndEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },
    { "|=", CfrTil_BitWise_OrEqual, IMMEDIATE | STACKING | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Bits", "Logic" },

    { "_fi", CfrTil_EndIf, IMMEDIATE, 0, "Asm", "Root" },
    { "_else_", CfrTil_Else, IMMEDIATE, 0, "Asm", "Root" },
    { "if_", CfrTil_If, IMMEDIATE, 0, "Asm", "Root" },
    { "Jmp", CfrTil_JMP, IMMEDIATE, 0, "Asm", "Root" },
    //{ "cmp", CfrTil_CMP, IMMEDIATE | CATEGORY_OP_ORDERED, 0, "Asm", "Root" },
    //{ "test", CfrTil_TEST, IMMEDIATE | CATEGORY_OP_ORDERED, 0, "Asm", "Root" },
    { "jmpHere", CfrTil_JmpToHere, IMMEDIATE, 0, "Asm", "Root" },
    { "Jcc", CfrTil_Compile_Jcc, IMMEDIATE, 0, "Asm", "Root" },
    { "jcc", CfrTil_Jcc_Label, IMMEDIATE, 0, "Asm", "Root" },
    { "goto", CfrTil_Goto, IMMEDIATE, 0, "Asm", "Root" },
    { "jmp", CfrTil_Goto, IMMEDIATE, 0, "Asm", "Root" },

    { "strlen", CfrTil_StrLen, 0, 0, "String", "Root" },
    { "strcmp", CfrTil_StrCmp, 0, 0, "String", "Root" },
    { "strcat", CfrTil_StrCat, 0, 0, "String", "Root" },
    { "strcpy", CfrTil_StrCpy, 0, 0, "String", "Root" },
    { "stricmp", CfrTil_StrICmp, 0, 0, "String", "Root" },

    { "\"", Context_DoubleQuoteMacro, IMMEDIATE | KEYWORD, T_LISP_TERMINATING_MACRO, "Lexer", "Root" },
    { "%", MultipleEscape, IMMEDIATE, 0, "Lexer", "Root" },

    { "printString", CfrTil_PrintString, 0, 0, "Io", "Root" },
    { "emit", CfrTil_Emit, 0, 0, "Io", "Root" },
    { "key", CfrTil_Key, 0, 0, "Io", "Root" },
    { "kbhit", CfrTil_Kbhit, 0, 0, "Io", "Root" },
    { "newline", CfrTil_NewLine, 0, 0, "Io", "Root" },
    { "cr", CfrTil_CarriageReturn, 0, 0, "Io", "Root" },
    { "tab", CfrTil_TAB, 0, 0, "Io", "Root" },
    { "space", CfrTil_SPACE, 0, 0, "Io", "Root" },
    { "logAppend", CfrTil_LogAppend, 0, 0, "Io", "Root" },
    { "logWrite", CfrTil_LogWrite, 0, 0, "Io", "Root" },
    { "logOn", CfrTil_LogOn, 0, 0, "Io", "Root" },
    { "logOff", CfrTil_LogOff, 0, 0, "Io", "Root" },

    //{ "nprintf", CfrTil_printf, PREFIX, 0, "Io", "Root" },

    { "dump", CfrTil_Dump, 0, 0, "Debug", "Root" },
    { "dis", CfrTil_Disassemble, 0, 0, "Debug", "Root" },
    { "wordDisassemble", CfrTil_Word_Disassemble, 0, 0, "Debug", "Root" },
    { "sourceAdd", CfrTil_Source_AddToHistory, 0, 0, "Debug", "Root" },
    { "source", CfrTil_Source_AddToHistory, 0, 0, "Debug", "Root" },
    { "sourceDontAdd", CfrTil_Source_DontAddToHistory, 0, 0, "Debug", "Root" },
    { "stack", CfrTil_PrintDataStack, 0, 0, "Debug", "Root" },
    { "rstack", CfrTil_PrintReturnStack, 0, 0, "Debug", "Root" },
    { "nrstack", CfrTil_PrintNReturnStack, 0, 0, "Debug", "Root" },
    { "_d:", CfrTil_DebugModeOn, DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    { "d:", CfrTil_DebugModeOn, IMMEDIATE | DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    { ";d", CfrTil_DebugModeOff, IMMEDIATE | DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    { "_dbg", CfrTil_Debug, DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    { "<dbg>", CfrTil_DebugRuntimeBreakpoint, DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    { "xtDbg", CfrTil_Debug_AtAddress, DEBUG_WORD | INTERPRET_DBG, 0, "Debug", "Root" },
    //{ "regs", CfrTil_CpuState_Show, DEBUG_WORD, 0, "Debug", "Root" },
    { "_info", _CfrTil_DebugInfo, DEBUG_WORD, 0, "Debug", "Root" },
    { "info", CfrTil_DebugInfo, DEBUG_WORD, 0, "Debug", "Root" },
    { "localsShow", CfrTil_LocalsShow, DEBUG_WORD, 0, "Debug", "Root" },
    { "dbgVerbosity", CfrTil_Debugger_Verbosity, DEBUG_WORD, 0, "Debug", "Root" },

    { "lisp", ( block ) LO_Repl, 0, 0, "System", "Root" },
    { "include", CfrTil_IncludeFile, 0, 0, "System", "Root" },
    { "reset", CfrTil_WarmInit, 0, 0, "System", "Root" },
    { "stackInit", CfrTil_DataStack_Init, 0, 0, "System", "Root" },
    { "stackDepth", CfrTil_DataStack_Size, 0, 0, "System", "Root" },
    { "filename", CfrTil_Filename, 0, 0, "System", "Root" },
    //{ "try", CfrTil_Try, 0, 0, "System", "Root" },
    //{ "catch", CfrTil_Catch, 0, 0, "System", "Root" },
    { "throw", CfrTil_Throw, 0, 0, "System", "Root" },
    { "lineNumber", CfrTil_LineNumber, 0, 0, "System", "Root" },
    { "charNumber", CfrTil_LineCharacterNumber, 0, 0, "System", "Root" },
    { "version", CfrTil_Version, 0, 0, "System", "Root" },
    { "decimal", CfrTil_Decimal, 0, 0, "System", "Root" },
    { "hex", CfrTil_Hex, 0, 0, "System", "Root" },
    { "binary", CfrTil_Binary, 0, 0, "System", "Root" },
    { "state", CfrTil_SystemState_Print, 0, 0, "System", "Root" },
    { "variables", CfrTil_Variables, 0, 0, "System", "Root" },
    //{ "verboseOn", CfrTil_VerboseOn, 0, 0, "System", "Root" },
    { "echoOff", CfrTil_EchoOff, 0, 0, "System", "Root" },
    { "echoOn", CfrTil_EchoOn, 0, 0, "System", "Root" },
    { "echoOff", CfrTil_EchoOff, 0, 0, "System", "Root" },
    { "echo", CfrTil_Echo, IMMEDIATE, 0, "System", "Root" },
    //{ "optimize", _CfrTil_SystemState_Print, 0, 0, "System", "Root" },
    { "timeInit", CfrTil_InitTime, 0, 0, "System", "Root" },
    { "timerInit", CfrTil_TimerInit, 0, 0, "System", "Root" },
    { "time", CfrTil_Time, 0, 0, "System", "Root" },
    //{ "shellFork", CfrTil_ShellForkKonsole, 0, 0, "System", "Root" },
    { "shell", CfrTil_ShellEscape, 0, 0, "System", "Root" },
    { "system0", CfrTil_system0, 0, 0, "System", "Root" },
    { "system1", CfrTil_system1, 0, 0, "System", "Root" },
    { "system2", CfrTil_system2, 0, 0, "System", "Root" },
    { "system3", CfrTil_system3, 0, 0, "System", "Root" },
    { "dlsym", _CfrTil_dlsym, 0, 0, "System", "Root" },
    { "returnFromFile", CfrTil_ReturnFromFile, 0, 0, "System", "Root" },
    { "allocateMem", CfrTil_AllocateNew, 0, 0, "System", "Root" },
    { "freeObjMem", OVT_MemListFree_Objects, 0, 0, "System", "Root" },
    { "mem", CfrTil_MemoryAllocated, 0, 0, "System", "Root" },
    { "resetAll", CfrTil_ResetAll, 0, 0, "System", "Root" },
    { "bye", CfrTil_Exit, 0, 0, "System", "Root" },
    { "abort", CfrTil_Abort, 0, 0, "System", "Root" },
    { "stop", CfrTil_DebugStop, 0, 0, "System", "Root" },
    { "pause", OpenVmTil_Pause, 0, 0, "System", "Root" },
    { "quit", CfrTil_Quit, 0, 0, "System", "Root" },
    { "freeSessionMem", OVT_MemListFree_Session, 0, 0, "System", "Root" },
    { "systemBreak", CfrTil_SystemBreak, 0, 0, "System", "Root" },
    { "exit", CfrTil_Exit, 0, 0, "System", "Root" },
    { "restart", CfrTil_Restart, 0, 0, "System", "Root" },
    { "restartInit", CfrTil_RestartInit, 0, 0, "System", "Root" },
    { "ri", CfrTil_RestartInit, 0, 0, "System", "Root" },
    { "fr", CfrTil_FullRestart, 0, 0, "System", "Root" },
    { "warmInit", CfrTil_WarmInit, 0, 0, "System", "Root" },
    { "ok", CfrTil_Ok, 0, 0, "System", "Root" },
    { "console", Console, 0, 0, "System", "Root" },
    { "cpuStateShow", CfrTil_CpuState_Show, 0, 0, "System", "Root" },
    //{ "call_a7", call_a7, 0, 0, "System", "Root" },
    //{ "root", CfrTil_Namespaces_Root, 0, 0, "System", "Root" },

    { "fopen", File_Open, 0, 0, "File", "System" },
    { "fclose", File_Close, 0, 0, "File", "System" },
    { "fwrite", File_Write, 0, 0, "File", "System" },
    { "fread", File_Read, 0, 0, "File", "System" },
    { "fseek", File_Seek, 0, 0, "File", "System" },
    { "ftell", File_Tell, 0, 0, "File", "System" },
    { "fexists", File_Exists, 0, 0, "File", "System" },

    { "setDefaultRGB", CfrTil_SetDefaultRGB, 0, 0, "Colors", "System" },
    { "setAlertRGB", CfrTil_SetAlertRGB, 0, 0, "Colors", "System" },
    { "setDebugRGB", CfrTil_SetDebugRGB, 0, 0, "Colors", "System" },
    { "setUserRGB", CfrTil_SetDebugRGB, 0, 0, "Colors", "System" },
    { "setNoticeRGB", CfrTil_SetNoticeRGB, 0, 0, "Colors", "System" },
    { "setUserColors", CfrTil_SetUserColors, 0, 0, "Colors", "System" },
    { "setDefaultColors", CfrTil_SetDefaultColors, 0, 0, "Colors", "System" },
    { "setAlertColors", CfrTil_SetAlertColors, 0, 0, "Colors", "System" },
    { "setDebugColors", CfrTil_SetDebugColors, 0, 0, "Colors", "System" },
    { "setNoticeColors", CfrTil_SetNoticeColors, 0, 0, "Colors", "System" },
    { "debugColors", Ovt_DebugColors, 0, 0, "Colors", "System" },
    { "noticeColors", Ovt_NoticeColors, 0, 0, "Colors", "System" },
    { "defaultColors", Ovt_DefaultColors, 0, 0, "Colors", "System" },
    { "userColors", Ovt_UserColors, 0, 0, "Colors", "System" },
    { "alertColors", Ovt_AlertColors, 0, 0, "Colors", "System" },

    //{ "contextNewRun", CfrTil_Contex_New_RunWord, 0, 0, "Context", "Root" },
    //{ "contextInit", CfrTil_ContextInit, 0, 0, "Context", "Root" },

    { "autoVar", Ovt_AutoVar, 0, 0, "OpenVmTil", "Root" },
    { "autoVarOn", Ovt_AutoVarOn, 0, 0, "OpenVmTil", "Root" },
    { "autoVarOff", Ovt_AutoVarOff, 0, 0, "OpenVmTil", "Root" },
    { "optimize", Ovt_Optimize, 0, 0, "OpenVmTil", "Root" },
    { "inlining", Ovt_Inlining, 0, 0, "OpenVmTil", "Root" },
    { "optimizeOn", CfrTil_OptimizeOn, 0, 0, "OpenVmTil", "Root" },
    { "optimizeOff", CfrTil_OptimizeOff, 0, 0, "OpenVmTil", "Root" },
    { "stringMacrosOn", CfrTil_StringMacrosOn, 0, 0, "OpenVmTil", "Root" },
    { "stringMacrosOff", CfrTil_StringMacrosOff, 0, 0, "OpenVmTil", "Root" },
    { "inlineOff", CfrTil_InlineOff, 0, 0, "OpenVmTil", "Root" },
    { "inlineOn", CfrTil_InlineOn, 0, 0, "OpenVmTil", "Root" },
    { "debugOff", CfrTil_DebugModeOff, 0, 0, "OpenVmTil", "Root" },
    { "debugOn", CfrTil_DebugModeOn, 0, 0, "OpenVmTil", "Root" },
    { "verbosity", OpenVmTil_Verbosity, 0, 0, "OpenVmTil", "Root" },
    { "codeSize", OpenVmTil_CodeSize, 0, 0, "OpenVmTil", "Root" },
    { "objectsSize", OpenVmTil_ObjectsSize, 0, 0, "OpenVmTil", "Root" },
    { "compilerTempObjectsSize", OpenVmTil_CompilerTempObjectsSize, 0, 0, "OpenVmTil", "Root" },
    { "sessionObjectsSize", OpenVmTil_SessionObjectsSize, 0, 0, "OpenVmTil", "Root" },
    { "dictionarySize", OpenVmTil_DictionarySize, 0, 0, "OpenVmTil", "Root" },
    { "dataStackSize", OpenVmTil_DataStackSize, 0, 0, "OpenVmTil", "Root" },
    { "historySize", OpenVmTil_HistorySize, 0, 0, "OpenVmTil", "Root" },
    { "addToHistory", OpenVmTil_AddStringToHistory, 0, 0, "OpenVmTil", "Root" },
    { "historyReset", HistorySpace_Reset, 0, 0, "OpenVmTil", "Root" },
    { "addToHistoryOn", OpenVmTil_AddStringToHistoryOn, 0, 0, "OpenVmTil", "Root" },
    { "addToHistoryOff", OpenVmTil_AddStringToHistoryOff, 0, 0, "OpenVmTil", "Root" },
    { "showAllocated", OVT_Mem_ShowAllocated, 0, 0, "OpenVmTil", "Root" },

    { "dup", CfrTil_Dup, IMMEDIATE | STACKING | CATEGORY_DUP, 0, "Stack", "Root" },
    { "ndup", CfrTil_NDup, STACKING, 0, "Stack", "Root" },
    { "drop", CfrTil_Drop, IMMEDIATE | STACKING, 0, "Stack", "Root" },
    { "dropN", CfrTil_DropN, IMMEDIATE | STACKING, 0, "Stack", "Root" },
    { "ndrop", CfrTil_DropN, IMMEDIATE, 0, "Stack", "Root" },
    { "pick", CfrTil_Pick, IMMEDIATE | STACKING, 0, "Stack", "Root" },
    { "_swap", CfrTil_Swap, STACKING, 0, "Stack", "Root" },
    { "dsp", CfrTil_Dsp, 0, 0, "Stack", "Root" },
    { "stackCheck", DataStack_Check, 0, 0, "Stack", "Root" },

    { "compileMode_get", CfrTil_CompileMode, 0, 0, "Interpreter", "Root" },
    { "compileModeOn", CfrTil_RightBracket, 0, 0, "Interpreter", "Root" },
    { "compileModeOff", CfrTil_LeftBracket, 0, 0, "Interpreter", "Root" },
    { "//", CfrTil_CommentToEndOfLine, IMMEDIATE, 0, "Interpreter", "Root" },
    { "*/", CfrTil_NoOp, IMMEDIATE, 0, "Interpreter", "Root" },
    { "/*", CfrTil_ParenthesisComment, IMMEDIATE, 0, "Interpreter", "Root" },
    { "eval", CfrTil_Interpreter_EvalWord, 0, 0, "Interpreter", "Root" },
    { "literalInterpret", CfrTil_InterpretALiteralToken, 0, 0, "Interpreter", "Root" },
    { "interpretString", CfrTil_InterpretString, 0, 0, "Interpreter", "Root" },
    { "interpretNextToken", CfrTil_InterpretNextToken, 0, 0, "Interpreter", "Root" },
    { "interpreterRun", CfrTil_InterpreterRun, 0, 0, "Interpreter", "Root" },
    { "interpreterInit", CfrTil_Interpreter_Init, 0, 0, "Interpreter", "Root" },
    { "interpreterStop", CfrTil_InterpreterStop, 0, 0, "Interpreter", "Root" },
    { "interpreterIsDone", CfrTil_Interpreter_IsDone, 0, 0, "Interpreter", "Root" },
    { "interpreterDone", CfrTil_Interpreter_Done, 0, 0, "Interpreter", "Root" },
    { "interpret", CfrTil_InterpreterRun, 0, 0, "Interpreter", "Root" },
    { "interpretFile", CfrTil_Interpret, 0, 0, "Interpreter", "Root" },
    { "#", CfrTil_CPreProcessor, IMMEDIATE, 0, "Interpreter", "Root" },
    { "!", CfrTil_CommentToEndOfLine, 0, 0, "PreProcessor", "C" }, // in init.cft ; needed for script files starting with #! cfrtil -s
    { "endif", CfrTil_NoOp, IMMEDIATE, 0, "PreProcessor", "C" },
    { "else", CfrTil_Else_ConditionalInterpret, IMMEDIATE, 0, "PreProcessor", "C" },
    { "if", CfrTil_If_ConditionalInterpret, IMMEDIATE, 0, "PreProcessor", "C" },
    { "elif", CfrTil_If_ConditionalInterpret, IMMEDIATE, 0, "PreProcessor", "C" },
    { "(", Interpret_DoParenthesizedRValue, IMMEDIATE | KEYWORD, 0, "PreProcessor", "C" },

    { "s:", CfrTil_StringMacro, 0, 0, "Macro", "Root" },
    { "alias", CfrTil_Alias, 0, 0, "Macro", "Root" },
    { "t:", CfrTil_TextMacro, 0, 0, "Macro", "Root" },

    { "name", Word_Name, 0, 0, "Word", "Root" },
    { "create", CfrTil_Word_Create, 0, 0, "Word", "Root" },
    { "wNamespace", Word_Namespace, 0, 0, "Word", "Root" },
    { "location", Word_Location, 0, 0, "Word", "Root" },
    { "wordEval", Word_Eval, 0, 0, "Word", "Root" },
    { "wordRun", Word_Run, 0, 0, "Word", "Root" },
    { "definition", Word_Definition, 0, 0, "Word", "Root" },
    { "value", Word_Value, 0, 0, "Word", "Root" },
    { "xt", Word_Xt, 0, 0, "Word", "Root" },
    //{ "xt@", Word_Definition, IMMEDIATE, 0, "Word", "Root" },
    { "xt!", Word_DefinitionStore, 0, 0, "Word", "Root" },
    { "wordFinish", Word_Finish, 0, 0, "Word", "Root" },
    { "wordAdd", Word_Add, 0, 0, "Word", "Root" },
    { "codeStart", Word_CodeStart, 0, 0, "Word", "Root" },
    { "codeSize", Word_CodeSize, 0, 0, "Word", "Root" },
    { "addrToWord", AddressToWord, 0, 0, "Word", "Root" },
    { "setupWordEval", CfrTil_Setup_WordEval, 0, 0, "Word", "Root" },

    // Dynamic Object - DObject
    { "dobject", CfrTil_DObject, 0, 0, "DObject", "Root" },
    { "clone", CfrTil_DObject_Clone, 0, 0, "DObject", "Root" },
    { "new", CfrTil_DObject_New, 0, 0, "DObject", "Root" },
    //{ "object", CfrTil_DObject, 0, 0, "DObject", "Root" },

    { "word", CfrTil_Word, 0, 0, "Reserved", "Compiler" },
    { "_dlsymWord", CfrTil_DlsymWord, 0, 0, "Reserved", "Compiler" },
    { "dlsym:", CfrTil_Dlsym, 0, 0, "Reserved", "Compiler" },
    { ":", CfrTil_Colon, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" },
    { ";", CfrTil_SemiColon, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" },
    { "}", CfrTil_EndBlock, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" }, // moved to init.cft and renamed below
    { "{", ( block ) CfrTil_BeginBlock, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" }, // moved to init.cft and renamed below
    { "end", CfrTil_EndBlock, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" },
    { "immediate", CfrTil_Immediate, IMMEDIATE | KEYWORD, 0, "Reserved", "Compiler" },
    { "keyword", CfrTil_Keyword, IMMEDIATE, 0, "Reserved", "Compiler" },
    { "swap", CfrTil_Swap, IMMEDIATE | STACKING, 0, "Reserved", "Compiler" },
    { "|}", CfrTil_RightBracket, IMMEDIATE, 0, "Reserved", "Compiler" },
    { "{|", CfrTil_LeftBracket, IMMEDIATE, 0, "Reserved", "Compiler" },

    //{ "tail", CfrTil_Tail, IMMEDIATE | CATEGORY_TAIL | CATEGORY_UNMOVEABLE, 0, "Compiler", "Root" },
    { "recurse", CfrTil_SetupRecursiveCall, IMMEDIATE | CATEGORY_RECURSIVE, 0, "Compiler", "Root" }, // put this here so "Compiler" will be in "Root"
    { "goto", CfrTil_Goto, IMMEDIATE, 0, "Compiler", "Root" },
    { "continue", CfrTil_Continue, IMMEDIATE, 0, "Compiler", "Root" },
    { "return", CfrTil_Return, IMMEDIATE, 0, "Compiler", "Root" },
    { "break", CfrTil_Break, IMMEDIATE, 0, "Compiler", "Root" },
    { "noop", CfrTil_NoOp, IMMEDIATE, 0, "Compiler", "Root" },
    { "macro", CfrTil_Word, IMMEDIATE, 0, "Compiler", "Root" },
    { ":recursive", CfrTil_BeginRecursiveWord, 0, 0, "Compiler", "Root" },
    { "recursive;", CfrTil_EndRecursiveWord, IMMEDIATE, 0, "Compiler", "Root" },
    { "variable", CfrTil_Variable, 0, 0, "Compiler", "Root" },
    { "label", CfrTil_Label, IMMEDIATE, 0, "Compiler", "Root" },
    { "constant", CfrTil_Constant, 0, 0, "Compiler", "Root" },
    { "_literal", CfrTil_Literal, 0, 0, "Compiler", "Root" },
    { "literal", CfrTil_Literal, IMMEDIATE, 0, "Compiler", "Root" },
    { "_end", CfrTil_EndBlock, 0, 0, "Compiler", "Root" },
    { "begin", ( block ) CfrTil_BeginBlock, 0, 0, "Compiler", "Root" },
    { "here", CfrTil_Here, 0, 0, "Compiler", "Root" },
    { "_immediate", CfrTil_Immediate, 0, 0, "Compiler", "Root" },
    { "?immediate", CfrTil_IsImmediate, INFIXABLE, 0, "Compiler", "Root" },
    { "inline", CfrTil_Inline, IMMEDIATE, 0, "Compiler", "Root" },
    { "_prefix", CfrTil_Prefix, 0, 0, "Compiler", "Root" },
    { "prefix", CfrTil_Prefix, IMMEDIATE, 0, "Compiler", "Root" },
    { "infixOn", CfrTil_InfixModeOn, IMMEDIATE, 0, "Compiler", "Root" },
    { "infixOff", CfrTil_InfixModeOff, IMMEDIATE, 0, "Compiler", "Root" },
    { "prefixOn", CfrTil_PrefixModeOn, IMMEDIATE, 0, "Compiler", "Root" },
    { "prefixOff", CfrTil_PrefixModeOff, IMMEDIATE, 0, "Compiler", "Root" },
    { "debugWord", CfrTil_DebugWord, 0, 0, "Compiler", "Root" },
    { "@", CfrTil_Peek, IMMEDIATE | CATEGORY_OP_LOAD, 0, "Compiler", "Root" },
    { "load", CfrTil_Peek, IMMEDIATE | CATEGORY_OP_LOAD, 0, "Compiler", "Root" },
    { "@=", CfrTil_AtEqual, IMMEDIATE | CATEGORY_OP_ORDERED, 0, "Compiler", "Root" },
    //{ "at", CfrTil_Peek, CATEGORY_FETCH, 0, "Compiler", "Root" },
    { "peek", CfrTil_Peek, CATEGORY_OP_LOAD, 0, "Compiler", "Root" },
    { "!", CfrTil_Store, IMMEDIATE | CATEGORY_OP_STORE, 0, "Compiler", "Root" },
    { "=", CfrTil_Poke, IMMEDIATE | CATEGORY_OP_EQUAL | INFIXABLE | KEYWORD, 0, "Compiler", "Root" },
    { "poke", CfrTil_Poke, IMMEDIATE | CATEGORY_OP_EQUAL, 0, "Compiler", "Root" },
    { "store", CfrTil_Store, IMMEDIATE | CATEGORY_OP_STORE, 0, "Compiler", "Root" },
    { "'", CfrTil_Tick, IMMEDIATE | KEYWORD, 0, "Compiler", "Root" },
    { "tick", CfrTil_Tick, IMMEDIATE | KEYWORD, 0, "Compiler", "Root" },
    { "_tick", _CfrTil_Tick, KEYWORD, 0, "Compiler", "Root" },
    //{ "\"", CfrTil_DoubleQuote, IMMEDIATE, 0, "Compiler", "Root" },
    //{ "quote", CfrTil_DoubleQuote, 0, 0, "Compiler", "Root" },
    { "token", CfrTil_Token, 0, 0, "Compiler", "Root" },
    { "sourceCodeInit", CfrTil_SourceCode_Init, 0, 0, "Compiler", "Root" },
    { "sourceCodeOn", CfrTil_Lexer_SourceCodeOn, 0, 0, "Compiler", "Root" },
    { ",", CompileInt32, 0, 0, "Compiling", "Compiler" },
    { "4,", CompileInt32, 0, 0, "Compiling", "Compiler" },
    { "8,", CompileInt64, 0, 0, "Compiling", "Compiler" },
    { "1,", CompileByte, 0, 0, "Compiling", "Compiler" },
    { "compileByte", CompileByte, 0, 0, "Compiling", "Compiler" },
    { "2,", CompileInt16, 0, 0, "Compiling", "Compiler" },
    { "n,", CompileN, 0, 0, "Compiling", "Compiler" },
    { "_compileCall", CompileCall, 0, 0, "Compiling", "Compiler" },
    { "_compileWord", CompileACfrTilWord, 0, 0, "Compiling", "Compiler" },

    { 0 }
} ;

MachineCodePrimitive MachineCodePrimitives [] = {
    { "getESP", CPRIMITIVE, 0, ( byte* ) _Compile_Debug_GetESP, 0, "System", "Root" },
    { "restoreCpuState", CPRIMITIVE, 0, ( byte* ) _Compile_CpuState_Restore, 0, "System", "Root" },
    { "saveCpuState", CPRIMITIVE, 0, ( byte* ) _Compile_CpuState_Save, 0, "System", "Root" },
    { "saveCpuState", CPRIMITIVE, 0, ( byte* ) _Compile_CpuState_Save, 0, "Debug", "Root" },
    { "<dbg>", CFRTIL_WORD | DEBUG_WORD | INTERPRET_DBG, 0, ( byte* ) _Compile_Debug1, - 1, "Debug", "Root" },
    { "rsp", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_Get, - 1, "System", "Root" },
    { "rsp@", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_Fetch, - 1, "System", "Root" },
    { ">rsp", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_To, - 1, "System", "Root" },
    { "rsp>", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_From, - 1, "System", "Root" },
    { "rdrop", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_Drop, - 1, "System", "Root" },
    { "rsp!", CFRTIL_WORD, 0, ( byte* ) _Compile_Rsp_Store, - 1, "System", "Root" },
    { "pushEAX", CFRTIL_WORD, 0, ( byte* ) Compile_DataStack_PushEAX, - 1, "System", "Root" },
#if NO_GLOBAL_REGISTERS  // NGR NO_GLOBAL_REGISTERS    
    { "DspToESI", CFRTIL_WORD, 0, ( byte* ) _Compile_Dsp_To_ESI, - 1, "System", "Root" },
    { "ESItoDsp", CFRTIL_WORD, 0, ( byte* ) _Compile_ESI_To_Dsp, - 1, "System", "Root" },
#endif    
    { 0 }
} ;


