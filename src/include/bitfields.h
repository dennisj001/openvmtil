

struct _T_CProperty0
{
    uint64 Cprimitive : 1; // ( (uint64) 1 << 0 )
    uint64 Cfrtil_word : 1; // ( (uint64) 1 << 1 )
    uint64 T_raw_string : 1; // ( (uint64) 1 << 2 )
    uint64 T_string : 1; // ( (uint64) 1 << 3 )
    uint64 T_int : 1; // ( (uint64) 1 << 4 )
    uint64 T_float : 1; // ( (uint64) 1 << 5 )
    uint64 T_big_int : 1; // ( (uint64) 1 << 6 )
    uint64 T_big_float : 1; // ( (uint64) 1 << 7 )
    uint64 T_head : 1; // ( (uint64) 1 << 8 )
    uint64 T_tail : 1; // ( (uint64) 1 << 9 )
    uint64 Literal : 1; // ( (uint64) 1 << 10 )

    // CProperty - C forth types
    uint64 Combinator : 1; // ( (uint64) 1 << 11 )
    uint64 Immediate : 1; // ( (uint64) 1 << 12 )
    uint64 Namespace : 1; // ( (uint64) 1 << 13 )
    uint64 Block : 1; // ( (uint64) 1 << 14 )
    uint64 Inline : 1; // ( (uint64) 1 << 15 )
    uint64 Class : 1; // ( (uint64) 1 << 16 )
    uint64 Variable : 1; // ( (uint64) 1 << 17 )
    uint64 Stack_variable : 1; // ( (uint64) 1 << 18 )
    uint64 Local_variable : 1; // ( (uint64) 1 << 19 )
    uint64 Prefix : 1; // ( (uint64) 1 << 20 )
    uint64 Infixable : 1; // ( (uint64) 1 << 21 )
    uint64 Object : 1; // ( (uint64) 1 << 22 )
    uint64 Constant : 1; // ( (uint64) 1 << 23 )
    uint64 Category_stack : 1; // ( (uint64) 1 << 24 )
    uint64 Register_variable : 1; // ( (uint64) 1 << 25 )
    uint64 This : 1; // ( (uint64) 1 << 26 )
    uint64 Object_field : 1; // ( (uint64) 1 << 27 )
#if 0
    union
    {
        uint64 List_function : 1; // ( (uint64) 1 << 28 )
        uint64 Lisp_word : 1;
    };
#else
    uint64 List_function : 1; // ( (uint64) 1 << 28 )
#endif    
    uint64 Category_op_store : 1; // ( (uint64) 1 << 29 )
    uint64 Category_dup : 1; // ( (uint64) 1 << 30 )
    uint64 Category_op_ordered : 1; // ( (uint64) 1 << 31 )
    uint64 Category_op_load : 1; // ( (uint64) 1 << 32 )
    uint64 Category_op_equal : 1; // ( (uint64) 1 << 33 )
    uint64 Category_logic : 1; // ( (uint64) 1 << 34 )
    uint64 Category_op_1_arg : 1; // ( (uint64) 1 << 35 )
    uint64 Category_recursive : 1; // ( (uint64) 1 << 36 )
    uint64 Category_op_unordered : 1; // ( (uint64) 1 << 37 )
    uint64 Category_op_stack : 1; // ( (uint64) 1 << 38 )
    uint64 Category_op_divide : 1; // ( (uint64) 1 << 39 )
    uint64 C_prefix : 1; // ( (uint64) 1 << 40 )
    uint64 Keyword : 1; // ( (uint64) 1 << 41 )
#if 0
    union
    {
        uint64 Dynamic_object : 1; // ( (uint64) 1 << 42 )
        uint64 Dobject : 1;
    };
#else
    uint64 Dynamic_object : 1; // ( (uint64) 1 << 42 )
#endif    
    uint64 C_prefix_rtl_args : 1; // ( (uint64) 1 << 43 )
    uint64 Alias : 1; // ( (uint64) 1 << 44 )
    uint64 Text_macro : 1; // ( (uint64) 1 << 45 )
    uint64 String_macro : 1; // ( (uint64) 1 << 46 )
    uint64 History_node : 1; // ( (uint64) 1 << 47 )
    uint64 Debug_word : 1; // ( (uint64) 1 << 48 )
    uint64 Rt_stack_op : 1; // ( (uint64) 1 << 49 ) // Run Time stack operation
    uint64 Word_create : 1; // ( (uint64) 1 << 50 ) // Run Time stack operation
    uint64 Interpret_dbg : 1; // ( (uint64) 1 << 51 ) 
    uint64 Infix_word : 1; // ( (uint64) 1 << 52 ) 
    uint64 C_return : 1; // ( (uint64) 1 << 53 ) 
    uint64 Dlsym_word : 1; // ( (uint64) 1 << 54 ) 
    uint64 C_type : 1; // ( (uint64) 1 << 55 ) 
    uint64 Class_clone : 1; // ( (uint64) 1 << 56 ) 
    uint64 C_class : 1; // ( (uint64) 1 << 57 ) 
    uint64 C_typedef : 1; // ( (uint64) 1 << 58 ) 
    uint64 Dobject : 1; // ( (uint64) 1 << 59 ) 
    uint64 Lisp_word : 1; // ( (uint64) 1 << 60 ) 
};

