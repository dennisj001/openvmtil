
//#define myprintf(a, b, rest...) printf (a, b, ## rest)

#define Stack_Pop(stack) Stack_Pop_WithExceptionOnEmpty ( stack )

#define _Q_CodeByteArray _Q_->CodeByteArray
#define _Q_CodeSpace _Q_->MemorySpace0->CodeSpace
#define _Compile_Int8( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 1, value )
//#define Compile_Int8( opCode ) Compile_StartOpCode_Int8 ( opCode ) 
#define _Compile_Int16( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 2, value )
#define _Compile_Int32( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 4, value )
#define _Compile_Int64( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, 8, value )
#define _Compile_Cell( value ) ByteArray_AppendCopyItem ( _Q_CodeByteArray, sizeof(int32), value )
#define Here ( _ByteArray_Here ( _Q_CodeByteArray ) )
#define SetHere( address )  _ByteArray_SetHere_AndForDebug ( _Q_CodeByteArray, address ) 
#define Set_CompilerSpace( byteArray ) (_Q_CodeByteArray = (byteArray))
#define Get_CompilerSpace( ) _Q_CodeByteArray

#define TOS ( Dsp [ 0 ] )
#define DSP_Drop() _DataStack_Drop ( ) //(Dsp --)
#define DSP_DropN( n ) (Dsp -= (int32) n )
#define DSP_Push( v ) _DataStack_Push ( (int32) v ) //(*++Dsp = (int32) v )
#define DSP_Pop() _DataStack_Pop () // ( Dsp -- [ 0 ] ) 
#define DSP_Dup() _DataStack_Dup ()
#define DSP_Top( ) TOS 
#define _DataStack_Top( ) TOS 
#define _DataStack_GetTop( ) TOS
#define _DataStack_SetTop( v ) TOS = v 
#define _GetTop( ) TOS
#define _SetTop( v ) (TOS = v)
#define Stack() CfrTil_PrintDataStack ( )

#define Calculate_FrameSize( numberOfLocals )  ( ( numberOfLocals + 1 ) * CELL ) // 1 : space for fp
#define ParameterVarOffset( word ) ( - ( _Context_->Compiler0->NumberOfArgs - word->Index + 1 ) )
#define LocalVarOffset( word ) ( word->Index )

#define _GetState( aState, state ) ( (aState) & (state) ) 
#define GetState( obj, state ) ((obj) && _GetState( (obj)->State, state )) 
#define _SetState( state, newState, flag ) ( ( (flag) > 0 ) ? ( (state) |= (newState) ) : ( (state) &= ~ (newState) ) ) 
#define SetState_TrueFalse( obj, _true, _false )  ( ( (obj)->State |= (_true) ), ( (obj)->State &= ~ (_false) ) ) 
#define SetState( obj, state, flag ) _SetState ( ((obj)->State), (state), flag )
#define Debugger_IsStepping( debugger ) GetState ( debugger, DBG_STEPPING )
#define Debugger_SetStepping( debugger, flag ) SetState ( debugger, DBG_STEPPING, flag )  
//#define Debugger_IsRestoreCpuState( debugger ) GetState ( debugger, DBG_RESTORE_REGS )
//#define Debugger_SetRestoreCpuState( debugger, flag ) SetState ( debugger, DBG_RESTORE_REGS, flag ) 
#define Debugger_SetMenu( debugger, flag ) SetState ( debugger, DBG_MENU, flag )
#define Debugger_IsDone( debugger ) GetState ( debugger, DBG_DONE )
#define Debugger_SetDone( debugger, flag ) SetState ( debugger, DBG_DONE, flag ) 
#define Debugger_IsNewLine( debugger ) GetState ( debugger, DBG_NEWLINE )
#define Debugger_SetNewLine( debugger, flag ) SetState ( debugger, DBG_NEWLINE, flag ) 

#define Set_CompileMode( tf ) SetState ( _Context_->Compiler0, COMPILE_MODE, tf ) ; _Q_->OVT_LC ? SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, tf ) : 0 ; 
#define Get_CompileMode() ( GetState ( _Context_->Compiler0, COMPILE_MODE ) || ( _Q_->OVT_LC ? GetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) : 0 ) ) 
#define CompileMode (_Context_ ? ( GetState ( _Context_->Compiler0, COMPILE_MODE ) || ( _Q_->OVT_LC && GetState ( _Q_->OVT_LC, ( LC_COMPILE_MODE ) ) ) ) : 0)
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
#define B_FREE  1
#define B_UNLOCKED 2
#define B_LOCKED  4
#define B_IN_USE B_LOCKED
#define B_PERMANENT 8
#define Buffer_Data( b ) b->B_Data
#define Buffer_DataCleared( b ) Buffer_Data_Cleared (b) 
#define Buffer_Size( b ) b->B_Size
#define SetBuffersUnused( force ) Buffers_SetAsUnused ( force ) 
#define Buffer_MakePermanent( b ) b->InUseFlag = B_PERMANENT
#define Buffer_Lock( b ) b->InUseFlag = B_LOCKED
#define Buffer_Unlock( b ) b->InUseFlag = B_UNLOCKED
//#define Buffer_AllowReUse( b ) b->InUseFlag = B_FREE 
#define _Buffer_SetAsFree( b )  b->InUseFlag = B_FREE 

