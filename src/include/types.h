// see readme.txt for a text description
// TODO : types, database, garbage collection : integration
typedef unsigned char byte;
typedef byte uint8;
typedef char int8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef char * CString;
typedef byte CharSet;
typedef int32 Boolean;
typedef void (* VoidFunction) (void);
typedef void (*vFunction_1_Arg) (int32);
typedef void (*vFunction_2_Arg) (int32, int32);
typedef int32(*cFunction_0_Arg) ();
typedef int32(*cFunction_1_Arg) (int32);
typedef int32(*cFunction_2_Arg) (int32, int32);
typedef VoidFunction block;
typedef byte AsciiCharSet [ 256 ];

typedef byte * function, * object, * type, * slot;

typedef struct
{

    struct
    {
        uint64 T_CProperty;
        uint64 T_CProperty2;

        union
        {
            uint64 T_LProperty;
            uint64 T_AProperty;
        };
    };

    union
    {
        uint32 T_NumberOfSlots;
        uint32 T_NumberOfBytes;
        uint32 T_Size;
        uint32 T_ChunkSize; // remember MemChunk is prepended at memory allocation time
    };
    uint32 T_WordProperty;
    uint32 T_WAllocationType;
} CfrTilPropInfo, PropInfo, PropertyInfo, PI;

typedef struct
{

    union
    {
        PropInfo O_Property;
        type O_type; // for future dynamic types and dynamic objects 
    };

    union
    {
        slot * O_slots; // number of slots should be in T_NumberOfSlots
        object * O_object; // size should be in T_Size
    };
} Object, Tuple;
#define Tp_NodeAfter O_slots [0] ;
#define Tp_NodeBefore O_slots [1] ;
#define Tp_SymbolName O_slots [2] ;

typedef object * (*primop) (object *);
typedef Object * (*Primop) (Object *);

typedef struct _dlnode
{

    struct
    {
        struct _dlnode * n_After;
        struct _dlnode * n_Before;
    };
} dlnode, node;

typedef struct _dllist
{

    struct
    {
        dlnode * n_After;
        dlnode * n_Before;
    };
    node * n_CurrentNode;
} dllist;
#define after n_After
#define before n_Before
#define head after
#define tail before

enum types
{
    BOOL, BYTE, INTEGER, STRING, POINTER, XCODE, WORD, WORD_LOCATION
};

typedef struct _dobject
{

    struct
    {
        dlnode * do_After;
        dlnode * do_Before;
    };
    int16 do_Type;
    int16 do_Slots;
    int16 do_Size;
    int16 do_Etc;

    union
    {
        byte * do_bData;
        int32 * do_iData;
    };
} dobject;

typedef struct
{

    struct
    {
        dlnode * n_After;
        dlnode * n_Before;
    };

    union
    {
        node * n_CurrentNode;

        struct
        {
            int16 n_Type;
            int16 n_Slots;
        };
        byte * n_unmap;
    };

    union
    {
        PropInfo n_Property;
        type n_type; // for future dynamic types and dynamic objects 
    };
} _DLNode, _Node, _listNode, _List;

typedef struct
{

    union
    {

        struct
        {

            struct
            {
                dlnode * n_After;
                dlnode * n_Before;
            };

            union
            {
                node * n_CurrentNode;

                struct
                {
                    int16 n_Type;
                    int16 n_Slots;
                };
                byte * n_unmap;
            };

            union
            {
                PropInfo n_Property;
                type n_type; // for future dynamic types and dynamic objects 
            };

        };

        struct
        {

            struct
            {
                dlnode * do_After;
                dlnode * do_Before;
            };
            int16 do_Type;
            int16 do_Slots;
            int16 do_Size;
            int16 do_Etc;

            union
            {
                byte * do_bData;
                int32 * do_iData;
            };
        };
        _DLNode n_DLNode;
        dobject n_dobject;
    };
} DLNode, Node, listNode, List;

