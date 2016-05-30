
#define Stack_Pop(stack) Stack_Pop_WithExceptionOnEmpty ( stack )

#define _Q_CodeByteArray _Q_->CodeByteArray
#define _Compile_Int8( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 1, value )
#define _Compile_Int16( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 2, value )
#define _Compile_Int32( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 4, value )
#define _Compile_Int64( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 8, value )
#define _Compile_Cell( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, sizeof(int32), value )
#define Here ( _ByteArray_Here ( _Q_CodeByteArray ) )
#define SetHere( address )  _ByteArray_SetHere_AndForDebug ( _Q_CodeByteArray, address ) 
#define Set_CompilerSpace( byteArray ) (_Q_CodeByteArray = (byteArray))
#define Get_CompilerSpace( ) _Q_CodeByteArray

#define TOS ( Dsp [ 0 ] )
#define _Drop() _DataStack_Drop ( ) //(Dsp --)
#define _DropN( n ) (Dsp -= (int32) n )
#define _Push( v ) _DataStack_Push ( (int32) v ) //(*++Dsp = (int32) v )
#define _Pop() _DataStack_Pop () // ( Dsp -- [ 0 ] ) 
#define _Dup() { Dsp [ 1 ] = TOS ;  Dsp ++ ; }
#define _Top( ) TOS 
#define _DataStack_Top( ) TOS 
#define _DataStack_GetTop( ) TOS
#define _DataStack_SetTop( v ) TOS = v 
#define _GetTop( ) TOS
#define _SetTop( v ) (TOS = v)
#define Stack() CfrTil_PrintDataStack ( )

#define Calculate_FrameSize( numberOfLocals )  ( ( numberOfLocals + 1 ) * CELL ) // 1 : space for fp
#define ParameterVarOffset( word ) ( - ( _Context_->Compiler0->NumberOfParameterVariables - word->Index + 1 ) )
#define LocalVarOffset( word ) ( word->Index )

#define _GetState( aState, state ) ( (aState) & (state) ) 
#define GetState( obj, state ) _GetState( (obj)->State, state ) 
#define _SetState( state, newState, flag ) ( ( (flag) > 0 ) ? ( (state) |= (newState) ) : ( (state) &= ~ (newState) ) ) 
#define SetState_TrueFalse( obj, _true, _false )  ( ( (obj)->State |= (_true) ), ( (obj)->State &= ~ (_false) ) ) 
#define SetState( obj, state, flag ) _SetState ( ((obj)->State), (state), flag )
#define Debugger_IsStepping( debugger ) GetState ( debugger, DBG_STEPPING )
#define Debugger_SetStepping( debugger, flag ) SetState ( debugger, DBG_STEPPING, flag )  
#define Debugger_IsRestoreCpuState( debugger ) GetState ( debugger, DBG_RESTORE_REGS )
#define Debugger_SetRestoreCpuState( debugger, flag ) SetState ( debugger, DBG_RESTORE_REGS, flag ) 
#define Debugger_SetMenu( debugger, flag ) SetState ( debugger, DBG_MENU, flag )
#define Debugger_IsDone( debugger ) GetState ( debugger, DBG_DONE )
#define Debugger_SetDone( debugger, flag ) SetState ( debugger, DBG_DONE, flag ) 
#define Debugger_IsNewLine( debugger ) GetState ( debugger, DBG_NEWLINE )
#define Debugger_SetNewLine( debugger, flag ) SetState ( debugger, DBG_NEWLINE, flag ) 

#define Set_CompileMode( tf ) SetState ( _Context_->Compiler0, COMPILE_MODE, tf ) ; _Q_->OVT_LC ? SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, tf ) : 0 ; 
#define Get_CompileMode ( GetState ( _Context_->Compiler0, COMPILE_MODE ) || ( _Q_->OVT_LC ? SetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) : 0 ) ) 
#define CompileMode ( GetState ( _Context_->Compiler0, COMPILE_MODE ) || ( _Q_->OVT_LC && GetState ( _Q_->OVT_LC, ( LC_COMPILE_MODE ) ) ) )
#define Compiling CompileMode
#define ImmediateWord( word) (word->CProperty & IMMEDIATE)
#define CPrimitiveWord( word) (word->CProperty & CPRIMITIVE)

