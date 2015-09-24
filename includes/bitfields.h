typedef unsigned short bit ;
struct _T_CType0
{
    bit Cprimitive : 1; // ( (uint64) 1 << 0 )
    bit Cfrtil_word : 1; // ( (uint64) 1 << 1 )
    bit T_raw_string : 1; // ( (uint64) 1 << 2 )
    bit T_string : 1; // ( (uint64) 1 << 3 )
    bit T_int : 1; // ( (uint64) 1 << 4 )
    bit T_float : 1; // ( (uint64) 1 << 5 )
    bit T_big_int : 1; // ( (uint64) 1 << 6 )
    bit T_big_float : 1; // ( (uint64) 1 << 7 )
    bit T_head : 1; // ( (uint64) 1 << 8 )
    bit T_tail : 1; // ( (uint64) 1 << 9 )
    bit Literal : 1; // ( (uint64) 1 << 10 )

    // CType - C forth types
    bit Combinator : 1; // ( (uint64) 1 << 11 )
    bit Immediate : 1; // ( (uint64) 1 << 12 )
    bit Namespace : 1; // ( (uint64) 1 << 13 )
    bit Block : 1; // ( (uint64) 1 << 14 )
    bit Inline : 1; // ( (uint64) 1 << 15 )
    bit Class : 1; // ( (uint64) 1 << 16 )
    bit Variable : 1; // ( (uint64) 1 << 17 )
    bit Stack_variable : 1; // ( (uint64) 1 << 18 )
    bit Local_variable : 1; // ( (uint64) 1 << 19 )
    bit Prefix : 1; // ( (uint64) 1 << 20 )
    bit Infixable : 1; // ( (uint64) 1 << 21 )
    bit Object : 1; // ( (uint64) 1 << 22 )
    bit Constant : 1; // ( (uint64) 1 << 23 )
    bit Category_stack : 1; // ( (uint64) 1 << 24 )
    bit Register_variable : 1; // ( (uint64) 1 << 25 )
    bit This : 1; // ( (uint64) 1 << 26 )
    bit Object_field : 1; // ( (uint64) 1 << 27 )
#if 0
    union
    {
        bit List_function : 1; // ( (uint64) 1 << 28 )
        bit Lisp_word : 1;
    };
#else
    bit List_function : 1; // ( (uint64) 1 << 28 )
#endif    
    bit Category_op_store : 1; // ( (uint64) 1 << 29 )
    bit Category_dup : 1; // ( (uint64) 1 << 30 )
    bit Category_op_ordered : 1; // ( (uint64) 1 << 31 )
    bit Category_op_load : 1; // ( (uint64) 1 << 32 )
    bit Category_op_equal : 1; // ( (uint64) 1 << 33 )
    bit Category_logic : 1; // ( (uint64) 1 << 34 )
    bit Category_op_1_arg : 1; // ( (uint64) 1 << 35 )
    bit Category_recursive : 1; // ( (uint64) 1 << 36 )
    bit Category_op_unordered : 1; // ( (uint64) 1 << 37 )
    bit Category_op_stack : 1; // ( (uint64) 1 << 38 )
    bit Category_op_divide : 1; // ( (uint64) 1 << 39 )
    bit C_prefix : 1; // ( (uint64) 1 << 40 )
    bit Keyword : 1; // ( (uint64) 1 << 41 )
#if 0
    union
    {
        bit Dynamic_object : 1; // ( (uint64) 1 << 42 )
        bit Dobject : 1;
    };
#else
    bit Dynamic_object : 1; // ( (uint64) 1 << 42 )
#endif    
    bit C_prefix_rtl_args : 1; // ( (uint64) 1 << 43 )
    bit Alias : 1; // ( (uint64) 1 << 44 )
    bit Text_macro : 1; // ( (uint64) 1 << 45 )
    bit String_macro : 1; // ( (uint64) 1 << 46 )
    bit History_node : 1; // ( (uint64) 1 << 47 )
    bit Debug_word : 1; // ( (uint64) 1 << 48 )
    bit Rt_stack_op : 1; // ( (uint64) 1 << 49 ) // Run Time stack operation
    bit Word_create : 1; // ( (uint64) 1 << 50 ) // Run Time stack operation
    bit Interpret_dbg : 1; // ( (uint64) 1 << 51 ) 
    bit Infix_word : 1; // ( (uint64) 1 << 52 ) 
    bit C_return : 1; // ( (uint64) 1 << 53 ) 
    bit Dlsym_word : 1; // ( (uint64) 1 << 54 ) 
    bit C_type : 1; // ( (uint64) 1 << 55 ) 
    bit Class_clone : 1; // ( (uint64) 1 << 56 ) 
    bit C_class : 1; // ( (uint64) 1 << 57 ) 
    bit C_typedef : 1; // ( (uint64) 1 << 58 ) 
    bit Dobject : 1; // ( (uint64) 1 << 59 ) 
    bit Lisp_word : 1; // ( (uint64) 1 << 60 ) 
};