#define Property_FromWord( word ) (( Property * ) (word)->This )

// formatting
//#define AllowNewlines //if (_Q_) SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, false ) 
//#define ConserveNewlines //if (_Q_) SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) 
// ansi/vt102 escape code
#define ClearLine _ReadLine_PrintfClearTerminalLine ( )
#define Cursor_Up( n ) _Printf ( (byte*) "%c[%dA", ESC, n )
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

#define _Context_ _Q_->OVT_Context
#define _CfrTil_ _Q_->OVT_CfrTil
#define _Compiler_ _Context_->Compiler0
#define _Interpreter_ _Context_->Interpreter0
#define _ReadLiner_ _Context_->ReadLiner0
#define _Lexer_ _Context_->Lexer0
#define _Finder_ _Context_->Finder0
#define _DataStack_ _CfrTil_->DataStack
#define _DataStackPointer_ _DataStack_->StackPointer
#define _SP_ _DataStackPointer_ 
#define _AtCommandLine() ( ! _Context_->System0->IncludeFileStackNumber ) 
#define AtCommandLine( rl ) \
        ( GetState ( _Debugger_, DBG_COMMAND_LINE ) || GetState ( _Context_, AT_COMMAND_LINE ) ||\
        ( GetState ( rl, CHAR_ECHO ) && ( ! _Context_->System0->IncludeFileStackNumber ) ) ) // ?? essentially : at a command line ??
//#define SessionString_New( string ) String_New ( string, SESSION ) 
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
#define _Throw( e ) OpenVmTil_Throw ( (e == QUIT) ? (byte*) "\nQuit?\n" : (e == ABORT) ? (byte*) "\nAbort?\n" : (byte*) "", e, 1 )
#define _ThrowIt OpenVmTil_Throw ( (byte*) "",  _Q_->Thrown, 1 )
#define Throw( msg, e ) OpenVmTil_Throw ( (byte*) msg, e, 1 )
#define ThrowIt( msg ) OpenVmTil_Throw ( (byte*) msg,  _Q_->Thrown, 1 )
#define catchAll if ( _OpenVmTil_Catch () ) 
#define SyntaxError( abortFlag ) CfrTil_Exception ( SYNTAX_ERROR, abortFlag )
#define stopThisTry _OVT_PopExceptionStack ( )
#define stopTrying _OVT_ClearExceptionStack ( )

#define Assert( testBoolean ) d1 ({ if ( ! (testBoolean) ) { _Printf ( (byte*) "\n\nAssert failed : %s\n\n", _Context_Location ( _Context_ ) ) ; _throw ( QUIT ) ; }})
#define Pause() OpenVmTil_Pause ()
#define _Pause( msg ) _OpenVmTil_Pause ( msg )
#define Pause_1( msg ) AlertColors; _Printf ( (byte*)"\n%s", msg ) ; OpenVmTil_Pause () ;
#define Pause_2( msg, arg ) AlertColors; _Printf ( (byte*)msg, arg ) ; OpenVmTil_Pause () ;

#define Error_Abort( msg ) Throw ( (byte*) msg, ABORT )
#define Error( msg, state ) { AlertColors; _Printf ( (byte*)"\n\n%s\n\n", (byte*) msg, state ) ; if ((state) & PAUSE ) Pause () ; if ((state) >= QUIT ) Throw ( (byte*) msg, state ) ; }
#define Error_1( msg, arg, state ) AlertColors; _Printf ( (byte*)"\n%s : %d\n\n", (byte*) msg, arg ) ; if (state & PAUSE ) Pause () ; if (state >= QUIT ) Throw ( (byte*) msg, state ) ; 
#define Warning2( msg, str ) _Printf ( (byte*)"\n%s : %s", (byte*) msg, str ) ; 
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

#define String_Equal( string1, string2 ) (strncmp ( (char*) string1, (char*) string2, 128 ) == 0 )
#define sconvbs( d, s ) (byte*) _String_ConvertStringToBackSlash ( d, s )
#define String_CB( string0 ) String_ConvertToBackSlash ( string0 )