#define Stack_N( stack, offset ) ((stack)->StackPointer [ (offset) ] )
#define Stack_OffsetValue( stack, offset ) ((stack)->StackPointer [ (offset) ] )
//#define _Compiler_WordStack( compiler, n ) ((Word*) (Stack_OffsetValue ( (compiler)->WordStack, (n))))
//#define Compiler_WordStack( n ) ((Word*) _Compiler_WordStack( _Context_->Compiler0, (n) ))
//#define _Compiler_WordList( compiler, n ) ((Word*) (List_GetN ( (compiler)->WordList, (n))))
//#define Compiler_WordList( n ) ((Word*) _Compiler_WordList( _Context_->Compiler0, (n) ))
//#define CompilerWordStack _Context_->Compiler0->WordStack
//#define CompilerLastWord Compiler_WordStack( 0 )
//#define WordsBack( n ) Compiler_WordStack( (-n) )
#define WordsBack( n ) Compiler_WordList( (n) )
#define IncrementCurrentAccumulatedOffset( increment ) \
        {\
            if ( _Context_->Compiler0->AccumulatedOffsetPointer )\
            {\
                ( *( int32* ) (_Context_->Compiler0->AccumulatedOffsetPointer) ) += (increment) ;\
            }\
            if ( _Context_->Compiler0->AccumulatedOptimizeOffsetPointer )\
            {\
                ( *( int32* ) (_Context_->Compiler0->AccumulatedOptimizeOffsetPointer) ) += (increment) ;\
            }\
        }

#define SetCurrentAccumulatedOffset( value ) \
        {\
            if ( _Context_->Compiler0->AccumulatedOffsetPointer )\
            {\
                ( *( int32* ) (_Context_->Compiler0->AccumulatedOffsetPointer) ) = (value) ;\
            }\
            if ( _Context_->Compiler0->AccumulatedOptimizeOffsetPointer )\
            {\
                ( *( int32* ) (_Context_->Compiler0->AccumulatedOptimizeOffsetPointer) ) = (value) ;\
            }\
        }

#define B_FREE  0
#define B_UNLOCKED 1
#define B_LOCKED  2
#define B_PERMANENT 3
#define Buffer_Data( b ) b->B_Data
#define Buffer_DataCleared( b ) Buffer_Clear (b) 
#define Buffer_Size( b ) b->B_Size
#define SetBuffersUnused Buffers_SetAsUnused ( ) 
#define Buffer_MakePermanent( b ) b->InUseFlag = B_PERMANENT
#define Buffer_Lock( b ) b->InUseFlag = B_LOCKED
#define Buffer_Unlock( b ) b->InUseFlag = B_UNLOCKED
#define Buffer_AllowReUse( b ) b->InUseFlag = B_FREE 
#define _Buffer_SetAsUnused( b )  b->InUseFlag = B_FREE 

#define Property_FromWord( word ) (( Property * ) (word)->This )

// formatting
#define AllowNewlines if (_Q_) SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, false ) 
#define ConserveNewlines if (_Q_) SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) 
// ansi/vt102 escape code
#define Color_Black 0
#define Color_Red 1
#define Color_Green 2
#define Color_Yellow 3
#define Color_Blue 4
#define Color_Magenta 5
#define Color_Cyan 6
#define Color_White 7
#define Color_Default 9

#define Colors_Setup6( c, fr, fg, fb, br, bg, bb )\
    int fr, fg, fb, br, bg, bb ;\
    fr = c->rgbcs_RgbColors.rgbc_Fg.Red, fg = c->rgbcs_RgbColors.rgbc_Fg.Green, fb = c->rgbcs_RgbColors.rgbc_Fg.Blue ;\
    br = c->rgbcs_RgbColors.rgbc_Bg.Red, bg = c->rgbcs_RgbColors.rgbc_Bg.Green, bb = c->rgbcs_RgbColors.rgbc_Bg.Blue ;

#define Colors_Get6( c, fr, fg, fb, br, bg, bb )\
    c->rgbcs_RgbColors.rgbc_Fg.Red = fr, c->rgbcs_RgbColors.rgbc_Fg.Green = fg, c->rgbcs_RgbColors.rgbc_Fg.Blue = fb ;\
    c->rgbcs_RgbColors.rgbc_Bg.Red = br, c->rgbcs_RgbColors.rgbc_Bg.Green = bg, c->rgbcs_RgbColors.rgbc_Bg.Blue = bb ;

#define _Show2Colors( fg, bg ) printf ( "%c[%d;%dm", ESC, fg, bg )
#define _ShowColors( fg, bg ) _Show2Colors( fg + 30, bg + 40 )
#define _String_Show2( buf, fg, bg ) sprintf ( (char*) buf, "%c[%d;%dm", ESC, fg, bg )
#define _String_ShowColors( buf, fg, bg ) _String_Show2 ( buf, fg + 30, bg + 40 )
#define COLORS_ON 1
#if COLORS_ON 
#define DefaultColors Ovt_DefaultColors () 
#define AlertColors Ovt_AlertColors () 
#define DebugColors Ovt_DebugColors () 
#define NoticeColors Ovt_NoticeColors () 