#define afterWord n_After 
#define beforeWord n_Before 
#define n_Car afterWord 
#define n_Cdr beforeWord
typedef void ( *MapFunction0) (dlnode *);
typedef void ( *MapFunction1) (dlnode *, int32);
typedef void ( *MapFunction2) (dlnode *, int32, int32);
typedef void ( *MapFunction2_64) (dlnode *, uint64, int32);
typedef int32(*MapFunction3) (dlnode *, int32, int32, int32);
typedef void ( *MapFunction4) (dlnode *, int32, int32, int32, int32);
typedef void ( *MapFunction5) (dlnode *, int32, int32, int32, int32, int32);
typedef Boolean(*MapFunction_1) (dlnode *);

typedef struct _Identifier
{
    DLNode S_Node;
    int32 Slots; // number of slots in Object
    byte * S_Name;
    uint64 State;

    union
    {
        uint32 S_Value;
        byte * S_PtrValue;
        dllist * S_SymbolList;
    };
    uint32 S_DObjectValue; // nb! DynamicObject value can not be a union with S_SymbolList
    uint32 * S_PtrToValue; // because we copy words with Compiler_PushCheckAndCopyDuplicates and we want the original value

    union // leave this here so we can add a ListObject to a namespace
    {
        struct _Identifier * S_ContainingNamespace;
        struct _Identifier * S_ClassFieldTypeNamespace;
        struct _Identifier * S_ContainingList;
        struct _Identifier * S_Prototype;
    };

    union
    {
        uint32 S_Value2;
        dlnode * S_Node2;
        byte * S_pb_Data;
    };

    block Definition;
    dllist * DebugWordList;
    struct _Identifier * CfrTilWord; // doesn't seem necessary
    struct _WordData * S_WordData;
} Identifier, ID, Word, Namespace, Vocabulary, Class, DynamicObject, DObject, ListObject, Symbol, MemChunk, HistoryStringNode;
#define S_Car S_Node.n_Car
#define S_Cdr S_Node.n_Cdr
#define S_After S_Cdr
#define S_Before S_Car
#define S_CurrentNode n_CurrentNode
#define S_AProperty S_Node.n_Property.T_AProperty
#define S_CProperty S_Node.n_Property.T_CProperty
#define S_CProperty2 S_Node.n_Property.T_CProperty2
#define S_CProperty0 S_Node.n_Property.T_CProperty0
#define S_WProperty S_Node.n_Property.T_WordProperty
#define S_WAllocType S_Node.n_Property.T_WAllocationType
#define S_LProperty S_Node.n_Property.T_LProperty
#define S_Size S_Node.n_Property.T_Size
#define S_ChunkSize S_Node.n_Property.T_ChunkSize
//#define S_Name S_Name 
#define S_NumberOfSlots S_Size
#define S_Pointer W_Value
#define S_String W_Value
#define S_unmap S_Node.n_unmap
//#define S_ChunkData S_Node.N_ChunkData
#define S_CodeSize Size // used by Debugger, Finder
#define S_MacroLength Size // used by Debugger, Finder

#define Head S_Car
#define Tail S_Cdr
#define Size S_Size 
#define Name S_Name
#define CProperty S_CProperty
#define CProperty2 S_CProperty2
#define LProperty S_LProperty
#define WProperty S_WProperty
#define WAllocType S_WAllocType
#define CProp S_CProperty
#define CProp2 S_CProperty2
#define LProp S_LProperty
#define WProp S_WProperty
#define Data S_pb_Data

#define Lo_CProperty CProperty
#define Lo_LProperty LProperty
#define Lo_CProp CProperty
#define Lo_LProp LProperty
#define Lo_Name Name
#define Lo_Car S_Car
#define Lo_Cdr S_Cdr
#define Lo_Size Size
#define Lo_Head Lo_Car
#define Lo_Tail Lo_Cdr
#define Lo_NumberOfSlots Size
#define Lo_CfrTilWord CfrTilWord //S_WordData->CfrTilWord
#define Lo_List S_SymbolList 
#define Lo_Value S_Value
#define Lo_PtrToValue S_PtrToValue 
#define Lo_Object Lo_Value
#define Lo_UInteger Lo_Value
#define Lo_Integer Lo_Value
#define Lo_String Lo_Value
#define Lo_LambdaFunctionParameters S_WordData->LambdaArgs
#define Lo_LambdaFunctionBody S_WordData->LambdaBody