struct LType0
{
    // LType - lisp types
bit T_unassigned0 : 10;
    bit T_lambda : 1; // ( (uint64) 1 << 11 )
    bit T_cons : 1; // ( (uint64) 1 << 12 )
    bit List : 1; // ( (uint64) 1 << 13 )
    bit Lisp_c_rtl_args : 1; // ( (uint64) 1 << 14 )
    bit List_object : 1; // ( (uint64) 1 << 15 )
    bit List_quotation : 1; // ( (uint64) 1 << 16 )
    bit T_lisp_if : 1; // ( (uint64) 1 << 17 )
    bit T_lisp_set : 1; // ( (uint64) 1 << 18 )
    bit List_node : 1; // ( (uint64) 1 << 19 )
    bit T_lisp_special : 1; // ( (uint64) 1 << 20 )
    bit T_lisp_begin : 1; // ( (uint64) 1 << 21 )
    bit T_lisp_token : 1; // ( (uint64) 1 << 22 )
    bit T_lisp_define : 1; // ( (uint64) 1 << 23 )
    bit T_lisp_macro : 1; // ( (uint64) 1 << 24 )
bit T_unassigned1 : 1;
    bit Lisp_void_return : 1; // ( (uint64) 1 << 26 )
    bit T_nil : 1; // ( (uint64) 1 << 27 )
    bit List_function : 1; // ((uint64) 1 << 28)
    bit T_lisp_compiled_word : 1; // ( (uint64) 1 << 29 )
    bit T_lisp_compile : 1; // ( (uint64) 1 << 30 )
    bit T_lisp_let : 1; // ( (uint64) 1 << 31 )
    bit T_lisp_terminating_macro : 1; // ( (uint64) 1 << 32 )
    bit T_lisp_read_macro : 1; // ( (uint64) 1 << 33 )
    //#define T_lisp_quasiquote ( (uint64) 1 << 34 )
    bit T_lisp_locals_arg : 1; // ( (uint64) 1 << 35 )
    //#define T_lisp_unquote ( (uint64) 1 << 35 )
    bit T_lisp_unquote_splicing : 1; // ( (uint64) 1 << 36 )
    bit T_lisp__define : 1; // ( (uint64) 1 << 37 )
    //#define T_lisp_defmacro ( (uint64) 1 << 37 )
    bit T_lisp_value : 1; // ( (uint64) 1 << 38 )
    bit T_lisp_word : 1; // ( (uint64) 1 << 39 )
bit T_unassigned2 : 4;
    bit T_lisp_symbol : 1; // ( (uint64) 1 << 44 )
    bit Lisp_word : 1 ; // ((uint64) 1 << 45)
};

typedef struct 
{
    unsigned int AddMemChunk : 1 ;
    unsigned int AddToList : 1 ;
    unsigned int InitMemChunk : 1 ;
    unsigned int ReturnMemChunk : 1 ;
    unsigned int Account : 1 ;
} AllocFlags ;
#define ADD_MEM_CHUNK ( (uint) 1 << 0 ) 
#define ADD_TO_LIST ( (uint) 1 << 1 ) 
#define INIT_MEM_CHUNK ( (uint) 1 << 2 ) 
#define RETURN_MEM_CHUNK ( (uint) 1 << 3 ) 
#define MEM_CHUNK_ACCOUNT ( (uint) 1 << 4 ) 
