
struct _T_CType0
{
    unsigned int Cprimitive : 1; // ( (uint64) 1 << 0 )
    unsigned int Cfrtil_word : 1; // ( (uint64) 1 << 1 )
    unsigned int T_raw_string : 1; // ( (uint64) 1 << 2 )
    unsigned int T_string : 1; // ( (uint64) 1 << 3 )
    unsigned int T_int : 1; // ( (uint64) 1 << 4 )
    unsigned int T_float : 1; // ( (uint64) 1 << 5 )
    unsigned int T_big_int : 1; // ( (uint64) 1 << 6 )
    unsigned int T_big_float : 1; // ( (uint64) 1 << 7 )
    unsigned int T_head : 1; // ( (uint64) 1 << 8 )
    unsigned int T_tail : 1; // ( (uint64) 1 << 9 )
    unsigned int Literal : 1; // ( (uint64) 1 << 10 )

    // CType - C forth types
    unsigned int Combinator : 1; // ( (uint64) 1 << 11 )
    unsigned int Immediate : 1; // ( (uint64) 1 << 12 )
    unsigned int Namespace : 1; // ( (uint64) 1 << 13 )
    unsigned int Block : 1; // ( (uint64) 1 << 14 )
    unsigned int Inline : 1; // ( (uint64) 1 << 15 )
    unsigned int Class : 1; // ( (uint64) 1 << 16 )
    unsigned int Variable : 1; // ( (uint64) 1 << 17 )
    unsigned int Stack_variable : 1; // ( (uint64) 1 << 18 )
    unsigned int Local_variable : 1; // ( (uint64) 1 << 19 )
    unsigned int Prefix : 1; // ( (uint64) 1 << 20 )
    unsigned int Infixable : 1; // ( (uint64) 1 << 21 )
    unsigned int Object : 1; // ( (uint64) 1 << 22 )
    unsigned int Constant : 1; // ( (uint64) 1 << 23 )
    unsigned int Category_stack : 1; // ( (uint64) 1 << 24 )
    unsigned int Register_variable : 1; // ( (uint64) 1 << 25 )
    unsigned int This : 1; // ( (uint64) 1 << 26 )
    unsigned int Object_field : 1; // ( (uint64) 1 << 27 )
#if 0
    union
    {
        unsigned int List_function : 1; // ( (uint64) 1 << 28 )
        unsigned int Lisp_word : 1;
    };
#else
    unsigned int List_function : 1; // ( (uint64) 1 << 28 )
#endif    
    unsigned int Category_op_store : 1; // ( (uint64) 1 << 29 )
    unsigned int Category_dup : 1; // ( (uint64) 1 << 30 )
    unsigned int Category_op_ordered : 1; // ( (uint64) 1 << 31 )
    unsigned int Category_op_load : 1; // ( (uint64) 1 << 32 )
    unsigned int Category_op_equal : 1; // ( (uint64) 1 << 33 )
    unsigned int Category_logic : 1; // ( (uint64) 1 << 34 )
    unsigned int Category_op_1_arg : 1; // ( (uint64) 1 << 35 )
    unsigned int Category_recursive : 1; // ( (uint64) 1 << 36 )
    unsigned int Category_op_unordered : 1; // ( (uint64) 1 << 37 )
    unsigned int Category_op_stack : 1; // ( (uint64) 1 << 38 )
    unsigned int Category_op_divide : 1; // ( (uint64) 1 << 39 )
    unsigned int C_prefix : 1; // ( (uint64) 1 << 40 )
    unsigned int Keyword : 1; // ( (uint64) 1 << 41 )
#if 0
    union
    {
        unsigned int Dynamic_object : 1; // ( (uint64) 1 << 42 )
        unsigned int Dobject : 1;
    };
#else
    unsigned int Dynamic_object : 1; // ( (uint64) 1 << 42 )
#endif    
    unsigned int C_prefix_rtl_args : 1; // ( (uint64) 1 << 43 )
    unsigned int Alias : 1; // ( (uint64) 1 << 44 )
    unsigned int Text_macro : 1; // ( (uint64) 1 << 45 )
    unsigned int String_macro : 1; // ( (uint64) 1 << 46 )
    unsigned int History_node : 1; // ( (uint64) 1 << 47 )
    unsigned int Debug_word : 1; // ( (uint64) 1 << 48 )
    unsigned int Rt_stack_op : 1; // ( (uint64) 1 << 49 ) // Run Time stack operation
    unsigned int Word_create : 1; // ( (uint64) 1 << 50 ) // Run Time stack operation
    unsigned int Interpret_dbg : 1; // ( (uint64) 1 << 51 ) 
    unsigned int Infix_word : 1; // ( (uint64) 1 << 52 ) 
    unsigned int C_return : 1; // ( (uint64) 1 << 53 ) 
    unsigned int Dlsym_word : 1; // ( (uint64) 1 << 54 ) 
    unsigned int C_type : 1; // ( (uint64) 1 << 55 ) 
    unsigned int Class_clone : 1; // ( (uint64) 1 << 56 ) 
    unsigned int C_class : 1; // ( (uint64) 1 << 57 ) 
    unsigned int C_typedef : 1; // ( (uint64) 1 << 58 ) 
    unsigned int Dobject : 1; // ( (uint64) 1 << 59 ) 
    unsigned int Lisp_word : 1; // ( (uint64) 1 << 60 ) 
};