#define DEBUG_PRINTSTACK if ( GetState ( _CfrTil_, DEBUG_MODE )  ) CfrTil_PrintDataStack () ;
#define TypeNamespace_Get( object ) (object->TypeNamespace ? object->TypeNamespace : object->ContainingNamespace)
#define _Lexer_IsCharDelimiter( lexer, c ) lexer->DelimiterCharSet [ c ]
#define _Lexer_IsCharDelimiterOrDot( lexer, c ) lexer->DelimiterOrDotCharSet [ c ]

#define NAMESPACE_TYPE ( NAMESPACE | DOBJECT | CLASS | C_TYPE | C_CLASS | CLASS_CLONE )
#define NAMESPACE_RELATED_TYPE ( NAMESPACE_TYPE | OBJECT_FIELD )
#define OBJECT_TYPE ( LITERAL | CONSTANT | NAMESPACE_VARIABLE | LOCAL_VARIABLE | OBJECT | DOBJECT | PARAMETER_VARIABLE )
#define _NON_MORPHISM_TYPE ( OBJECT_TYPE | NAMESPACE_RELATED_TYPE )
#define NON_MORPHISM_TYPE(word) (word->CProperty & _NON_MORPHISM_TYPE)
#define IS_MORPHISM_TYPE( word ) ( ( ( ! ( word->CProperty & ( _NON_MORPHISM_TYPE | OBJECT_OPERATOR ) ) ) && ( ! ( word->LProperty & ADDRESS_OF_OP ) ) ) || ( word->CProperty & ( KEYWORD|BLOCK ) ))

#define Is_NamespaceType( w ) ( w ? (( ( Namespace* ) w )->CProperty & NAMESPACE_TYPE) : 0 )
#define Is_ValueType( w ) ( w ? ( ( Namespace* ) w )->CProperty & (_NON_MORPHISM_TYPE (w)) : 0 )
#define String_Init( s ) s[0]=0 ; 

// memory allocation
#define _Allocate( size, nba ) _ByteArray_AppendSpace ( nba->ba_CurrentByteArray, size ) 
#define object_Allocate( type, slots, allocType ) (type *) _object_Allocate ( sizeof ( type ) * slots, allocType ) 
#define _listObject_Allocate( nodeType, slotType, slots, allocType ) (type *) _object_Allocate ( sizeof ( nodeType ) + (sizeof ( slotType ) * slots), allocType ) 

#define Get_NBA_Symbol_To_NBA( s )  ( NamedByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data2 ) 
#define Get_NBA_Node_To_NBA( node )  ( NamedByteArray* ) ( ( ( Symbol* ) node )->S_pb_Data2 ) 
#define Get_BA_Symbol_To_BA( s )  ( ByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data2 ) 
#define Set_NBA_Symbol_To_NBA( nba )  nba->NBA_Symbol.S_pb_Data2 = ( byte* ) nba
#define Set_BA_Symbol_To_BA( ba )  ba->BA_Symbol.S_pb_Data2 = ( byte* ) ba
#define MemCheck( block ) { _Calculate_TotalNbaAccountedMemAllocated ( 1 ) ; block ; _Calculate_TotalNbaAccountedMemAllocated ( 1 ) ; }

#define _Debugger_ _CfrTil_->Debugger0
#define DebugOff SetState ( _CfrTil_, DEBUG_MODE, false )
#define DebugOn SetState ( _CfrTil_, DEBUG_MODE|_DEBUG_SHOW_, true ) 
#define DebugShow_Off SetState ( _CfrTil_, _DEBUG_SHOW_, false ) 
#define DebugShow_On SetState ( _CfrTil_, _DEBUG_SHOW_, true ) 
#define _Is_DebugOn ( _CfrTil_ && GetState ( _CfrTil_, DEBUG_MODE ) && GetState ( _CfrTil_, _DEBUG_SHOW_ ) )
#define Is_DebugOn ( _Is_DebugOn && ( ! GetState ( _Debugger_, ( DBG_DONE ) ) ) )
#define Is_DebugShow GetState ( _CfrTil_, _DEBUG_SHOW_ )
#define DEBUG_SETUP( word ) if ( word && Is_DebugOn) _Debugger_PreSetup ( _Debugger_, word ) ;
#define _DEBUG_SHOW( word ) _Debugger_PostShow ( _Debugger_, word ) ; //, token, word ) ;
#define DEBUG_SHOW Debugger_PostShow ( _Debugger_ ) ; //, token, word ) ;

#define Is_LValue( word ) ( GetState ( _Context_->Compiler0, LC_ARG_PARSING ) ? 0 : Interpret_CheckEqualBeforeSemi_LValue ( word ))
#define IS_INCLUDING_FILES _Context_->System0->IncludeFileStackNumber