#define W_List S_SymbolList 
#define W_Value S_Value
#define W_Value2 S_Value2
#define W_PtrValue S_PtrValue
#define W_PtrToValue S_PtrToValue
#define W_DObjectValue S_DObjectValue

typedef int32(*cMapFunction_1) (Symbol *);
typedef ListObject* (*ListFunction0)();
typedef ListObject* (*ListFunction)(ListObject*);
typedef ListObject * (*LispFunction2) (ListObject*, ListObject*);
typedef ListObject * (*LispFunction3) (ListObject*, ListObject*, ListObject*);
typedef int32(*MapFunction_Word_PtrInt) (ListObject *, Word *, int32 *);
typedef int32(*MapFunction) (Symbol *);
typedef int32(*MapFunction_Cell_1) (Symbol *, int32);
typedef int32(*MapFunction_Cell_2) (Symbol *, int32, int32);
typedef void ( *MapSymbolFunction) (Symbol *);
typedef void ( *MapSymbolFunction2) (Symbol *, int32, int32);
typedef Word* (*MapNodeFunction) (dlnode * node);

typedef struct _WordData
{
    uint64 RunType;
    Namespace * TypeNamespace;
    byte * CodeStart; // set at Word allocation 
    byte * Coding; // nb : !! this field is set by the Interpreter and modified by the Compiler in some cases so we also need (!) CodeStart both are needed !!  
    byte * Filename; // ?? should be made a part of a accumulated string table ??
    int32 LineNumber;
    int32 CursorPosition;
    int32 StartCharRlIndex;
    int32 SC_ScratchPadIndex;
    int32 NumberOfArgs;
    int32 NumberOfLocals;
    uint32 * InitialRuntimeDsp ;

    byte * ObjectCode; // used by objects/class words
    byte * StackPushRegisterCode; // used by the optInfo
    Word * AliasOf, *OriginalWord;

    union
    {
        int32 * ArrayDimensions;
        byte *SourceCode;
    };

    union
    {
        int32 Offset; // used by ClassField
        int32 RegToUse; // reg code : ECX, EBX, EDX, EAX, (1, 3, 2, 0) : in this order, cf. machineCode.h
    };

    union
    {
        ListObject * LambdaBody;
        int32 AccumulatedOffset; // used by Do_Object 
    };

    union
    {
        ListObject * LambdaArgs;
        int32 Index; // used by Variable and LocalWord
    };
} WordData;

// to keep using existing code without rewriting ...
#define CodeStart S_WordData->CodeStart // set at Word allocation 
#define Coding S_WordData->Coding // nb : !! this field is set by the Interpreter and modified by the Compiler in some cases so we also need (!) CodeStart both are needed !!  
#define Offset S_WordData->Offset // used by ClassField
#define W_NumberOfArgs S_WordData->NumberOfArgs 
#define W_NumberOfLocals S_WordData->NumberOfLocals 
#define W_InitialRuntimeDsp S_WordData->InitialRuntimeDsp 
#define TtnReference S_WordData->TtnReference // used by Logic Words
#define RunType S_WordData->RunType // number of slots in Object
#define PtrObject S_WordData->WD_PtrObject 
#define AccumulatedOffset S_WordData->AccumulatedOffset // used by Do_Object
#define Index S_WordData->Index // used by Variable and LocalWord
#define NestedObjects S_WordData->NestedObjects // used by Variable and LocalWord
#define ObjectCode S_WordData->Coding // used by objects/class words
#define StackPushRegisterCode S_WordData->StackPushRegisterCode // used by Optimize
#define SourceCode S_WordData->SourceCode 
#define W_CursorPosition S_WordData->CursorPosition 
#define W_StartCharRlIndex S_WordData->StartCharRlIndex
#define S_FunctionTypesArray S_WordData->FunctionTypesArray
#define RegToUse S_WordData->RegToUse
#define ArrayDimensions S_WordData->ArrayDimensions
#define W_AliasOf S_WordData->AliasOf
#define TypeNamespace S_WordData->TypeNamespace 
#define Lo_ListProc S_WordData->ListProc
#define Lo_ListFirst S_WordData->ListFirst
#define ContainingNamespace S_ContainingNamespace
#define ClassFieldTypeNamespace S_ClassFieldTypeNamespace
#define ContainingList S_ContainingList
#define Prototype S_Prototype
#define W_SearchNumber W_Value2
#define W_OriginalWord S_WordData->OriginalWord
#define W_SC_ScratchPadIndex S_WordData->SC_ScratchPadIndex // set at Word allocation 

