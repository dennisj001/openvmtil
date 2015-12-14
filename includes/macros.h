
#define Stack_Pop(stack) Stack_Pop_WithExceptionOnEmpty ( stack )

#define CompilerMemByteArray _Q_->CodeByteArray
#define _Compile_Int8( value ) ByteArray_AppendCopyItem ( CompilerMemByteArray, 1, value )
#define _Compile_Int16( value ) ByteArray_AppendCopyItem ( CompilerMemByteArray, 2, value )
#define _Compile_Int32( value ) ByteArray_AppendCopyItem ( CompilerMemByteArray, 4, value )
#define _Compile_Int64( value ) ByteArray_AppendCopyItem ( CompilerMemByteArray, 8, value )
#define _Compile_Cell( value ) ByteArray_AppendCopyItem ( CompilerMemByteArray, sizeof(int32), value )
#define Here ( _ByteArray_Here ( CompilerMemByteArray ) )
#define SetHere( address )  _ByteArray_SetHere_AndForDebug ( CompilerMemByteArray, address ) 
#define Set_CompilerSpace( byteArray ) (CompilerMemByteArray = (byteArray))
#define Get_CompilerSpace( ) CompilerMemByteArray

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
#define LocalsFrameSize( compiler ) ( compiler->LocalsFrameSize )
#define ParameterVarOffset( word ) ( - ( word->Index + 1 ) )
#define LocalVarOffset( word ) ( word->Index + 1 )

#define _GetState( aState, state ) ( (aState) & (state) ) 
#define GetState( obj, state ) _GetState( (obj)->State, state ) 
#define _SetState( state, newState, flag ) ( ( (flag) > 0 ) ? ( (state) |= (newState) ) : ( (state) &= ~ (newState) ) ) 
#define SetState( obj, state, flag ) _SetState ( ((obj)->State), (state), flag )
#define SetState_TrueFalse( obj, _true, _false )  ( ( (obj)->State |= (_true) ), ( (obj)->State &= ~ (_false) ) ) 
#define Debugger_SetState( debugger, state, flag ) SetState ( debugger, state, flag )
#define Debugger_GetState( debugger, state ) GetState( debugger, state ) 
#define Debugger_SetState_TrueFalse( debugger, _true, _false )  SetState_TrueFalse( debugger, _true, _false ) 
#define Debugger_IsStepping( debugger ) Debugger_GetState ( debugger, DBG_STEPPING )
#define Debugger_SetStepping( debugger, flag ) Debugger_SetState ( debugger, DBG_STEPPING, flag )  
#define Debugger_IsRestoreCpuState( debugger ) Debugger_GetState ( debugger, DBG_RESTORE_REGS )
#define Debugger_SetRestoreCpuState( debugger, flag ) Debugger_SetState ( debugger, DBG_RESTORE_REGS, flag ) 
#define Debugger_SetMenu( debugger, flag ) Debugger_SetState ( debugger, DBG_MENU, flag )
#define Debugger_IsDone( debugger ) Debugger_GetState ( debugger, DBG_DONE )
#define Debugger_SetDone( debugger, flag ) Debugger_SetState ( debugger, DBG_DONE, flag ) 
#define Debugger_IsNewLine( debugger ) Debugger_GetState ( debugger, DBG_NEWLINE )
#define Debugger_SetNewLine( debugger, flag ) Debugger_SetState ( debugger, DBG_NEWLINE, flag ) 

#define Compiler_SetState( compiler, state, flag ) ( SetState( compiler, state, flag ) )
#define Compiler_GetState( compiler, state ) ( GetState( compiler, state ) ) 
#define CompileMode ( GetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE ) || ( _Q_->OVT_LC && GetState ( _Q_->OVT_LC, ( LC_COMPILE_MODE ) ) ) )
#define Set_CompileMode( tf ) { SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, tf ) ; _Q_->OVT_LC && SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, tf ) ; }
#define Compiling CompileMode
#define ImmediateWord( word) (word->CType & IMMEDIATE)
#define CPrimitiveWord( word) (word->CType & CPRIMITIVE)