#define dobject_Get_M_Slot( dobj, m ) (((dobject*) dobj)->do_iData [m]) 
#define dobject_Set_M_Slot( dobj, m, value ) (((dobject*) dobj)->do_iData [m] = ((int32)value) ) 
#define List_Set_N_Node_M_Slot( list, n, m, value ) _dllist_Set_N_Node_M_Slot ( list, 0, 0, value ) 
#define List_Get_N_Node_M_Slot( list, n, m ) _dllist_Get_N_Node_M_Slot ( (dllist * )list, (int32) n, (int32) m )
// List_* macros when not generic refer to a single valued node list
#define List_Init( list ) _dllist_Init ( list )
#define List_DropN( list, n ) _dllist_DropN ( list, n )
#define List_GetN( list, n ) List_Get_N_Node_M_Slot( list, n, 0 )  //_dllist_GetTopValue ( list )
#define List_SetTop( list, value ) List_Set_N_Node_M_Slot( list, 0, 0, value )
#define List_Pop( list ) _dllist_PopNode ( list )
#define List_Top( list ) List_GetN( list, 0 ) 
#define List_Depth( list ) _dllist_Depth ( list )
#define List_Length( list ) List_Depth ( list )
#define List_New() _dllist_New ( TEMPORARY ) 
#define List_Push_1Value_Node( list, value, allocType ) _dllist_Push_M_Slot_Node ( list, WORD, allocType, 1, ((int32) value) )
//#define List_Push_2Value_Node( list, value1, value2 ) _dllist_Push_M_Slot_Node ( list, WORD, DICTIONARY, 2, ((int32) value1), ((int32) value2) )
#define List_Push( list, value, allocType ) List_Push_1Value_Node ( list, value, allocType )
#define List_PushNode( list, node ) _dllist_AddNodeToHead ( list, ( dlnode* ) node )

#define WordList_Pop( list, m ) dobject_Get_M_Slot ( _dllist_PopNode ( list ), m ) 
//#define DebugWordList_PushNewNode( codePtr, scOffset ) _dllist_Push_M_Slot_Node ( _CfrTil_->DebugWordList, WORD_LOCATION, TEMPORARY, 3, ((int32) codePtr), (int32) scOffset )
#define DebugWordList_Push( dobj ) _dllist_AddNodeToHead ( _CfrTil_->DebugWordList, ( dlnode* ) dobj )
#define DbgWL_Node_SetCodeAddress( dobj, address ) dobject_Set_M_Slot( dobj, 1, adress ) 
#define DbgWL_Push( node ) DebugWordList_Push( node )  
#define Node_New_ForDebugWordList( allocType, scindex, word ) _dobject_New_M_Slot_Node ( allocType, WORD_LOCATION, 3, 0, scindex, word ) 
#define CompilerWordList_Push( word, dnode ) _dllist_Push_M_Slot_Node ( _Compiler_->WordList, WORD, TEMPORARY, 2, ((int32) word), ((int32) dnode) )
#define IsGlobalsSourceCodeOn ( GetState ( _CfrTil_, GLOBAL_SOURCE_CODE_MODE ))
#define _IsSourceCodeOn ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
#define IsSourceCodeOn ( _IsSourceCodeOn || IsGlobalsSourceCodeOn )
#define IsSourceCodeOff (!IsSourceCodeOn) //( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) || IsGlobalsSourceCodeOn ))
#define Set_SCA( index ) SC_DWL_PushCWL_Index ( index )
//#define SC_Push( word ) DebugWordList_PushWord ( word ) 
#define SC_DWL_Push( word ) DebugWordList_PushWord ( word ) 
#define SC_SetForcePush( tf ) SetState ( _CfrTil_, SC_FORCE_PUSH, tf ) 
#define _SC_Global_On SetState ( _CfrTil_, GLOBAL_SOURCE_CODE_MODE, true )
#define SC_Global_On if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ) { _SC_Global_On ; }
#define SC_Global_Off SetState ( _CfrTil_, GLOBAL_SOURCE_CODE_MODE, false )
#define _Block_SCA( index ) _CfrTil_Block_SetSourceCodeAddress( index )
#define _Block_SCA_Clear _Block_SCA( -1 ) ;
#define Compiler_OptimizerWordList_Reset( compiler ) List_Init ( compiler->WordList ) 

#define Strncat( dst, src, n ) strncat ( (char *__restrict) dst, (const char *__restrict) src, (size_t) n )
#define Strlen( s ) ( s ? strlen ( (const char *) s ) : 0 )
#define Strncpy( dst, src, n ) strncpy ( (char *__restrict) dst, (const char *__restrict) src, (size_t) n )
//#define Sprintf( s, fmt, ...) sprintf ( (char *__restrict) s, (const char *__restrict) fmt, ... )
#define Map0( dllist, mf ) dllist_Map ( dllist, (MapFunction0) mf )
//#define SetStackPointerFromDsp()  if ( _CfrTil_ && _CfrTil_->DataStack ) _CfrTil_->DataStack->StackPointer = Dsp ;