typedef struct
{
    Symbol P_Symbol;
    slot P_Attributes;
} Property;

struct NamedByteArray;

typedef struct
{
    MemChunk BA_MemChunk;
    Symbol BA_Symbol;
    struct NamedByteArray * OurNBA;
    int32 BA_DataSize, MemRemaining;
    byte * StartIndex;
    byte * EndIndex;
    byte * bp_Last;
    byte * BA_Data;
} ByteArray;
#define BA_AllocSize BA_MemChunk.S_Size
#define BA_CProperty BA_MemChunk.S_CProperty
#define BA_AProperty BA_MemChunk.S_AProperty

typedef struct NamedByteArray
{
    MemChunk NBA_MemChunk;
    Symbol NBA_Symbol;
    ByteArray *ba_CurrentByteArray;
    int32 NBA_DataSize, TotalAllocSize;
    int32 MemInitial;
    int32 MemAllocated;
    int32 MemRemaining;
    int32 NumberOfByteArrays, CheckTimes;
    dllist NBA_BaList;
    dlnode NBA_ML_HeadNode;
    dlnode NBA_ML_TailNode;
} NamedByteArray, NBA;
#define NBA_AProperty NBA_Symbol.S_AProperty
#define NBA_Chunk_Size NBA_Symbol.S_ChunkSize
#define NBA_Name NBA_Symbol.S_Name

typedef struct
{
    Symbol B_Symbol;
    int32 InUseFlag;
} Buffer;
#define B_CProperty B_Symbol.S_CProperty
#define B_Size B_Symbol.S_Size
#define B_ChunkSize B_Symbol.S_ChunkSize
#define B_Data B_Symbol.S_pb_Data

typedef struct
{
    Symbol CN_Symbol;
    block CaseBlock;
} CaseNode;
#define CN_CaseValue CN_Symbol.S_pb_Data

typedef struct
{
    Symbol GI_Symbol;
    byte * pb_LabelName;
    byte * pb_JmpOffsetPointer;
} GotoInfo;
#define GI_CProperty GI_Symbol.S_CProperty

typedef struct
{
    Symbol BI_Symbol;
    uint64 State;
    byte *FrameStart;
    byte * AfterEspSave;
    byte *Start;
    byte *bp_First;
    byte *bp_Last;
    byte *JumpOffset;
    byte *LogicCode;
    byte *CombinatorStartsAt;
    byte *ActualCodeStart;
    int32 Ttt;
    int32 NegFlag, OverWriteSize;
    Word * LogicCodeWord, *LiteralWord;
    Namespace * LocalsNamespace;
} BlockInfo;

typedef struct
{
    uint64 State;
    int32 OutputLineCharacterNumber;
} PrintStateInfo;

typedef struct
{
    int32 State;

    union
    {

        struct
        {
            uint32 * Eax;
            uint32 * Ecx;
            uint32 * Edx;
            uint32 * Ebx;
            uint32 * Esp;
            uint32 * Ebp;
            uint32 * Esi;
            uint32 * Edi;
            uint32 * EFlags;
            uint32 * Eip;
        };
        uint32 * Registers [ 10 ];
    };
    //uint32 * RegisterStack [12];
    //uint32 * SaveEsp;
} Cpu;

typedef struct
{
    int32 StackSize;
    uint32 *StackPointer;
    uint32 *StackMin;
    uint32 *StackMax;
    uint32 *InitialTosPointer;
    uint32 StackData [];
} Stack;

