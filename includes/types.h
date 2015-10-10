// see readme.txt for a text description
// TODO : types, database, garbage collection : integration
typedef unsigned char byte;
typedef byte uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint;
typedef unsigned long uint32;
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

typedef byte * function, object, type, slot;

typedef struct
{

    struct
    {
        uint64 T_CType;

        union
        {
            uint64 T_LType;
            uint64 T_AType;
        };
    };

    union
    {
        uint32 T_NumberOfSlots; // : 26;
        uint32 T_NumberOfBytes; // : 26;
        uint32 T_Size; //: 26;
        uint32 T_ChunkSize; //: 26; // remember MemChunk is prepended at memory allocation time
    };
    uint32 T_WordType; //: 6 ;
} CfrTilType, Type;

typedef struct
{

    union
    {
        Type O_Type;
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

typedef struct DLNode
{

    union
    {
        Type N_Type;
        type N_type; // for future dynamic types and dynamic objects 
    };
    byte * N_unmap;
    byte * N_ChunkData;

    struct
    {
        struct DLNode * N_After; // slots[0]
        struct DLNode * N_Before; // slots[1]
    };
} DLNode, Node, listNode, List;
#define After N_After 
#define Before N_Before 
#define N_Car After 
#define N_Cdr Before
typedef void ( *MapFunction0) (DLNode *);
typedef void ( *MapFunction1) (DLNode *, int32);
typedef void ( *MapFunction2) (DLNode *, int32, int32);
typedef void ( *MapFunction2_64) (DLNode *, uint64, int32);
typedef void ( *MapFunction3) (DLNode *, int32, int32, int32);
typedef void ( *MapFunction4) (DLNode *, int32, int32, int32, int32);
typedef void ( *MapFunction5) (DLNode *, int32, int32, int32, int32, int32);
typedef Boolean(*MapFunction_1) (DLNode *);

typedef struct _Identifier
{
    DLNode S_Node;
    int32 Slots; // number of slots in Object
    byte * S_pb_Name;
    int32 State;

    union
    {
        struct _Identifier * S_SymbolList;
        uint32 S_Value;
    };

    union // leave this here so we can add a ListObject to a namespace
    {
        struct _Identifier * S_ContainingNamespace;
        struct _Identifier * S_ClassFieldNamespace;
        struct _Identifier * S_ContainingList;
        struct _Identifier * S_Prototype;
    };

    union
    {
        DLNode * S_Node2 ;
        struct _Identifier * S_plo_listObject;
        int32 S_i32_Data;
        int32 S_i32_Value;
        uint32 S_ui32_Data;
        uint32 S_ui32_Value;
        byte * S_pb_Value;
        byte * S_pb_Data;
        Object * S_po_lo_Slots[1];
    };
    block Definition;
    struct _Identifier * S_CfrTilWord; // doesn't seem necessary for some reason
    struct _WordData * S_WordData;
} Identifier, ID, Word, Namespace, Class, DynamicObject, DObject, ListObject, DLList, listObject, Symbol, MemChunk, HistoryStringNode;
#define S_Car S_Node.N_Car
#define S_Cdr S_Node.N_Cdr
#define Head S_Car
#define Tail S_Cdr
#define S_CurrentNode S_Node2
#define S_AType S_Node.N_Type.T_AType
#define S_CType S_Node.N_Type.T_CType
#define S_CType0 S_Node.N_Type.T_CType0
#define S_WType S_Node.N_Type.T_WordType
#define S_LType S_Node.N_Type.T_LType

#define S_Size S_Node.N_Type.T_Size
#define S_ChunkSize S_Node.N_Type.T_ChunkSize
#define S_Name S_pb_Name 
#define S_NumberOfSlots S_Size
#define S_Pointer W_Value
#define S_String W_Value
#define S_unmap S_Node.N_unmap
#define S_ChunkData S_Node.N_ChunkData
#define Size S_Size 
#define Name S_Name
#define CType S_CType
#define CType0 S_CType0
#define LType S_LType
#define WType S_WType
#define Data S_pb_Data
#define S_CodeSize Size // used by Debugger, Finder
#define S_MacroLength Size // used by Debugger, Finder

#define Lo_CType CType
#define Lo_LType LType
#define Lo_Name Name
#define Lo_Car S_Car
#define Lo_Cdr S_Cdr
#define Lo_Size Size
#define Lo_Head Lo_Car
#define Lo_Tail Lo_Cdr
#define Lo_NumberOfSlots Size
#define Lo_CfrTilWord S_CfrTilWord

#define Lo_List S_SymbolList 
#define Lo_Value S_Value 
#define W_List S_SymbolList 
#define W_Value S_Value

#define Lo_Object Lo_Value
#define Lo_UInteger Lo_Value
#define Lo_Integer Lo_Value
#define Lo_String Lo_Value

#define Lo_LambdaFunctionParameters S_WordData->LambdaArgs
#define Lo_LambdaFunctionBody S_WordData->LambdaBody

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

typedef struct _WordData
{
    byte ** WD_PtrObject; // necessary for compiling class words and variables 
    uint64 RunType;
    Namespace * TypeNamespace;
    byte * CodeStart; // set at Word allocation 
    byte * Coding; // nb : !! this field is set by the Interpreter and modified by the Compiler in some cases so we also need (!) CodeStart both are needed !!  
    byte * ObjectCode; // used by objects/class words
    byte * StackPushRegisterCode; // used by the optimizer
    int32 * ArrayDimensions;
    Word * AliasOf;
    byte *SourceCode;
    byte * Filename; // ?? should be made a part of a accumulated string table ??
    int32 LineNumber;
    int32 CursorPosition;

    union
    {
        int32 Offset; // used by ClassField
        int32 NumberOfArgs;
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
#define NumberOfArgs S_WordData->NumberOfArgs 
#define TtnReference S_WordData->TtnReference // used by Logic Words
#define RunType S_WordData->RunType // number of slots in Object
#define PtrObject S_WordData->WD_PtrObject // necessary for compiling class words and variables -- might as well be used" otherwise
#define AccumulatedOffset S_WordData->AccumulatedOffset // used by Do_Object
#define Index S_WordData->Index // used by Variable and LocalWord
#define NestedObjects S_WordData->NestedObjects // used by Variable and LocalWord
#define ObjectCode S_WordData->ObjectCode // used by objects/class words
#define StackPushRegisterCode S_WordData->StackPushRegisterCode // used by Optimize
#define SourceCode S_WordData->SourceCode 
#define Filename S_WordData->Filename // ?? should be made a part of a accumulated string table ??
#define LineNumber S_WordData->LineNumber 
#define CursorPosition S_WordData->CursorPosition 
#define S_FunctionTypesArray S_WordData->FunctionTypesArray
#define RegToUse S_WordData->RegToUse
#define ArrayDimensions S_WordData->ArrayDimensions
#define AliasOf S_WordData->AliasOf
#define TypeNamespace S_WordData->TypeNamespace 
#define Lo_ListProc S_WordData->ListProc
#define Lo_ListFirst S_WordData->ListFirst
#define ContainingNamespace S_ContainingNamespace
#define ClassFieldNamespace S_ClassFieldNamespace
#define ContainingList S_ContainingList
#define Prototype S_Prototype

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
    int32 BA_DataSize;
    byte * StartIndex;
    byte * EndIndex;
    byte * bp_Last;
    byte * BA_Data;
} ByteArray;
#define BA_AllocSize BA_MemChunk.S_Size
#define BA_CType BA_MemChunk.S_CType
#define BA_AType BA_MemChunk.S_AType

typedef struct NamedByteArray
{
    MemChunk NBA_MemChunk;
    Symbol NBA_Symbol;
    ByteArray *ba_CurrentByteArray;
    int32 NBA_Size, TotalAllocSize;
    int32 MemInitial;
    int32 MemAllocated;
    int32 MemRemaining;
    int32 NumberOfByteArrays;
    DLList NBA_BaList;
    DLNode NBA_ML_HeadNode;
    DLNode NBA_ML_TailNode;
} NamedByteArray, NBA;
#define NBA_AType NBA_Symbol.S_AType
#define NBA_Chunk_Size NBA_Symbol.S_ChunkSize
#define NBA_Name NBA_Symbol.S_Name

typedef struct
{
    Symbol B_Symbol;
    int32 InUseFlag;
} Buffer;
#define B_CType B_Symbol.S_CType
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
#define GI_CType GI_Symbol.S_CType

typedef struct
{
    Symbol BI_Symbol;
    int32 State;
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
    int32 NegFlag;
    Word * LogicCodeWord, *LiteralWord;
    Namespace * LocalsNamespace;
} BlockInfo;

typedef struct
{
    int32 State;
    int32 OutputLineCharacterNumber;
} PrintStateInfo;

typedef struct
{

    union
    {

        struct
        {
            int32 Eax;
            int32 Ecx;
            int32 Edx;
            int32 Ebx;
            int32 Esp;
            int32 Ebp;
            int32 Esi;
            int32 Edi;
            int32 EFlags;
            int32 Eip;
        };
        int32 Registers [ 10 ];
    };
} CpuState;

typedef struct
{
    int32 StackSize;
    int32 *StackPointer;
    int32 *StackMin;
    int32 *StackMax;
    int32 *InitialTosPointer;
    int32 StackData [];
} Stack;

typedef struct TCI
{
    int32 State;
    int32 TokenFirstChar, TokenLastChar, EndDottedPos, DotSeparator, TokenLength;
    byte *SearchToken, * PreviousIdentifier, *Identifier, *LastUpMove, *LastMove;
    Word * TrialWord, * OriginalWord, *RunWord, *OriginalRunWord, *LastNamespace, *MarkWord, *NextWord, *ObjectExtWord;
    Namespace * OriginalContainingNamespace, * MarkNamespace;
} TabCompletionInfo, TCI;

struct ReadLiner;
typedef byte(*ReadLiner_KeyFunction) (struct ReadLiner *);

typedef struct ReadLiner
{
    int32 State;
    ReadLiner_KeyFunction Key; //byte(*Key)( struct ReadLiner * );
    FILE *InputFile;
    FILE *OutputFile;
    byte *bp_Filename;

    byte InputKeyedCharacter;
    byte LastCheckedInputKeyedCharacter;
    int32 FileCharacterNumber;
    int32 i32_LineNumber;
    int32 InputLineCharacterNumber; // set by _CfrTil_Key
    int32 OutputLineCharacterNumber; // set by _CfrTil_Key
    int32 ReadIndex;
    int32 EndPosition; // index where the next input character is put
    int32 MaxEndPosition; // index where the next input character is put
    int32 i32_CursorPosition; //
    int32 EscapeModeFlag;
    byte * DebugPrompt;
    byte * NormalPrompt;
    byte * AltPrompt;
    byte * Prompt;
    HistoryStringNode * HistoryNode;
    TabCompletionInfo * TabCompletionInfo0;
    byte * InputLine;
    byte * InputStringOriginal;
    byte * InputStringCurrent;
    int32 InputStringIndex;
    int32 LineStartFileIndex;
    Stack * TciNamespaceStack;
} ReadLiner;
typedef void ( * ReadLineFunction) (ReadLiner *);

typedef struct
{
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
    int32 State;
    byte *OriginalToken;
    int32 Literal;
    uint64 TokenType;
    Word * TokenWord;
    byte TokenInputCharacter;
    byte CurrentTokenDelimiter;
    int32 TokenStart_ReadLineIndex;
    int32 TokenEnd_ReadLineIndex;
    byte * TokenDelimiters;
    byte * DelimiterCharSet;
    byte * BasicTokenDelimiters;
    byte * BasicDelimiterCharSet;
    byte * TokenDelimitersAndDot;
    byte * DelimiterOrDotCharSet;
    int32 CurrentReadIndex, TokenWriteIndex;
    struct Interpreter * OurInterpreter;
    ReadLiner * ReadLiner;
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
    Word *O_zero, * O_one, *O_two, *O_three, *O_four, *O_five;
} CompileOptimizer;

typedef struct
{
    int32 State;
    byte *IfZElseOffset;
    byte *ContinuePoint; // used by 'continue'
    byte * BreakPoint;
    byte * StartPoint;
    int32 NumberOfLocals;
    int32 NumberOfStackVariables;
    int32 NumberOfRegisterVariables;
    int32 LocalsFrameSize;
    int32 SaveCompileMode;
    int32 LispParenLevel;
    int32 ParenLevel;
    int32 GlobalParenLevel;
    int32 BlockLevel;
    int32 ArrayEnds;
    //int32 ArrayIndex, PreviousArrayIndex ;
    //int32 ArrayIndexWithNonInteger;
    byte * InitHere;
    int32 * AccumulatedOptimizeOffsetPointer;
    int32 * AccumulatedOffsetPointer;
    int32 * AddOffsetCodePointer;
    int32 * FrameSizeCellOffset;
    byte * EspSaveOffset;
    byte * EspRestoreOffset;
    Word * ReturnVariableWord;
    Word * RecursiveWord;
    Word * CurrentWord;
    Word * CurrentCreatedWord;
    Word * LHS_Word;
    Namespace ** FunctionTypesArray, *C_BackgroundNamespace;
    DLList * GotoList;
    DLList * CurrentSwitchList;
    CompileOptimizer * Optimizer;
    Stack * CombinatorInfoStack;
    Stack * ObjectStack;
    Stack * PointerToOffset;
    Stack * WordStack;
    Stack * LocalNamespaces;
    Stack * CombinatorBlockInfoStack;
    Stack * BlockStack;
    Stack * NamespacesStack;
    Stack * InfixOperatorStack;
} Compiler;

typedef struct Interpreter
{
    int32 State;
    ReadLiner * ReadLiner;
    Finder * Finder;
    Lexer * Lexer;
    Compiler * Compiler;
    Word *w_Word;
    Word * BaseObject, *QidObject;
    Word *ObjectField;
    Word *CurrentPrefixWord;
    Symbol * s_List;
    int32 InterpretStringBufferIndex;
    byte * Token; //InterpretStringBuffer;
    int32 * PrefixWord_SaveSP, ParenLevel;
} Interpreter;

struct _Debugger;
typedef void (* DebuggerFunction) (struct _Debugger *);

typedef struct _Debugger
{
    int32 State;
    int32 * SaveDsp;
    int32 * WordDsp;
    int32 SaveTOS;
    int32 SaveStackDepth;
    int32 Key;
    int32 SaveKey;
    Word * w_Word, *LastShowWord, *LastPreWord, *SteppedWord;
    byte * Token;
    block SaveCpuState;
    block RestoreCpuState;
    block GetEIP, GetESP, RestoreEbpEsp;
    int32 TokenEnd_FileCharNumber;
    byte * OptimizedCodeAffected;
    byte * PreHere;
    Stack *DebugStack;
    CpuState * cs_CpuState;
    CpuState * cs_CpuState_Entry;
    byte* DebugAddress;
    int32 * DebugESP;
    Stack *AddressAfterJmpCallStack;
    ByteArray * StepInstructionBA;
    byte CharacterTable [ 128 ];
    DebuggerFunction CharacterFunctionTable [ 30 ];
    ud_t * Udis;
    Namespace * Locals;
} Debugger;

typedef struct
{
    int32 State;
    int32 NumberBase;
    int32 BigNumPrecision;
    int32 BigNumWidth;
    int32 ExceptionFlag;
    int32 IncludeFileStackNumber;
    struct timespec Timers [ 8 ];
} System;

typedef struct
{
    int32 State;
    int32 MemoryType;
    ReadLiner *ReadLiner0;
    Lexer *Lexer0;
    Finder * Finder0;
    Interpreter * Interpreter0;
    Compiler *Compiler0;
    System * System0;
    Stack * ContextDataStack;
    byte * Location;
    jmp_buf JmpBuf0;
} Context;
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

typedef struct LambdaCalculus
{
    int32 State, DontCopyFlag, Loop, *SaveStackPtr;
    Namespace * LispTemporariesNamespace, *LispNamespace;
    ListObject * Nil, *True, *CurrentList, *CurrentFunction ; //, *ListFirst;
    ByteArray * SavedCodeSpace;
    uint32 ItemQuoteState, QuoteState;
    struct _CfrTil * OurCfrTil;
    Stack * QuoteStateStack;
    byte * SaveStackPointer ;
    struct LambdaCalculus * SaveLC ;
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
    int32 State;
    int32 StartIndex, EndIndex;
    byte * In, *Out, *Delimiters, *SMNamespace;
    CharSet * CharSet;
} StringTokenInfo, StrTokInfo;
// StrTokInfo State constants
#define STI_INITIALIZED     ( 1 << 0 )

typedef struct _CfrTil
{
    int32 State;
    Stack * DataStack;
    Namespace * Namespaces;
    Context * Context0;
    Stack * ContextStack;
    Debugger * Debugger0;
    Stack * ObjectStack;
    Namespace * InNamespace, *LispNamespace;
    LambdaCalculus * LC;
    FILE * LogFILE;
    int LogFlag;
    int32 * SaveDsp;
    CpuState * cs_CpuState;
    block SaveCpuState;
    Word * LastFinishedWord, *StoreWord, *PokeWord;
    int32 DebuggerVerbosity;
    jmp_buf JmpBuf0;
    byte ReadLine_CharacterTable [ 256 ];
    ReadLineFunction ReadLine_FunctionTable [ 23 ];
    CharacterType LexerCharacterTypeTable [ 256 ];
    LexerFunction LexerCharacterFunctionTable [ 24 ];
    Buffer *StringB, * TokenB, *OriginalInputLineB, *InputLineB, *SourceCodeSPB, *StringInsertB, *StringInsertB2, *StringInsertB3;
    Buffer *TabCompletionBuf, * LambdaCalculusPB, *PrintfB, *DebugB, *DebugB2, *HistoryExceptionB, *StringMacroB; // token buffer, tab completion backup, source code scratch pad, 
    StrTokInfo Sti;
    byte * OriginalInputLine;
    byte * TokenBuffer;
    byte * SourceCodeScratchPad; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    int32 SC_ScratchPadIndex;
    byte * LispPrintBuffer; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    DLList * TokenList, *PeekTokenList;
    Word * CurrentRunWord;
} CfrTil;

typedef struct
{
    MemChunk MS_MemChunk;
    // static buffers
    // short term memory
    NamedByteArray * SessionObjectsSpace; // until reset
    NamedByteArray * TempObjectSpace; // last for one line
    NamedByteArray * CompilerTempObjectSpace; // last for compile of one word
    // long term memory
    NamedByteArray * CodeSpace;
    NamedByteArray * ObjectSpace;
    NamedByteArray * DictionarySpace;
    NamedByteArray * BufferSpace;
    NamedByteArray * ContextSpace;
    NamedByteArray * HistorySpace;
    NamedByteArray * CfrTilInternalSpace;
    NamedByteArray * OpenVmTilSpace;
    NamedByteArray * LispTempSpace;
    DLList NBAs;
    DLNode NBAsHeadNode;
    DLNode NBAsTailNode;
    DLList * BufferList;
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
    DLNode _StringList_HeadNode, _StringList_TailNode;
    DLList _StringList, * StringList;
} HistorySpace;

typedef struct
{
    int32 State;
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

    jmp_buf JmpBuf0;
    int Thrown;

    int32 Argc;
    char ** Argv;
    void * SigAddress;
    byte * SigLocation;
    Colors *Current, Default, Alert, Debug, Notice, User;
    int32 Console;

    DLList PermanentMemList;
    DLNode PML_HeadNode;
    DLNode PML_TailNode;
    MemorySpace * MemorySpace0;
    int32 PermanentMemListAccounted, MemRemaining, TotalAccountedMemAllocated;
    int32 Mmap_TotalMemoryAllocated, OVT_InitialUnAccountedMemory, NumberOfByteArrays;

    // variables accessible from cfrTil
    int32 Verbosity;
    int32 StartIncludeTries;
    int32 StartedTimes;
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
} OpenVmTil;

// note : this puts these namespaces on the search list such that last, in the above list, will be searched first

typedef struct
{
    const char * ccp_Name;
    block blk_Definition;
    uint64 ui64_CType;
    uint64 ui64_LType;
    const char *NameSpace;
    const char * SuperNamespace;
} CPrimitive;

// ( byte * name, int32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, int32 allocType )
// ( const char * name, block b, uint64 ctype, uint64 ltype, const char *nameSpace, const char * superNamespace )

typedef struct
{
    const char * ccp_Name;
    uint64 ui64_CType;
    block blk_CallHook;
    byte * Function;
    int32 i32_FunctionArg;
    const char *NameSpace;
    const char * SuperNamespace;
} MachineCodePrimitive;