// Change Colors
#define cc( s, c ) (byte*) _String_InsertColors ( (byte*) ( (byte*) s ? (byte*) s : (byte*) "" ), (c) ) 
#define c_ud( s ) cc ( (byte*) s, (_Q_->Current == &_Q_->User) ? &_Q_->Default : &_Q_->User ) 
#define c_ad( s ) cc ( (byte*) s, (_Q_->Current == &_Q_->Alert) ? &_Q_->Default : &_Q_->Alert ) 
#define c_dd( s ) cc ( (byte*) s, (_Q_->Current == &_Q_->Debug) ? &_Q_->Default : &_Q_->Debug ) 
#define c_du( s ) cc ( (byte*) s, (_Q_->Current == &_Q_->Debug) ? &_Q_->User : &_Q_->Debug ) 
#else
#define DefaultColors 
#define AlertColors 
#define DebugColors 
#define NoticeColors 
#define cc( s, c ) s
#define c_ud( s ) s
#define c_ad( s ) s
#define c_dd( s ) s
#endif

#define _Context_ _Q_->OVT_Context
#define _CfrTil_ _Q_->OVT_CfrTil
#define _Compiler_ _Context_->Compiler0
#define _Interpreter_ _Context_->Interpreter0
#define _DataStack_ _Q_->OVT_CfrTil->DataStack
#define _DataStackPointer_ _DataStack_->StackPointer
#define _SP_ _DataStackPointer_ 
#define _AtCommandLine() ( ! _Context_->System0->IncludeFileStackNumber ) 
#define AtCommandLine( rl ) \
        ( GetState ( _Debugger_, DBG_COMMAND_LINE ) || \
        ( GetState ( rl, CHAR_ECHO ) && ( ! _Context_->System0->IncludeFileStackNumber ) ) ) // ?? essentially : at a command line ??
#define SessionString_New( string ) String_New ( string, SESSION ) 
#define TemporaryString_New( string ) String_New ( string, TEMPORARY ) 
#define IsWordRecursive CfrTil_CheckForGotoPoints ( GI_RECURSE )
#define AppendCharToSourceCode( c ) //_Lexer_AppendCharToSourceCode ( lexer, c ) 
#define ReadLine_Nl (ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) == '\n')
#define ReadLine_Eof (ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) == eof)
#define ReadLine_ClearLineQuick _Context_->ReadLiner0->InputLine [ 0 ] = 0 
#define _ReadLine_CursorPosition( rl ) (rl->CursorPosition)
#define ReadLine_GetCursorChar( rl ) (rl->InputLine [ _ReadLine_CursorPosition (rl) ])
#define ReadLine_SetCursorChar( rl, c ) (rl->InputLine [ _ReadLine_CursorPosition (rl) ] = c )
#define Stack_Clear( stk ) Stack_Init ( stk )

// exception handling
#define _try( object ) if ( _OpenVmTil_Try ( &object->JmpBuf0 ) ) 
//#define _catch( e ) if ( _OpenVmTil_Catch () ) // nb. : if no _throw in _catch block don't use 'return'
#define _finally _OpenVmTil_Finally () // nb. : ! use only once and after the first _try block !
#define _throw( e ) _Throw (e) // _longjmp( *(jmp_buf*) _Stack_PopOrTop ( _Q_->ExceptionStack ), e ) 
#define _Throw( e ) OpenVmTil_Throw ( (byte*) "", e )
#define _ThrowIt OpenVmTil_Throw ( (byte*) "",  _Q_->Thrown )
#define Throw( msg, e ) OpenVmTil_Throw ( (byte*) msg, e )
#define ThrowIt( msg ) OpenVmTil_Throw ( (byte*) msg,  _Q_->Thrown )
#define catchAll if ( _OpenVmTil_Catch () ) 
#define SyntaxError( abortFlag ) CfrTil_Exception ( SYNTAX_ERROR, abortFlag )
#define stopThisTry _OVT_PopExceptionStack ( )
#define stopTrying _OVT_ClearExceptionStack ( )

#define Assert( testBoolean ) d1 ({ if ( ! (testBoolean) ) Printf ( "\n\nAssert failed : %s\n\n", _Context_Location ( _Context_ ) ) ; _throw ( QUIT ) ; })
#define Pause _OpenVmTil_Pause
#define Pause_1( msg ) AlertColors; Printf ( (byte*)"\n%s", msg ) ; _OpenVmTil_Pause () ;
#define Pause_2( msg, arg ) AlertColors; Printf ( (byte*)msg, arg ) ; _OpenVmTil_Pause () ;