typedef struct TCI
{
    uint64 State;
    int32 TokenFirstChar, TokenLastChar, EndDottedPos, DotSeparator, TokenLength, WordCount;
    byte *SearchToken, * PreviousIdentifier, *Identifier;
    Word * TrialWord, * OriginalWord, *RunWord, *OriginalRunWord, *NextWord, *ObjectExtWord;
    Namespace * OriginalContainingNamespace, * MarkNamespace;
} TabCompletionInfo, TCI;

struct ReadLiner;
typedef byte(*ReadLiner_KeyFunction) (struct ReadLiner *);

typedef struct ReadLiner
{
    uint64 State;
    ReadLiner_KeyFunction Key; //byte(*Key)( struct ReadLiner * );
    FILE *InputFile;
    FILE *OutputFile;
    byte *Filename;

    byte InputKeyedCharacter;
    byte LastCheckedInputKeyedCharacter;
    int32 FileCharacterNumber;
    int32 LineNumber;
    int32 InputLineCharacterNumber; // set by _CfrTil_Key
    int32 OutputLineCharacterNumber; // set by _CfrTil_Key
    int32 ReadIndex;
    int32 EndPosition; // index where the next input character is put
    int32 MaxEndPosition; // index where the next input character is put
    int32 CursorPosition; //
    int32 EscapeModeFlag;
    byte * DebugPrompt;
    byte * DebugAltPrompt;
    byte * NormalPrompt;
    byte * AltPrompt;
    byte * Prompt;
    HistoryStringNode * HistoryNode;
    TabCompletionInfo * TabCompletionInfo0;
    byte InputLine [ BUFFER_SIZE ];
    byte * InputLineString;
    byte * InputStringOriginal;
    byte * InputStringCurrent;
    int32 InputStringIndex, InputStringLength;
    int32 LineStartFileIndex;
    Stack * TciNamespaceStack;
} ReadLiner;
typedef void ( * ReadLineFunction) (ReadLiner *);

typedef struct
{
    uint64 State;
    int32 FinderFlags;
    Word *w_Word;
    Word *FoundWord;
    Word *LastWord;
    Namespace * FoundWordNamespace;
    Namespace * QualifyingNamespace;
} Finder;

struct Interpreter;

typedef struct Lexer
{
    uint64 State;
    byte *OriginalToken;

    union
    {
        int32 Literal;
        byte * LiteralString;
    };
    uint64 TokenType;
    Word * TokenWord;
    byte TokenInputCharacter;
    byte CurrentTokenDelimiter;
    int32 TokenStart_ReadLineIndex;
    int32 TokenEnd_ReadLineIndex;
    int32 Token_Length;
    byte * TokenDelimiters;
    byte * DelimiterCharSet;
    byte * BasicTokenDelimiters;
    byte * BasicDelimiterCharSet;
    byte * TokenDelimitersAndDot;
    byte * DelimiterOrDotCharSet;
    int32 CurrentReadIndex, TokenWriteIndex;
    struct Interpreter * OurInterpreter;
    ReadLiner * ReadLiner0;
    byte(*NextChar) (ReadLiner * rl);
    byte * TokenBuffer;
} Lexer;

typedef struct
{
    int32 OptimizeFlag;
    int32 Optimize_Dest_RegOrMem;
    int32 Optimize_Mod;
    int32 Optimize_Reg;
    int32 Optimize_Rm;
    int32 Optimize_Disp;
    int32 Optimize_Imm;
    int32 Optimize_SrcReg;
    int32 Optimize_DstReg;
    Word *O_zero, * O_one, *O_two, *O_three, *O_four, *O_five, *O_six;
} CompileOptimizeInfo;