#define ReadLiner_SetState( readliner, state, flag ) SetState( readliner, state, flag ) 
#define ReadLiner_GetState( readliner, state ) GetState( readliner, state ) 
#define Interpreter_SetState( interpreter, state, flag ) SetState( interpreter, state, flag ) 
#define Interpreter_GetState( interpreter, state ) GetState( interpreter, state )  
#define Interpreter_SetState_TrueFalse( interpreter, _true, _false )  SetState_TrueFalse( interpreter, _true, _false ) 
#define PrintStateInfo_SetState( psi, state, flag ) SetState( psi, state, flag ) 
#define PrintStateInfo_GetState( psi, state ) GetState( psi, state ) 
#define Lexer_SetState( lexer, state, flag ) SetState ( lexer, state, flag ) 
#define Lexer_GetState( lexer, state ) GetState( lexer, state ) 
#define System_SetState( system, state, flag ) SetState( system, state, flag ) 
#define System_GetState( system, state ) GetState( system, state ) 
#define CfrTil_SetState( cfrtil, state, flag ) SetState( cfrtil, state, flag ) 
#define CfrTil_GetState( cfrtil, state ) GetState( cfrtil, state ) 
#define Verbose System_GetState( _Q_->OVT_Context->System0, VERBOSE)
#define System_SetStateTF( system, _true, _false )  SetState_TrueFalse ( Object, _true, _false ) 

#define Stack_N( stack, offset ) ((stack)->StackPointer [ (offset) ] )
#define Stack_OffsetValue( stack, offset ) ((stack)->StackPointer [ (offset) ] )
#define Compiler_WordStack( compiler, n ) ((Word*) (Stack_OffsetValue ( (compiler)->WordStack, (n))))
#define WordStack( n ) ((Word*) Compiler_WordStack( _Q_->OVT_Context->Compiler0, (n) ))
#define CompilerWordStack _Q_->OVT_Context->Compiler0->WordStack
#define CompilerLastWord WordStack( 0 )
#define WordsBack( n ) WordStack( (-n) )
#define IncrementCurrentAccumulatedOffset( increment ) \
        {\
            if ( _Q_->OVT_Context->Compiler0->AccumulatedOffsetPointer )\
            {\
                ( *( int32* ) (_Q_->OVT_Context->Compiler0->AccumulatedOffsetPointer) ) += (increment) ;\
            }\
            if ( _Q_->OVT_Context->Compiler0->AccumulatedOptimizeOffsetPointer )\
            {\
                ( *( int32* ) (_Q_->OVT_Context->Compiler0->AccumulatedOptimizeOffsetPointer) ) += (increment) ;\
            }\
        }