#define Error_Abort( msg ) Throw ( (byte*) msg, ABORT )
#define Error( msg, state ) { AlertColors; Printf ( (byte*)"\n%s", (byte*) msg, state ) ; if ((state) & PAUSE ) Pause ; if ((state) >= QUIT ) Throw ( (byte*) msg, state ) ; }
#define Error_1( msg, arg, state ) AlertColors; Printf ( (byte*)"\n%s : %d", (byte*) msg, arg ) ; if (state & PAUSE ) Pause_0 () ; if (state >= QUIT ) Throw ( (byte*) msg, state ) ; 
#define Warning2( msg, str ) Printf ( (byte*)"\n%s : %s", (byte*) msg, str ) ; 
#define ErrorWithContinuation( msg, continuation ) Throw ( (byte*) msg, continuation )
#define Error_Quit( msg ) ErrorWithContinuation( msg, QUIT )
#define ErrorN( n ) Throw ( (byte*) "", n )
#define ClearLine _ReadLine_PrintfClearTerminalLine ( )

// !! Get - Set - Exec !!
#define Get( obj, field ) obj->field
#define Set( obj, field, value ) (obj)->(field) = (value) 

// lisp macros
#define Cons( first, second ) _LO_Cons ( (first), (second), LispAllocType )
#define Car( sym ) ((ListObject*) sym)->Lo_Car
#define Cdr( sym ) ((ListObject*) sym)->Lo_Cdr

#define String_Equal( string1, string2 ) (strcmp ( (char*) string1, (char*) string2 ) == 0 )
#define String_CB( string0 ) String_ConvertToBackSlash ( string0 )

#define DEBUG_PRINTSTACK if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE )  ) CfrTil_PrintDataStack () ;
#define TypeNamespace_Get( object ) (object->TypeNamespace ? object->TypeNamespace : object->ContainingNamespace)
#define _Lexer_IsCharDelimiter( lexer, c ) lexer->DelimiterCharSet [ c ]
#define _Lexer_IsCharDelimiterOrDot( lexer, c ) lexer->DelimiterOrDotCharSet [ c ]

#if 1
#define NAMESPACE_TYPE ( NAMESPACE | DOBJECT | CLASS | C_TYPE | C_CLASS | CLASS_CLONE )
#define NAMESPACE_RELATED_TYPE ( NAMESPACE_TYPE | OBJECT_FIELD )
//#define OBJECT_TYPES ( OBJECT | DOBJECT | THIS | VARIABLE | LOCAL_VARIABLE | PARAMETER_VARIABLE | OBJECT_FIELD | CONSTANT | C_TYPE | C_CLASS | CLASS_CLONE )
#define OBJECT_TYPE ( LITERAL | CONSTANT | VARIABLE | LOCAL_VARIABLE | OBJECT | DOBJECT | PARAMETER_VARIABLE )
//#define OBJECT_TYPE ( CONSTANT | LITERAL | VARIABLE | LOCAL_VARIABLE | OBJECT | DOBJECT | PARAMETER_VARIABLE )
#define NON_MORPHISM_TYPE ( OBJECT_TYPE | NAMESPACE_RELATED_TYPE )
#define IS_MORPHISM_TYPE( word ) (( ! ( word->CProperty & ( NON_MORPHISM_TYPE | OBJECT_OPERATOR ) ) ) || ( word->CProperty & ( KEYWORD ) ))
//#define IS_MORPHISM_TYPE( word ) (( ! ( ( word->CProperty & ( NON_MORPHISM_TYPE | OBJECT_OPERATOR ) ) || ( word->LType & T_LISP_SYMBOL ) || ( CompileMode && ( word->CProperty & ( LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) ) ) ) || ( word->CProperty & ( KEYWORD ) ))
//#define IS_MORPHISM_TYPE( word ) (( ! ( word->CProperty & OBJECT_OPERATOR ) ) || ( word->CProperty & ( KEYWORD ) ))
// #define NON_MORPHISM_TYPE ( CONSTANT | VARIABLE | LOCAL_VARIABLE | NAMESPACE | CLASS | OBJECT_FIELD | OBJECT | DOBJECT | C_TYPE | C_CLASS | CLASS_CLONE | PARAMETER_VARIABLE )
#else
#define NAMESPACE_TYPES ( NAMESPACE | DOBJECT | OBJECT | CLASS  )
#define OBJECT_TYPES ( DOBJECT | OBJECT )
#endif