typedef struct
{
    uint64 State;
    byte *IfZElseOffset;
    byte *ContinuePoint; // used by 'continue'
    byte * BreakPoint;
    byte * StartPoint;
    int32 NumberOfLocals;
    int32 NumberOfArgs;
    int32 NumberOfRegisterVariables;
    int32 LocalsFrameSize;
    int32 SaveCompileMode;
    //int32 LispParenLevel;
    int32 ParenLevel;
    int32 GlobalParenLevel;
    int32 BlockLevel;
    int32 ArrayEnds;
    byte * InitHere;
    int32 * AccumulatedOptimizeOffsetPointer;
    int32 AccumulatedOffsetPointerFlag, * AccumulatedOffsetPointer;
    int32 * FrameSizeCellOffset;
    int RegOrder [ 4 ]; //= { EBX, EDX, ECX, EAX } ;
    byte * EspSaveOffset;
    byte * EspRestoreOffset;
    Word * ReturnVariableWord;
    Word * CurrentWord;
    Word * LHS_Word; //, *OptimizeOffWord;
    Namespace *C_BackgroundNamespace; //, ** FunctionTypesArray ;
    dllist * GotoList;
    dllist * CurrentSwitchList;
    CompileOptimizeInfo * optInfo;
    Stack * CombinatorInfoStack;
    Stack * PointerToOffset;
    dllist * WordList, *PostfixLists;
    Stack * LocalNamespaces;
    Stack * CombinatorBlockInfoStack;
    Stack * BlockStack;
    Stack * NamespacesStack;
    Stack * InfixOperatorStack;
} Compiler;

typedef struct Interpreter
{
    uint64 State;
    ReadLiner * ReadLiner0;
    Finder * Finder0;
    Lexer * Lexer0;
    Compiler * Compiler0;
    byte * Token;
    Word *w_Word, *LastWord; //*IncDecWord, *IncDecOp, 
    Word * BaseObject; //, *QidObject, *ArrayObject;
    Word *CurrentObjectNamespace, *ThisNamespace;
    int32 WordType;
    dllist * PreprocessorStackList;
    dllist * InterpList;
} Interpreter;

struct _Debugger;
typedef void (* DebuggerFunction) (struct _Debugger *);

typedef struct _Debugger
{
    uint64 State;
    uint32 * SaveDsp, *SaveEdi;
    uint32 * WordDsp;
    int32 SaveTOS;
    int32 SaveStackDepth;
    int32 Key;
    int32 SaveKey; //Verbosity;
    int32 TokenStart_ReadLineIndex, Esi, Edi;
    Word * w_Word, *EntryWord, *LastShowWord, *LastEffectsWord, *LastSetupWord, *SteppedWord, *DebugWordListWord, *CurrentlyRunningWord;
    byte * Token;
    block SaveCpuState;
    block RestoreCpuState;
    block GetESP;
    byte * OptimizedCodeAffected;
    byte * PreHere, *StartHere, *LastDisHere, *ShowLine, * Filename;
    Stack *DebugStack;
    Cpu * cs_Cpu;
    byte* DebugAddress, *ReturnStackCopyPointer, *LastSourceCodeAddress;
    uint32 * DebugESP, *DebugEBP, *DebugESI, *DebugEDI; //, *SavedIncomingESP, *SavedIncomingEBP ; //, SavedRunningESP, SavedRunningEBP;
    int32 LastSourceCodeIndex;
    ByteArray * StepInstructionBA;
    byte CharacterTable [ 128 ];
    DebuggerFunction CharacterFunctionTable [ 32 ];
    ud_t * Udis;
    Namespace * Locals;
    dllist * DebugWordList;
    sigjmp_buf JmpBuf0; //, JmpBuf1 ; 
} Debugger;

typedef struct
{
    uint64 State;
    int32 NumberBase;
    int32 BigNumPrecision;
    int32 BigNumWidth;
    int32 ExceptionFlag;
    int32 IncludeFileStackNumber;
    struct timespec Timers [ 8 ];
} System;

typedef struct
{
    uint64 State;
    int32 NsCount, WordCount;
    ReadLiner *ReadLiner0;
    Lexer *Lexer0;
    Finder * Finder0;
    Interpreter * Interpreter0;
    Compiler *Compiler0;
    System * System0;
    Stack * ContextDataStack;
    byte * Location;
    Word * CurrentlyRunningWord, *NlsWord, *SC_CurrentCombinator;
    NBA * ContextNba;
    sigjmp_buf JmpBuf0;
} Context;
typedef void (* ContextFunction_2) (Context * cntx, byte* arg1, int32 arg2);
typedef void (* ContextFunction_1) (Context * cntx, byte* arg);
typedef void (* ContextFunction) (Context * cntx);
typedef void (* LexerFunction) (Lexer *);