#define SetCurrentAccumulatedOffset( value ) \
        {\
            if ( _Q_->OVT_Context->Compiler0->AccumulatedOffsetPointer )\
            {\
                ( *( int32* ) (_Q_->OVT_Context->Compiler0->AccumulatedOffsetPointer) ) = (value) ;\
            }\
            if ( _Q_->OVT_Context->Compiler0->AccumulatedOptimizeOffsetPointer )\
            {\
                ( *( int32* ) (_Q_->OVT_Context->Compiler0->AccumulatedOptimizeOffsetPointer) ) = (value) ;\
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
#define AllowNewlines if (_Q_) PrintStateInfo_SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, false ) 
#define ConserveNewlines if (_Q_) PrintStateInfo_SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) 
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
#define _String_Show2( buf, fg, bg ) sprintf ( buf, "%c[%d;%dm", ESC, fg, bg )
#define _String_ShowColors( buf, fg, bg ) _String_Show2 ( buf, fg + 30, bg + 40 )
#define DefaultColors Ovt_DefaultColors () 
#define AlertColors Ovt_AlertColors () 
#define DebugColors Ovt_DebugColors () 
#define NoticeColors Ovt_NoticeColors () 

// Change Colors
#define cc( s, c ) _String_InsertColors ( (char*) ( (char*) s ? (char*) s : "" ), (c) ) 
#define c_ud( s ) cc ( (char*) s, (_Q_->Current == &_Q_->User) ? &_Q_->Default : &_Q_->User ) 
#define c_ad( s ) cc ( (char*) s, (_Q_->Current == &_Q_->Alert) ? &_Q_->Default : &_Q_->Alert ) 
#define c_dd( s ) cc ( (char*) s, (_Q_->Current == &_Q_->Debug) ? &_Q_->Default : &_Q_->Debug ) 

#define _DataStack_ _Q_->OVT_CfrTil->DataStack
#define _AtCommandLine() ( ! _Q_->OVT_Context->System0->IncludeFileStackNumber ) 
#define AtCommandLine( rl ) \
        ( Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_COMMAND_LINE ) || \
        ( ReadLiner_GetState ( rl, CHAR_ECHO ) && ( ! _Q_->OVT_Context->System0->IncludeFileStackNumber ) ) ) // ?? essentially : at a command line ??
#define SessionString_New( string ) String_New ( string, SESSION ) 
#define TemporaryString_New( string ) String_New ( string, TEMPORARY ) 
#define IsWordRecursive CfrTil_CheckForGotoPoints ( GI_RECURSE )
#define AppendCharToSourceCode( c ) //_Lexer_AppendCharToSourceCode ( lexer, c ) 
#define ReadLine_Nl (ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) == '\n')
#define ReadLine_Eof (ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) == eof)
#define ReadLine_ClearLineQuick _Q_->OVT_Context->ReadLiner0->InputLine [ 0 ] = 0 
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

#define Pause OpenVmTil_Pause () 
#define Pause_1( msg ) AlertColors; Printf ( (byte*)"\n%s", msg ) ; OpenVmTil_Pause () ;
#define Pause_2( msg, arg ) AlertColors; Printf ( (byte*)msg, arg ) ; OpenVmTil_Pause () ;

#define Error_Abort( msg ) Throw ( (byte*) msg, ABORT )
#define Error( msg, state ) { AlertColors; if ((state) & PAUSE ) Pause ; if ((state) >= QUIT ) Throw ( (byte*) msg, state ) ; else Printf ( (byte*)"\n%s", (byte*) msg, state ) ; }
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

#define LispAllocType LISP_TEMP
#define LO_IsQuoted( l0 ) (( l0->State & QUOTED ) || ( ( l0->State & QUASIQUOTED ) && (! ( l0->State & (UNQUOTED|UNQUOTE_SPLICE) ) ) ) ) //( ! ( l0->State & ( QUOTED | QUASIQUOTED ) )  || (l1->State & UNQUOTED) ) )
#define LO_Last( lo ) (ListObject*) DLList_Last ( (DLList*) lo->Lo_List )
//#define LO_First( lo ) (ListObject*) DLList_First ( (DLList*) lo->Lo_List )
//#define LO_Remove( lo ) DLNode_Remove ( (DLNode *) lo )
#define LO_Previous( lo ) ( ListObject* ) DLNode_Previous ( ( DLNode* ) lo )
#define LO_Next( lo ) ( ListObject* ) DLNode_Next ( ( DLNode* ) lo )
#define LO_AddToTail( lo, lo1 ) DLList_AddNodeToTail ( lo->Lo_List, ( DLNode* ) (lo1) ) 
#define LO_AddToHead( lo, lo1 ) DLList_AddNodeToHead ( lo->Lo_List, ( DLNode* ) (lo1) ) 
#define LO_New( lType, object ) (ListObject *) _LO_New ( lType, 0, (byte*) object, 0, 0, 0, LispAllocType )
#define LambdaArgs( proc ) proc->p[0]
#define LambdaProcedureBody( proc ) proc->p[1]
#define LambdaVals( proc ) proc->p[2]
#define LO_ReplaceNode( node, anode) DLNode_Replace ( (DLNode *) node, (DLNode *) anode ) 
#define LO_PrintWithValue( l0 ) Printf ( (byte*) "%s", _LO_Print ( (ListObject *) l0 , 0, 0, 1 ) ) 
#define _LO_PRINT(l0) _LO_Print ( ( ListObject * ) l0, 0, 0, 0 )
#define _LO_PRINT_WITH_VALUE(l0) _LO_Print ( ( ListObject * ) l0, 0, 0, 1 )
#define LC_Print( l0 ) LO_PrintWithValue ( l0 ) 
#define LO_CopyTemp( l0 ) _LO_Copy ( l0, LispAllocType )
#define LO_Copy( l0 ) _LO_Copy ( l0, LISP )
#define LO_CopyOne( l0 ) _LO_AllocCopyOne ( l0, LispAllocType )
#define LO_Eval( l0 ) _LO_Eval ( l0, 0, 1 )
#define nil (_Q_->OVT_LC ? _Q_->OVT_LC->Nil : 0)
//#define SaveStackPointer() Dsp
#define LC_SaveStackPointer( lc ) { if ( lc ) lc->SaveStackPointer = (int32*) Dsp ; }
#define LC_RestoreStackPointer( lc ) _LC_ResetStack ( lc ) //{ if ( lc && lc->SaveStackPointer ) Dsp = lc->SaveStackPointer ; }

#define String_Equal( string1, string2 ) (strcmp ( (char*) string1, (char*) string2 ) == 0 )
#define String_CB( string0 ) String_ConvertToBackSlash ( string0 )

#define DEBUG_PRINTSTACK if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE )  ) CfrTil_PrintDataStack () ;
#define TypeNamespace_Get( object ) (object->TypeNamespace ? object->TypeNamespace : object->ContainingNamespace)
#define _Lexer_IsCharDelimiter( lexer, c ) lexer->DelimiterCharSet [ c ]
#define _Lexer_IsCharDelimiterOrDot( lexer, c ) lexer->DelimiterOrDotCharSet [ c ]

#if 1
#define NAMESPACE_TYPE ( NAMESPACE | DOBJECT | CLASS  )
#define NAMESPACE_RELATED_TYPE ( NAMESPACE | DOBJECT | CLASS | OBJECT_FIELD | C_TYPE | C_CLASS | CLASS_CLONE )
//#define OBJECT_TYPES ( OBJECT | DOBJECT | THIS | VARIABLE | LOCAL_VARIABLE | PARAMETER_VARIABLE | OBJECT_FIELD | CONSTANT | C_TYPE | C_CLASS | CLASS_CLONE )
#define OBJECT_TYPE ( CONSTANT | VARIABLE | LOCAL_VARIABLE | OBJECT | DOBJECT | PARAMETER_VARIABLE )
#define NON_MORPHISM_TYPE ( OBJECT_TYPE | NAMESPACE_RELATED_TYPE )
// #define NON_MORPHISM_TYPE ( CONSTANT | VARIABLE | LOCAL_VARIABLE | NAMESPACE | CLASS | OBJECT_FIELD | OBJECT | DOBJECT | C_TYPE | C_CLASS | CLASS_CLONE | PARAMETER_VARIABLE )
#else
#define NAMESPACE_TYPES ( NAMESPACE | DOBJECT | OBJECT | CLASS  )
#define OBJECT_TYPES ( DOBJECT | OBJECT )
#endif

#define DEBUG_INIT \
        Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;\
        int32 dm = 0 ;\
        if ( debugger ) dm = GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) && ( ! GetState ( debugger, ( DBG_DONE | DBG_STEPPING | DBG_SKIP_INNER_SHOW ) ) ) ;\
        ;