struct LType0
{
    // LType - lisp types
uint64 T_unassigned0 : 10;
    uint64 T_lambda : 1; // ( (uint64) 1 << 11 )
    uint64 T_cons : 1; // ( (uint64) 1 << 12 )
    uint64 List : 1; // ( (uint64) 1 << 13 )
    uint64 Lisp_c_rtl_args : 1; // ( (uint64) 1 << 14 )
    uint64 List_object : 1; // ( (uint64) 1 << 15 )
    uint64 List_quotation : 1; // ( (uint64) 1 << 16 )
    uint64 T_lisp_if : 1; // ( (uint64) 1 << 17 )
    uint64 T_lisp_set : 1; // ( (uint64) 1 << 18 )
    uint64 List_node : 1; // ( (uint64) 1 << 19 )
    uint64 T_lisp_special : 1; // ( (uint64) 1 << 20 )
    uint64 T_lisp_begin : 1; // ( (uint64) 1 << 21 )
    uint64 T_lisp_token : 1; // ( (uint64) 1 << 22 )
    uint64 T_lisp_define : 1; // ( (uint64) 1 << 23 )
    uint64 T_lisp_macro : 1; // ( (uint64) 1 << 24 )
uint64 T_unassigned1 : 1;
    uint64 Lisp_void_return : 1; // ( (uint64) 1 << 26 )
    uint64 T_nil : 1; // ( (uint64) 1 << 27 )
    uint64 List_function : 1; // ((uint64) 1 << 28)
    uint64 T_lisp_compiled_word : 1; // ( (uint64) 1 << 29 )
    uint64 T_lisp_compile : 1; // ( (uint64) 1 << 30 )
    uint64 T_lisp_let : 1; // ( (uint64) 1 << 31 )
    uint64 T_lisp_terminating_macro : 1; // ( (uint64) 1 << 32 )
    uint64 T_lisp_read_macro : 1; // ( (uint64) 1 << 33 )
    //#define T_lisp_quasiquote ( (uint64) 1 << 34 )
    uint64 T_lisp_locals_arg : 1; // ( (uint64) 1 << 35 )
    //#define T_lisp_unquote ( (uint64) 1 << 35 )
    uint64 T_lisp_unquote_splicing : 1; // ( (uint64) 1 << 36 )
    uint64 T_lisp__define : 1; // ( (uint64) 1 << 37 )
    //#define T_lisp_defmacro ( (uint64) 1 << 37 )
    uint64 T_lisp_value : 1; // ( (uint64) 1 << 38 )
    uint64 T_lisp_word : 1; // ( (uint64) 1 << 39 )
uint64 T_unassigned2 : 4;
    uint64 T_lisp_symbol : 1; // ( (uint64) 1 << 44 )
    uint64 Lisp_word : 1 ; // ((uint64) 1 << 45)
};

typedef struct 
{
    uint64 AddMemChunk : 1 ;
    uint64 AddToList : 1 ;
    uint64 InitMemChunk : 1 ;
    uint64 ReturnMemChunk : 1 ;
    uint64 Account : 1 ;
} AllocFlags ;
#define ADD_MEM_CHUNK_HEADER ( (uint64) 1 << 0 ) 
#define ADD_TO_LIST ( (uint64) 1 << 1 ) 
#define INIT_HEADER ( (uint64) 1 << 2 ) 
#define RETURN_CHUNK_HEADER ( (uint64) 1 << 3 ) 
#define MEM_CHUNK_ACCOUNT ( (uint64) 1 << 4 ) 
#define RETURN_CHUNK_DATA ( (uint64) 1 << 5 ) 
#define ADD_TO_PERMANENT_MEMORY_LIST ( (uint64) 1 << 6 ) 
#define ADD_TO_PML ADD_TO_PERMANENT_MEMORY_LIST 