typedef struct _CombinatorInfo
{

    union
    {
        int32 CI_i32_Info;

        struct
        {
            unsigned BlockLevel : 16;
            unsigned ParenLevel : 16;
        };
    };
} CombinatorInfo;
struct _CfrTil;

typedef struct _LambdaCalculus
{
    uint64 State;
    int32 DontCopyFlag, Loop, LispParenLevel;
    Namespace * LispTemporariesNamespace, *LispNamespace;
    ListObject * Nil, *True, *CurrentList, *CurrentLambdaFunction; //, *ListFirst;
    ByteArray * SavedCodeSpace;
    uint32 ItemQuoteState, QuoteState;
    struct _CfrTil * OurCfrTil;
    Stack * QuoteStateStack;
    int32 * SaveStackPointer;
    struct _LambdaCalculus * SaveLC;
} LambdaCalculus;

typedef struct
{

    union
    {

        struct
        {
            unsigned CharFunctionTableIndex : 16;
            unsigned CharType : 16;
        };
        int32 CharInfo;
    };
} CharacterType;

typedef struct _StringTokenInfo
{
    uint64 State;
    int32 StartIndex, EndIndex;
    byte * In, *Out, *Delimiters, *SMNamespace;
    CharSet * CharSet0;
} StringTokenInfo, StrTokInfo;
// StrTokInfo State constants
#define STI_INITIALIZED     ( 1 << 0 )

typedef struct _CfrTil
{
    uint64 State;
    Stack * DataStack;
    Namespace * Namespaces;
    Context * Context0;
    Stack * ContextStack;
    Debugger * Debugger0;
    Stack * ObjectStack, *DebugStateStack;
    Namespace * InNamespace, *LispNamespace;
    LambdaCalculus * LC;
    FILE * LogFILE;
    int32 LogFlag, WordsAdded;
    uint32 * SaveDsp;
    Cpu * cs_Cpu;
    block SaveCpuState, RestoreCpuState;
    Word * LastFinishedWord, *StoreWord, *PokeWord, *ScoOcCrw; //, *DebugWordListWord ;
    byte ReadLine_CharacterTable [ 256 ];
    ReadLineFunction ReadLine_FunctionTable [ 24 ];
    CharacterType LexerCharacterTypeTable [ 256 ];
    LexerFunction LexerCharacterFunctionTable [ 24 ];
    Buffer *StringB, * TokenB, *OriginalInputLineB, *InputLineB, *SourceCodeSPB, *StringInsertB, *StringInsertB2, *StringInsertB3, *StringInsertB4, *StringInsertB5, *StrCatBuffer;
    Buffer *TabCompletionBuf, * LC_PrintB, * LC_DefineB, *DebugB, *DebugB1, *DebugB2, *ScratchB1, *ScratchB2, *StringMacroB; // token buffer, tab completion backup, source code scratch pad, 
    StrTokInfo Sti;
    byte * OriginalInputLine;
    byte * TokenBuffer;
    byte * SourceCodeScratchPad; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    int32 SC_ScratchPadIndex, SC_QuoteMode, DWL_SC_ScratchPadIndex; //, SCA_BlockedIndex ;
    byte * LispPrintBuffer; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    dllist *DebugWordList, *TokenList;
    sigjmp_buf JmpBuf0;
    //dllist *TokenList;
} CfrTil;

typedef struct
{
    MemChunk MS_MemChunk;
    // static buffers
    // short term memory
    NamedByteArray * SessionObjectsSpace; // until reset
    NamedByteArray * TempObjectSpace; // lasts for one line
    NamedByteArray * CompilerTempObjectSpace; // lasts for compile of one word
    NamedByteArray * ContextSpace;
    NamedByteArray * LispTempSpace;
    // quasi long term
    NamedByteArray * BufferSpace;
    NamedByteArray * CfrTilInternalSpace;
    // long term memory
    NamedByteArray * CodeSpace;
    NamedByteArray * ObjectSpace;
    NamedByteArray * DictionarySpace;
    NamedByteArray * HistorySpace;
    NamedByteArray * OpenVmTilSpace;
    NamedByteArray * StringSpace;
    dllist NBAs;
    dlnode NBAsHeadNode;
    dlnode NBAsTailNode;
    dllist * BufferList;
} MemorySpace;