#define _DEBUG_PRE if ( dm ) _Debugger_PreSetup ( debugger, token, word ) ;
#define DEBUG_PRE if ( dm && (! GetState ( debugger, DBG_DONE ))) _Debugger_PreSetup ( debugger, token, word ) ;
#define DEBUG_FINISH if ( dm ) _Debugger_PostShow ( debugger, token, word ) ;
#define DEBUG_POST DEBUG_FINISH
#define DEBUG_SHOW DEBUG_FINISH
#define DEBUG_START DEBUG_INIT DEBUG_PRE
#define Debugger_WrapBlock( token, word, block ) DEBUG_INIT DEBUG_PRE { block } DEBUG_FINISH ;

#define Is_NamespaceType( w ) ( w ? ( ( Namespace* ) w )->CType & NAMESPACE_TYPE : 0 )
#define Is_ValueType( w ) ( w ? ( ( Namespace* ) w )->CType & NON_MORPHISM_TYPE : 0 )
#define String_Init( s ) s[0]=0 ; 

// memory allocation
#define _Object_Allocate( size, allocType ) Mem_Allocate ( size, allocType ) 
#define Object_Allocate( type, slots, allocType ) (type *) _Object_Allocate ( (sizeof ( type ) + ((slots -1) * CELL)), allocType ) 
#define mmap_AllocMem( size ) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
#define _Allocate( size, nba ) _ByteArray_AppendSpace ( nba->ba_CurrentByteArray, size ) 

#define Get_NBA_Symbol_To_NBA( s )  ( NamedByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data ) 
#define Get_NBA_Node_To_NBA( node )  ( NamedByteArray* ) ( ( ( Symbol* ) node )->S_pb_Data ) 
#define Get_BA_Symbol_To_BA( s )  ( ByteArray* ) ( ( ( Symbol* ) s )->S_pb_Data ) 
#define Set_NBA_Symbol_To_NBA( nba )  nba->NBA_Symbol.S_pb_Data = ( byte* ) nba
#define Set_BA_Symbol_To_BA( ba )  ba->BA_Symbol.S_pb_Data = ( byte* ) ba
#define MemCheck( block ) { _Calculate_CurrentNbaMemoryAllocationInfo ( 1 ) ; block ; _Calculate_CurrentNbaMemoryAllocationInfo ( 1 ) ; }

#define DebugOn (GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ))
#define DebugLevel( n ) (DebugOn && ( _Q_->Verbosity >= ( n ) ) ) 

#define _IsLValue() _Interpret_CheckEqualBeforeSemi_LValue ()
#define IsLValue( word ) Interpret_CheckEqualBeforeSemi_LValue ( word )