#define Is_NamespaceType( w ) ( w ? ( ( Namespace* ) w )->CProperty & NAMESPACE_TYPE : 0 )
#define Is_ValueType( w ) ( w ? ( ( Namespace* ) w )->CProperty & NON_MORPHISM_TYPE : 0 )
#define String_Init( s ) s[0]=0 ; 

// memory allocation
#define _Object_Allocate( size, allocType ) Mem_Allocate ( size, allocType ) 
#define Object_Allocate( type, slots, allocType ) (type *) _Object_Allocate ( (sizeof ( type ) + ((slots -1) * CELL)), allocType ) 
//#define mmap_AllocMem( size ) (byte*) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
#define _Allocate( size, nba ) _ByteArray_AppendSpace ( nba->ba_CurrentByteArray, size ) 

#define Get_NBA_Symbol_To_NBA( s )  ( NamedByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data ) 
#define Get_NBA_Node_To_NBA( node )  ( NamedByteArray* ) ( ( ( Symbol* ) node )->S_pb_Data ) 
#define Get_BA_Symbol_To_BA( s )  ( ByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data ) 
#define Set_NBA_Symbol_To_NBA( nba )  nba->NBA_Symbol.S_pb_Data = ( byte* ) nba
#define Set_BA_Symbol_To_BA( ba )  ba->BA_Symbol.S_pb_Data = ( byte* ) ba
#define MemCheck( block ) { _Calculate_CurrentNbaMemoryAllocationInfo ( 1 ) ; block ; _Calculate_CurrentNbaMemoryAllocationInfo ( 1 ) ; }

#define _Debugger_ _CfrTil_->Debugger0
#define IS_DEBUG_MODE ( _Q_->OVT_CfrTil && GetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_ ) && ( ! GetState ( _Debugger_, ( DBG_DONE | DBG_STEPPING | DBG_SKIP_INNER_SHOW ) ) ) )
#define Is_DebugShow GetState ( _Q_->OVT_CfrTil, _DEBUG_SHOW_ )
#define IS_DEBUG_SHOW_MODE ( Is_DebugOn && Is_DebugShow && ( ! GetState ( _Debugger_, ( DBG_DONE | DBG_STEPPING | DBG_SKIP_INNER_SHOW ) ) ) )
#define Is_DebugOn IS_DEBUG_MODE
#define DebugOff SetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_, false )
#define DebugOn SetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_, true ) 

#define DBG_STATE_STACK _Q_->OVT_CfrTil->DebugStateStack
#define DebugShow_Off _Stack_Push ( DBG_STATE_STACK, GetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_ ) ) ; SetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_, false ) 
#define DebugShow_On _Stack_Push ( DBG_STATE_STACK, GetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_ ) ) ; SetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_, true ) 
#define DebugShow_StateRestore SetState ( _Q_->OVT_CfrTil, DEBUG_MODE|_DEBUG_SHOW_, _Stack_Pop ( DBG_STATE_STACK ) )
#define DebugShow_OFF Stack_Init ( DBG_STATE_STACK ) ; SetState ( _Q_->OVT_CfrTil, _DEBUG_SHOW_, false ) 
#define DebugShow_ON SetState ( _Q_->OVT_CfrTil, _DEBUG_SHOW_, true ) 
#define Is_DebugLevel( n ) ( _Q_->Verbosity >= ( n ) )
#define DEBUG_SETUP _Debugger_PreSetup ( _Debugger_, 0 )//, token, word ) ;
#define _DEBUG_SETUP( word ) if ( word && IS_DEBUG_MODE ) _Debugger_PreSetup ( _Debugger_, word ) ;
#define DEBUG_SHOW _Debugger_PostShow ( _Debugger_ ) ; //, token, word ) ;
#define DEBUB_WORD( word, block ) _DEBUG_SETUP( word ) ; block ; DEBUG_SHOW
#define Debugger_WrapBlock( word, block ) _DEBUG_SETUP( word ) ; block ; DEBUG_SHOW

#define IsLValue( word ) ( GetState ( _Context_->Compiler0, LC_ARG_PARSING ) ? 0 : Interpret_CheckEqualBeforeSemi_LValue ( word ))
#define IS_INCLUDING_FILES _Context_->System0->IncludeFileStackNumber

#define List_Push( list, value, allocType ) _DLList_PushValue ( list, ((int32) value), allocType )
#define List_Pop( list ) _DLList_PopValue ( list )
#define List_Init( list ) _DLList_Init ( list )
#define List_DropN( list, n ) _DLList_DropN ( list, n )
#define List_GetN( list, n ) _DLList_GetNValue ( list, n )
#define List_Depth( list ) _DLList_Depth ( list )