struct LType0
{
    // LType - lisp types
unsigned int T_unassigned0 : 10;
    unsigned int T_lambda : 1; // ( (uint64) 1 << 11 )
    unsigned int T_cons : 1; // ( (uint64) 1 << 12 )
    unsigned int List : 1; // ( (uint64) 1 << 13 )
    unsigned int Lisp_c_rtl_args : 1; // ( (uint64) 1 << 14 )
    unsigned int List_object : 1; // ( (uint64) 1 << 15 )
    unsigned int List_quotation : 1; // ( (uint64) 1 << 16 )
    unsigned int T_lisp_if : 1; // ( (uint64) 1 << 17 )
    unsigned int T_lisp_set : 1; // ( (uint64) 1 << 18 )
    unsigned int List_node : 1; // ( (uint64) 1 << 19 )
    unsigned int T_lisp_special : 1; // ( (uint64) 1 << 20 )
    unsigned int T_lisp_begin : 1; // ( (uint64) 1 << 21 )
    unsigned int T_lisp_token : 1; // ( (uint64) 1 << 22 )
    unsigned int T_lisp_define : 1; // ( (uint64) 1 << 23 )
    unsigned int T_lisp_macro : 1; // ( (uint64) 1 << 24 )
unsigned int T_unassigned1 : 1;
    unsigned int Lisp_void_return : 1; // ( (uint64) 1 << 26 )
    unsigned int T_nil : 1; // ( (uint64) 1 << 27 )
    unsigned int List_function : 1; // ((uint64) 1 << 28)
    unsigned int T_lisp_compiled_word : 1; // ( (uint64) 1 << 29 )
    unsigned int T_lisp_compile : 1; // ( (uint64) 1 << 30 )
    unsigned int T_lisp_let : 1; // ( (uint64) 1 << 31 )
    unsigned int T_lisp_terminating_macro : 1; // ( (uint64) 1 << 32 )
    unsigned int T_lisp_read_macro : 1; // ( (uint64) 1 << 33 )
    //#define T_lisp_quasiquote ( (uint64) 1 << 34 )
    unsigned int T_lisp_locals_arg : 1; // ( (uint64) 1 << 35 )
    //#define T_lisp_unquote ( (uint64) 1 << 35 )
    unsigned int T_lisp_unquote_splicing : 1; // ( (uint64) 1 << 36 )
    unsigned int T_lisp__define : 1; // ( (uint64) 1 << 37 )
    //#define T_lisp_defmacro ( (uint64) 1 << 37 )
    unsigned int T_lisp_value : 1; // ( (uint64) 1 << 38 )
    unsigned int T_lisp_word : 1; // ( (uint64) 1 << 39 )
unsigned int T_unassigned2 : 4;
    unsigned int T_lisp_symbol : 1; // ( (uint64) 1 << 44 )
    unsigned int Lisp_word : 1 ; // ((uint64) 1 << 45)
};