typedef struct
{
    int Red, Green, Blue;
} RgbColor;

typedef struct
{
    RgbColor rgbc_Fg;
    RgbColor rgbc_Bg;
} RgbColors;

typedef struct
{
    int Fg;
    int Bg;
} IntColors;

typedef struct
{

    union
    {
        RgbColors rgbcs_RgbColors;
        IntColors ics_IntColors;
    };
} Colors;

typedef struct
{
    NamedByteArray * HistorySpaceNBA;
    dlnode _StringList_HeadNode, _StringList_TailNode;
    dllist _StringList, * StringList;
} HistorySpace;

typedef struct
{
    uint64 State;
    CfrTil * OVT_CfrTil;
    struct termios * SavedTerminalAttributes;
    Context * OVT_Context;
    Interpreter * OVT_Interpreter;
    HistorySpace OVT_HistorySpace;
    LambdaCalculus * OVT_LC;
    ByteArray * CodeByteArray; // a variable

    PrintStateInfo *psi_PrintStateInfo;
    int32 SignalExceptionsHandled;

    byte *InitString;
    byte *StartupString;
    byte *StartupFilename;
    byte *ErrorFilename;
    byte *VersionString;
    byte *ExceptionMessage;
    int32 RestartCondition;
    int32 Signal;

    int32 Argc;
    char ** Argv;
    void * SigAddress;
    byte * SigLocation;
    Colors *Current, Default, Alert, Debug, Notice, User;
    int32 Console;

    dllist PermanentMemList;
    dlnode PML_HeadNode;
    dlnode PML_TailNode;
    MemorySpace * MemorySpace0;
    int32 PermanentMemListRemainingAccounted, TotalNbaAccountedMemRemaining, TotalNbaAccountedMemAllocated, TotalMemSizeTarget;
    int32 Mmap_RemainingMemoryAllocated, OVT_InitialUnAccountedMemory, TotalMemFreed, TotalMemAllocated, NumberOfByteArrays;

    // variables accessible from cfrTil
    int32 Verbosity;
    int32 StartIncludeTries;
    int32 StartedTimes, InitSessionCoreTimes, SigSegvs, AllocationRequestLacks;

    int32 DictionarySize;
    int32 LispTempSize;
    int32 MachineCodeSize;
    int32 ObjectsSize;
    int32 ContextSize;
    int32 TempObjectsSize;
    int32 CompilerTempObjectsSize;
    int32 SessionObjectsSize;
    int32 DataStackSize;
    int32 HistorySize;
    int32 OpenVmTilSize;
    int32 CfrTilSize;
    int32 BufferSpaceSize;
    int32 StringSpaceSize;

    int Thrown;
    sigjmp_buf JmpBuf0;
    
    byte ** _Name_ ;
} OpenVmTil;

// note : this puts these namespaces on the search list such that last, in the above list, will be searched first

typedef struct
{
    const char * ccp_Name;
    block blk_Definition;
    uint64 ui64_CProperty;
    uint64 ui64_LProperty;
    const char *NameSpace;
    const char * SuperNamespace;
} CPrimitive;

// ( byte * name, int32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
// ( const char * name, block b, uint64 ctype, uint64 ltype, const char *nameSpace, const char * superNamespace )

typedef struct
{
    const char * ccp_Name;
    uint64 ui64_CProperty;
    block blk_CallHook;
    byte * Function;
    int32 i32_FunctionArg;
    const char *NameSpace;
    const char * SuperNamespace;
} MachineCodePrimitive;

typedef struct ppibs
{

    union
    {
        int32 int32_Ppibs; // for ease of initializing and conversion

        struct
        {
            unsigned IfBlockStatus : 1; // status of whether we should do an ifBlock or not
            unsigned ElifStatus : 1; // remembers when we have done an elif in a block; only one can be done in a block in the C syntax definition whick we emulate
            unsigned DoIfStatus : 1; // controls whether we do nested if block
        };
    };
}
PreProcessorIfBlockStatus, Ppibs;



