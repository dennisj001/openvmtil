#include "../../include/cfrtil32.h"

// Intel notes - cf. InstructionSet-A-M-253666.pdf - section 2.1; InstructionSet-N-Z-253667.pdf section B.1/B.2 :
// b prefix = binary code
// -----------------------------------------------------------------------
// instuction format ( in bytes )
// prefixes  opcode  modRm     sib     disp   immediate
//  0 - 4    1 - 3   0 - 1    0 - 1    0,1,4    0,1,4      -- number of bytes
// optional          ------------optional--------------
// -----------------------------------------------------------------------
//   modRm byte ( bits ) :: mod 0 : no disp ;; mod 1 : 1 byte disp : mod 2 : 4 byte disp ;; mod 3 : reg value :: sections 2.1.3/2.1.5, Table 2-2
//   the mod field is a semantic function on the r/m field determining its meaning either as the reg value itself or the value at the [reg] as an addr + offset
//   Intel InstructionSet-N-Z-253667.pdf section 2.1.5
//    mod     reg     r/m  
//   7 - 6   5 - 3   2 - 0 
//    0-3              4 - b100 => sib, instead of reg ESP   : mod bit determines size of displacement 
// -----------------------------------------------------------------------
//  reg/rm codes :
//  EAX 0, ECX 1, EDX 2, EBX 3, ESP 4, EBP 5, ESI 6, EDI 7
// -----------------------------------------------------------------------
//  bit positions encoding :  ...|7|6|5|4|3|2|1|0|  but nb. intel is little endian
// -----------------------------------------------------------------------
//  opCode direction bit 'd' is bit position 1 : 1 => rm/sib to reg ; 0 => reg to rm/sib -- for some instructions
//  sign extend bit 's' is bit position 1 for some instructions
//  operand size bit 'w' is bit position 0 for some instructions
// -----------------------------------------------------------------------
//       sib byte ( bits ) with rm 4 - b100 - ESP
//    scale  index   base
//    7 - 6  5 - 3  2 - 0
//    scale 0 : [index * 1 + base]
//    scale 1 : [index * 2 + base]
//    scale 2 : [index * 4 + base]
//    scale 1 : [index * 4 + base]
// -----------------------------------------------------------------------
// intel syntax : opcode dst, src
// att syntax   : opcode src, dst

// note : x86-32 instruction format : || prefixes : 0-4 bytes | opCode : 1-3 bytes | mod : 0 - 1 byte | sib : 0 - 1 byte | disp : 0-4 bytes | immediate : 0-4 bytes ||
// note : intex syntax  : instruction dst, src - cfrTil uses this order convention
//        att   syntax  : instruction src, dst
// note : rm : reg memory - the register which contains the memory address in mod instructions

// cfrTil uses intel syntax convention

// ----------------------------------
// | intel addressing ideas summary |
// ----------------------------------
// remember : the intel cpus can not reference to memory operands in one instruction so the modr/m byte selects with the mod and rm field an operand to use
// with the reg field value (generally)
// the mod field ( 2 bits ) contols whether the r/m field reg refers to a direct reg or indirect + disp values (disp values are in the displacement field)
// mod 0 is for register indirect -- no displacement the register is interpreted as an address; it refers to a value in a memory address with no disp
// mod 1 is for register indirect -- 8 bit disp the register is interpreted as an address; it refers to a value in a memory address with 8 bit disp
// mod 2 is for register indirect -- 32 bit disp the register is interpreted as an address; it refers to a value in a memory address with 32 bit disp
// mod 3 is for register direct   -- using the direct register value -- not as an address 
// the reg field of the modr/m byte generally refers to to register to use with the mod modified r/m field -- intel can't address two memory fields in any instruction
// --------------------------------------

int32
_CalculateModRmByte ( int32 mod, int32 reg, int32 rm, int32 disp, int32 sib )
{
    // mod reg r/m bits :
    //  00 000 000
    int32 modRm ;
    if ( mod != 3 )
    {
        if ( disp == 0 )
            mod = 0 ;
        else if ( disp <= 0xff )
            mod = 1 ;
            //else if ( disp >= 0x100 ) mod = 2 ;
        else
            mod = 2 ;
    }
    if ( ( mod < 3 ) && ( rm == 4 ) ) //|| ( ( rm == 5 ) && ( disp == 0 ) ) ) )
        //if ( ( mod < 3 ) && ( ( ( rm == 4 ) && ( sib == 0 ) ) || ( ( rm == 5 ) && ( disp == 0 ) ) ) )
    {
        // cf. InstructionSet-A-M-253666.pdf Table 2-2
        CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ;
    }
    if ( sib )
    {
        rm = 4 ; // from intel mod tables
        reg = 0 ;
    }
    return modRm = ( mod << 6 ) + ( ( reg & 0x7 ) << 3 ) + ( rm & 0x7 ) ; // only use 3 bits of reg/rm
}

//-----------------------------------------------------------------------
//   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
//    mod     reg      rm
//   7 - 6   5 - 3   2 - 0
//-----------------------------------------------------------------------
//  reg/rm values :
//  EAX 0, ECX 1, EDX 2, ECX 3, ESP 4, EBP 5, ESI 6, EDI 7
//-----------------------------------------------------------------------
// some checks of the internal consistency of the instruction bits

void
_Compile_Displacement ( int32 modRm, int32 disp )
{
    int32 mod = modRm & 0xc0 ; // 1100 0000
    int32 reg = modRm & 0x38 ; // 0011 1000
    if ( ( ( ( mod == 0 ) && ( reg != ESP ) && disp ) ) || ( ( mod == 1 ) && ( disp >= 0x100 ) )
        || ( ( mod == 2 ) && ( disp <= 0xff ) ) )
    {
        CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ;
    }
    else if ( disp )
    {
        if ( disp >= 0x100 )
            _Compile_Int32 ( disp ) ;
        else
            _Compile_Int8 ( ( byte ) disp ) ;
    }
}

int32
_CalculateSib ( int32 scale, int32 indexReg, int32 baseReg )
{
    //  scale index base bits  : scale 1 = *2, 2 = *4, 3 = *8 ; index and base refer to registers
    //  00    000   000
    int32 sib = ( scale << 6 ) + ( indexReg << 3 ) + baseReg ;
    return sib ;
}

#if ABI == 64

byte
_CalculateRex ( int32 reg, int32 rm, int32 sib, int32 operandSize )
{
    //  0100    WRXB
    byte rex = 0x40 ;
    if ( reg > 0x7 ) rex += 4 ; // (1 << 2) ;
    if ( rm > 0x7 ) rex += 1 ;
    if ( sib > 0x7 ) rex += 2 ; // 1 << 1 ;
    if ( operandSize > BYTE ) rex += 8 ; // 1 << 3 ;
    return rex ;
}
#endif

void
_Compile_ModRmSibDisplacement ( int32 modRm, int32 modRmFlag, int32 sib, int disp )
{
    if ( modRmFlag )
        _Compile_Int8 ( modRm ) ;
    if ( sib )
        _Compile_Int8 ( sib ) ; // sib = sib_modFlag if sib_modFlag > 1
    if ( modRmFlag )
    {
        if ( disp )
            _Compile_Displacement ( modRm, disp ) ;
    }
    else if ( disp )
        _Compile_Int32 ( disp ) ;
}
// instruction letter codes : I - immediate data ; 32 : 32 bit , 8 : 8 bit ; EAX, DSP : registers
// we could have a mod of 0 so the modFlag is necessary
// operandSize : specific size of immediate data - BYTE or WORD
// SIB : scale, index, base addressing byte

void
_Compile_ImmediateData ( int32 imm, int32 immSize )
{
#if 1 // the opcode probably is all that needs to be adjusted for this to not be necessary    
    // to not compile an imm when imm is a parameter, set isize == 0 and imm == 0
    if ( immSize > 0 )
    {
        if ( immSize == BYTE )
            _Compile_Int8 ( ( byte ) imm ) ;
        else if ( immSize == CELL )
            _Compile_Cell ( imm ) ;
    }
    else // with operandSize == 0 let the compiler use the minimal size ; nb. can't be imm == 0
#endif        
    {
        if ( imm >= 0x100 )
            _Compile_Int32 ( imm ) ;
        else if ( imm )
            _Compile_Int8 ( ( byte ) imm ) ;
    }
}

// Intel - InstructionSet-A-M-253666.pdf - section 2.1 :
//-----------------------------------------------------------------------
// instuction format ( number of bytes )
// prefixes  opcode  modRm   sib       disp    immediate
//  0 - 4    1 - 3   0 - 1  0 - 1    0,1,2,4    0,1,2,4      -- number of bytes
//-----------------------------------------------------------------------
//   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
//    mod     reg      rm
//   7 - 6   5 - 3   2 - 0 
//-----------------------------------------------------------------------
//  reg/rm values :
//  EAX 0, ECX 1, EDX 2, ECX 3, ESP 4, EBP 5, ESI 6, EDI 7
//-----------------------------------------------------------------------
//       sib byte ( bits )
//    scale  index   base
//    7 - 6  5 - 3  2 - 0
//-----------------------------------------------------------------------

void
Compile_StartOpCode_Int8 ( int opCode )
{
    _Compile_Int8 ( ( byte ) opCode ) ;
}

void
_Compile_InstructionX86 ( int opCode, int mod, int reg, int rm, int modFlag, int sib, int32 disp,
    int32 imm, int immSize )
{
    D0 ( byte *here = Here ) ;
    Compile_StartOpCode_Int8 ( ( byte ) opCode ) ;
    int32 modRm = _CalculateModRmByte ( mod, reg, rm, disp, sib ) ;
    _Compile_ModRmSibDisplacement ( modRm, modFlag, sib, disp ) ;
    _Compile_ImmediateData ( imm, immSize ) ;
    PeepHole_Optimize ( ) ;
    D0 ( if ( _Debugger_ ) Debugger_UdisOneInstruction ( _Debugger_, here, ( byte* ) "", ( byte* ) "" ) ; )
    }

// load reg with effective address of [ mod rm sib disp ]

void
_Compile_LEA ( int32 reg, int32 rm, int32 sib, int32 disp )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0x8d, MEM, reg, rm, 1, sib, disp, 0, 0 ) ;
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND SUB XOR CMP : but not with immediate data
// s and w bits of the x86 opCode : w seems to refer to word and is still used probably for historic and traditional reasons
// note : the opReg - operand register parameter is always used for the rm field of the resulting machine code
// These are all operating on a memory operand
// for use of immediate data with this group use _Compile_Group1_Immediate

void
_Compile_X_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
{
    int32 opCode = code << 3 ;
    if ( osize > BYTE ) opCode |= 1 ;
    if ( toRegOrMem == REG ) opCode |= 2 ;
    // we need to be able to set the size so we can know how big the instruction will be in eg. CompileVariable
    // otherwise it could be optimally deduced but let caller control by keeping operandSize parameter
    // some times we need cell_t where bytes would work
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( opCode, mod, reg, rm, 1, sib, disp, 0, 0 ) ;
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND_OPCODE SUB XOR CMP : with immediate data
// this is for immediate operands operating on REG direction
// mod := REG | MEM
// rm is operand register
// ?!? shouldn't we just combine this with _Compile_Group1 (above) ?!?

void
_Compile_X_Group1_Immediate ( int32 code, int32 mod, int32 rm, int32 disp, int32 imm, int32 iSize )
{
    // 0x80 is the base opCode for this group of instructions but 0x80 is an alias for 0x82
    // we always sign extend so opCodes 0x80 and 0x82 are not being used
    // #x80 is the base opCode for this group of instructions 
    // 1000 00sw 
    int32 opCode = 0x80 ;
    if ( ( iSize > BYTE ) || ( imm > 0xff ) )
    {
        opCode |= 1 ;
        //iSize = CELL ;
    }
    else if ( ( iSize <= BYTE ) || ( imm < 0x100 ) ) opCode |= 3 ;
    // we need to be able to set the size so we can know how big the instruction will be in eg. CompileVariable
    // otherwise it could be optimally deduced but let caller control by keeping operandSize parameter
    // some times we need cell_t where bytes would work
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( opCode, mod, code, rm, 1, 0, disp, imm, iSize ) ;
}

void
_Compile_Op_Group1_Reg_To_Reg ( int32 code, int32 dstReg, int32 srcReg )
{
    _Compile_X_Group1 ( code, 2, REG, srcReg, dstReg, 0, 0, CELL ) ;
}

// opCode group 1 - 0x80-0x83 : ADD OR ADC SBB AND_OPCODE SUB XOR CMP
// to reg ( toRm == 0 ) is default
// toRm flag is like a mod field but is coded as part of opCode
// toRm flag = 0 => ( dst is reg, src is rm ) is default - reg  - like mod 3
// toRm flag = 1 => ( dst is rm, src is reg )             [reg] - like mod 0 // check this ??


// TEST XCHG

void
_Compile_Op_Special_Reg_To_Reg ( int32 code, int32 rm, int32 reg ) // toRm = 0 => ( dst is reg, src is rm ) is default
{
    int32 opCode ;
    if ( code == TEST_R_TO_R )
        opCode = 0x85 ;
    else if ( code == XCHG_R_TO_R )
        opCode = 0x87 ;
    else
        CfrTil_Exception ( MACHINE_CODE_ERROR, ABORT ) ;
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( opCode, 3, reg, rm, 1, 0, 0, 0, 0 ) ;
}

// Group2 : 0pcodes C0-C3/D0-D3
// ROL RLR RCL RCR SHL SHR SAL SAR
// mod := REG | MEM

void
_Compile_Group2 ( int mod, int regOpCode, int rm, int sib, int32 disp, int32 imm )
{
    //cell opCode = 0xc1 ; // rm32 imm8
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xc1, mod, regOpCode, rm, 1, sib, disp, imm, BYTE ) ;
}

void
_Compile_Group2_CL ( int mod, int regOpCode, int rm, int sib, int32 disp )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xd3, mod, regOpCode, rm, 1, sib, disp, 0, 0 ) ;
}

// some Group 3 code is UNTESTED
// opCodes TEST NOT NEG MUL DIV IMUL IDIV // group 3 - F6-F7
// s and w bits of the x86 opCode : w seems to refer to word and is still used probably for historic and traditional reasons
// note : the opReg - operand register parameter is always used for the rm field of the resulting machine code
// operating with either a direct register, or indirect memory operand on a immediate operand
// mod := RegOrMem - direction is REG or MEM
// 'size' is operand size

void
_Compile_Group3 ( int32 code, int32 mod, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xf7, mod, code, rm, 1, sib, disp, imm, size ) ;
}

// mul reg with imm to rm

void
_Compile_IMULI ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
{
    int32 opCode = 0x69 ;
    if ( imm < 256 )
    {
        opCode |= 2 ;
        size = 0 ;
    }
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( opCode, mod, reg, rm, 1, sib, disp, imm, size ) ; //size ) ;
}

void
_Compile_IMUL ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp )
{
    Compile_StartOpCode_Int8 ( 0x0f ) ;
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xaf, mod, reg, rm, 1, sib, disp, 0, 0 ) ;
}

void
_Compile_Test ( int32 mod, int32 reg, int32 rm, int32 disp, int32 imm )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xf7, mod, reg, rm, 1, 0, disp, imm, CELL ) ; //??
}

// inc/dec/push/pop

void
_Compile_Group5 ( int32 code, int32 mod, int32 rm, int32 sib, int32 disp, int32 size )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xff, mod, code, rm, 1, sib, disp, 0, size ) ;
}

// inc/dec/push/pop reg
#if 0

void
_Compile_IncDecPushPopReg ( int32 op, int32 reg )
{
    byte opCode ;
    if ( op == INC )
        opCode = 0x40 ;
    else if ( op == DEC )
        opCode = 0x48 ;
    if ( op == PUSH )
        opCode = 0x40 ;
    else if ( op == POP )
        opCode = 0x48 ;
    opCode += reg ;
    Compile_StartOpCode_Int8 ( opCode ) ;
}
#endif

// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

void
_Compile_Move_Reg_To_Reg ( int32 dstReg, int32 srcReg )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0x8b, 3, dstReg, srcReg, 1, 0, 0, 0, 0 ) ;
}

// direction : MEM or REG
// reg : is address in case of MEMORY else it is the register (reg) value
#if 1

void
_Compile_MoveImm ( int32 direction, int32 rm, int32 sib, int32 disp, int32 imm, int32 operandSize )
{
    int32 opCode = 0xc6, mod ;
    if ( operandSize > BYTE ) opCode |= 1 ;
    if ( direction == REG ) mod = 3 ;
    else
    {
        if ( disp == 0 ) mod = 0 ;
        else if ( disp < 0x100 ) mod = 1 ;
        else mod = 2 ;
    }
    _Compile_InstructionX86 ( opCode, mod, 0, rm, 1, sib, disp, imm, operandSize ) ;
}
#else

void
_Compile_MoveImm ( int32 direction, int32 rm, int32 sib, int32 disp, int32 imm, int32 operandSize )
{
    int32 opCode = 0xc6, mod ;
    if ( ( operandSize > BYTE ) || ( imm >= 0x100 ) ) opCode |= 1 ;
    //if ( ( imm >= 0x100 ) ) opCode |= 1 ;
    if ( direction == REG ) mod = 3 ;
    else
    {
        if ( disp == 0 ) mod = 0 ;
        else if ( disp < 0x100 ) mod = 1 ;
        else mod = 2 ;
    }
    _Compile_InstructionX86 ( opCode, mod, 0, rm, 1, sib, disp, imm, operandSize ) ;
}
#endif

void
_Compile_MoveImm_To_Reg ( int32 reg, int32 imm, int32 iSize )
{
    _Compile_MoveImm ( REG, reg, 0, 0, imm, iSize ) ;
}

// _Compile_MoveImm_To_Mem ( int32 reg, int32 imm, int32 iSize )

void
_Compile_MoveImm_To_Mem ( int32 reg, int32 imm, int32 iSize )
{
    _Compile_MoveImm ( MEM, reg, 0, 0, imm, iSize ) ;
}

void
_Compile_MoveMem_To_Reg ( int32 reg, byte * address, int32 thruReg, int32 iSize )
{
    _Compile_MoveImm_To_Reg ( thruReg, ( int32 ) address, iSize ) ;
    _Compile_Move_Rm_To_Reg ( reg, thruReg, 0 ) ;
}

void
_Compile_MoveMem_To_Reg_NoThru ( int32 reg, byte * address, int32 iSize )
{
    _Compile_MoveImm_To_Reg ( reg, ( int32 ) address, iSize ) ;
    _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ;
}

void
_Compile_MoveReg_To_Mem ( int32 reg, byte * address, int32 thruReg, int32 iSize )
{
    _Compile_MoveImm_To_Reg ( thruReg, ( int32 ) address, iSize ) ;
    _Compile_Move_Reg_To_Rm ( thruReg, reg, 0 ) ;
}

// the basic move instruction
// mov reg to mem or mem to reg ( but not reg to reg or move immediate )
// note this function uses the bit order of operands in the mod byte : (mod) reg r/m
// not the intel syntax as with _CompileMoveRegToMem _CompileMoveMemToReg
// the 'rmReg' parameter must always refer to a memory location; 'reg' refers to the register, either to or from which we move mem

void
_Compile_Move ( int32 direction, int32 reg, int32 rm, int32 sib, int32 disp )
{
    int32 opCode, mod = direction, size = 0 ;
    opCode = 0x89 ;
    if ( mod == REG ) opCode |= 2 ; // 0x8b ; // 0x89 |= 2 ; // d : direction bit = 'bit 1'

    // note : mod is never 3 here - this is not! move REG to REG; see _CompileMoveRegToReg
    //else
    {
        if ( ! disp )
            mod = 0 ;
        else if ( disp <= 0xff )
            mod = 1 ;
        else if ( disp >= 0x100 )
            mod = 2 ;
        else
        {
            CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ; // note : mod is never 3 here - this is not! move REG to REG; see _CompileMoveRegToReg
            //mod = 3 ; // does this work - not in intel documentation but gcc uses it
            //if ( disp != 0 ) 
        }
    }
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( opCode, mod, reg, rm, 1, sib, disp, 0, size ) ;
}

// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

void
_Compile_Move_Reg_To_Rm ( int32 dstRmReg, int32 srcReg, int32 disp )
{
    _Compile_Move ( MEM, srcReg, dstRmReg, 0, disp ) ;
}

void
_Compile_Move_AddressValue_To_EAX ( int32 address )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xa1, 0, 0, 0, 0, 0, address, 0, 0 ) ;
}

void
_Compile_Move_EAX_To_MemoryAddress ( int32 address )
{
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( 0xa3, 0, 0, 0, 0, 0, address, 0, 0 ) ;
}

// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src

void
_Compile_Move_Rm_To_Reg ( int32 dstReg, int32 srcRmReg, int32 disp )
{
    _Compile_Move ( REG, dstReg, srcRmReg, 0, disp ) ;
}

#if 0 // NEW

void
_Compile_Move_FromAtMem_ToMem ( int32 dstAddress, int32 srcAddress ) // thruReg == EAX
{
    _Compile_Move_AddressValue_To_EAX ( srcAddress ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    _Compile_Move_EAX_To_MemoryAddress ( dstAddress ) ;
}
#endif

byte *
Calculate_Address_FromOffset_ForCallOrJump ( byte * address )
{
    byte * iaddress = 0 ;
    int32 offset ;
    if ( ( * address == JMPI32 ) || ( * address == CALLI32 ) )
    {
        offset = * ( ( int32 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + CELL ;
    }
    else if ( ( ( * address == 0x0f ) && ( ( * ( address + 1 ) >> 4 ) == 0x8 ) ) )
    {
        offset = * ( ( int32 * ) ( address + 2 ) ) ;
        iaddress = address + offset + 2 + CELL ;
    }
    return iaddress ;
}


// compileAtAddress is the address of the offset to be compiled
// for compileAtAddress of the disp : where the space has *already* been allocated
// call 32BitOffset ; <- intel call instruction format
// endOfCallingInstructionAddress + disp = jmpToAddr
// endOfCallingInstructionAddress = compileAtAddress + 4 ; for ! 32 bit disp only !
// 		-> disp = jmpToAddr - compileAtAddress - 4

int32
_CalculateOffsetForCallOrJump ( byte * compileAtAddress, byte * jmpToAddr )
{
    int32 offset ;
    offset = ( jmpToAddr - compileAtAddress - sizeof (int32 ) ) ; // we have to go back the instruction size to get to the start of the insn 
    return offset ;
}

void
_SetOffsetForCallOrJump ( byte * compileAtAddress, byte * jmpToAddr )
{
    int32 offset = _CalculateOffsetForCallOrJump ( compileAtAddress, jmpToAddr ) ;
    * ( ( int32* ) compileAtAddress ) = offset ;
}

void
_Compile_JumpToAddress ( byte * jmpToAddr ) // runtime
{
#if 1
    if ( jmpToAddr != ( Here + 5 ) ) // optimization : don't need to jump to the next instruction
    {
        int imm = _CalculateOffsetForCallOrJump ( Here + 1, jmpToAddr ) ;
        // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
        _Compile_InstructionX86 ( 0xe9, 0, 0, 0, 0, 0, 0, imm, INT ) ; // with jmp instruction : disp is compiled an immediate offset
    }
#else
    _Compile_MoveImm_To_EAX ( jumpToAddr ) ;
    _Compile_Group5 ( JMP, 0, 0, 0, 0 ) ;
#endif
}

void
_Compile_JumpToReg ( int32 reg ) // runtime
{
    _Compile_Group5 ( JMP, 0, reg, 0, 0, 0 ) ;
}

void
_Compile_UninitializedJumpEqualZero ( )
{
    Compile_JCC ( NZ, ZERO_TTT, 0 ) ;
}

void
_Compile_JumpWithOffset ( int32 disp ) // runtime
{
    Compile_StartOpCode_Int8 ( JMPI32 ) ;
    _Compile_Cell ( disp ) ;
}

void
_Compile_UninitializedCall ( ) // runtime
{
    Compile_StartOpCode_Int8 ( CALLI32 ) ;
    _Compile_Cell ( 0 ) ;
}

void
_Compile_UninitializedJump ( ) // runtime
{
    Compile_StartOpCode_Int8 ( JMPI32 ) ;
    _Compile_Cell ( 0 ) ;
}

// JE, JNE, ... see machineCode.h

void
_Compile_JCC ( int32 negFlag, int32 ttt, uint32 disp )
{
    Compile_StartOpCode_Int8 ( 0xf ) ; // little endian ordering
    _Compile_Int8 ( 0x8 << 4 | ttt << 1 | negFlag ) ; // little endian ordering
    _Compile_Int32 ( disp ) ;
}

void
Compile_JCC ( int32 negFlag, int32 ttt, byte * jmpToAddr )
{
    unsigned int disp ;
    if ( jmpToAddr )
    {
        disp = _CalculateOffsetForCallOrJump ( Here + 1, jmpToAddr ) ;
    }
    else disp = 0 ; // allow this function to be used to have a delayed compile of the actual address
    _Compile_JCC ( negFlag, ttt, disp ) ;
}

void
_Compile_Call ( int32 callAddr )
{
    _Compile_InstructionX86 ( CALLI32, 0, 0, 0, 0, 0, 0, callAddr, INT_T ) ;
}

void
Compile_Call ( byte * callAddr )
{
    int32 imm = _CalculateOffsetForCallOrJump ( Here + 1, callAddr ) ;
    _Compile_Call ( imm ) ;
}

void
_Compile_Call_NoOptimize ( byte * callAddr )
{
    int32 imm = _CalculateOffsetForCallOrJump ( Here + 1, callAddr ) ;
    // _Compile_InstructionX86 ( opCode, mod, reg, rm, modFlag, sib, disp, imm, immSize )
    _Compile_InstructionX86 ( CALLI32, 0, 0, 0, 0, 0, 0, imm, INT_T ) ;
    // push rstack here + 5
    // _Compile_MoveImm_To_Reg ( EAX, callToAddr, CELL ) ;
    //_Compile_JumpToReg ( EAX ) ;
}

#if RETURN_STACK

void
_Compile_JmpCall_Using_RStack ( byte * jmpToAddr )
{
    // push rstack here + 5 so RET can jmp back 
    _Compile_MoveImm_To_Reg ( EAX, &Rsp, CELL ) ; // the lvalue of Rsp
    Compile_ADDI ( MEM, EAX, 0, CELL, BYTE ) ; // add 4 to Rsp
    Compile_ADDI ( REG, EAX, 0, CELL, BYTE ) ; // 
    //_Compile_Move_Reg_To_Reg ( int32 dstReg, int32 srcReg ) ;
    _Compile_MoveImm_To_Reg ( ECX, Here + x, CELL ) ; // x : number of bytes to the first byte after the jmp instruction
    _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
    _Compile_JumpToAddress ( byte * jmpToAddr ) ;
}

void
_Compile_Return_Using_RStack ( )
{
    // pop rstack to EAX
    //_Compile_JumpToReg ( EAX ) ;
}

#endif

void
_Compile_TEST_Reg_To_Reg ( int32 dstReg, int32 srcReg )
{
    _Compile_Op_Special_Reg_To_Reg ( TEST_R_TO_R, dstReg, srcReg ) ;
}

void
_Compile_Return ( )
{
    Compile_StartOpCode_Int8 ( 0xc3 ) ;
    //RET ( ) ; // use codegen_x86.h just to include it in
    // pop rstack to EAX
    //_Compile_JumpToReg ( EAX ) ;
}

// push onto the C esp based stack with the 'push' instruction

void
_Compile_PushReg ( int32 reg )
{
    // only EAX ECX EDX EBX : 0 - 4
    Compile_StartOpCode_Int8 ( 0x50 + reg ) ;
}

// pop from the C esp based stack with the 'pop' instruction

void
_Compile_PopToReg ( int32 reg )
{
    // only EAX ECX EDX EBX : 0 - 4
    Compile_StartOpCode_Int8 ( 0x58 + reg ) ;
}

void
_Compile_PopAD ( )
{
    Compile_StartOpCode_Int8 ( 0x61 ) ;
}

void
_Compile_PushAD ( )
{
    Compile_StartOpCode_Int8 ( 0x60 ) ;
}

void
_Compile_PopFD ( )
{
    Compile_StartOpCode_Int8 ( 0x9d ) ;
}

void
_Compile_PushFD ( )
{
    Compile_StartOpCode_Int8 ( 0x9c ) ;
}

void
_Compile_Sahf ( )
{
    Compile_StartOpCode_Int8 ( 0x9e ) ;
}

void
_Compile_Lahf ( )
{
    Compile_StartOpCode_Int8 ( 0x9f ) ;
}

void
_Compile_IRET ( )
{
    Compile_StartOpCode_Int8 ( 0xcf ) ;
}

void
_Compile_INT3 ( )
{
    Compile_StartOpCode_Int8 ( 0xcc ) ;
}

void
_Compile_INT80 ( )
{
    Compile_StartOpCode_Int8 ( 0xcd ) ;
    _Compile_Int8 ( 0x80 ) ;
}

void
_Compile_Noop ( )
{
    Compile_StartOpCode_Int8 ( 0x90 ) ;
}

// Zero eXtend from byte to cell

void
_Compile_MOVZX_REG ( int32 reg )
{
    Compile_StartOpCode_Int8 ( ( byte ) 0x0f ) ;
    _Compile_Int8 ( 0xb6 ) ;
    _Compile_Int8 ( _CalculateModRmByte ( REG, reg, reg, 0, 0 ) ) ;
}

// inc/dec only ( not call or push which are also group 5 - cf : sandpile.org )

void
Compile_X_Group5 ( Compiler * compiler, int32 op )
{
    int optFlag = CheckOptimize ( compiler, 3 ) ;
    //Word *one = Compiler_WordStack ( - 1 ) ; // assumes two values ( n m ) on the DSP stack 
    Word *one = Compiler_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_MoveImm_To_Reg ( EAX, compiler->optInfo->Optimize_Imm, CELL ) ;
            compiler->optInfo->Optimize_Mod = REG ;
            compiler->optInfo->Optimize_Rm = EAX ;
        }
        _Compile_Group5 ( op, compiler->optInfo->Optimize_Mod, compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, 0 ) ;
    }
    else if ( one && one->CProperty & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) ) // *( ( cell* ) ( TOS ) ) += 1 ;
    {
        SetHere ( one->Coding ) ;
        _Compile_GetVarLitObj_RValue_To_Reg ( one, EAX ) ;
        //_Compile_Group5 ( int32 code, int32 mod, int32 rm, int32 sib, int32 disp, int32 size )
        _Compile_Group5 ( op, REG, EAX, 0, 0, 0 ) ;
        // ++ == += :: -- == -= so :
        _Compile_SetVarLitObj_With_Reg ( one, EAX, ECX ) ;
    }
    else
    {
        // assume rvalue on stack
        _Compile_Group5 ( op, MEM, DSP, 0, 0, 0 ) ;
    }
    _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO_TTT, NZ, 3 ) ; // ?? // not less than 0 == greater than 0
}

// X variable op compile for group 1 opCodes : +/-/and/or/xor - ia32 

void
_Compile_optInfo_X_Group1 ( Compiler * compiler, int32 op )
{
    Set_SCA (0) ;
    if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM )
    {
        // Compile_SUBI( mod, operandReg, offset, immediateData, size )
        _Compile_X_Group1_Immediate ( op, compiler->optInfo->Optimize_Mod,
            compiler->optInfo->Optimize_Rm, compiler->optInfo->Optimize_Disp,
            compiler->optInfo->Optimize_Imm, CELL ) ;
    }
    else
    {
        // _Compile_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
        _Compile_X_Group1 ( op, compiler->optInfo->Optimize_Dest_RegOrMem, compiler->optInfo->Optimize_Mod,
            compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Rm, 0,
            compiler->optInfo->Optimize_Disp, CELL ) ;
    }
}

// subtract second operand from first and store result in first

// X variable op compile for group 1 opCodes : +/-/and/or/xor - ia32 

void
Compile_X_Group1 ( Compiler * compiler, int32 op, int32 ttt, int32 n )
{
    int optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        _Compile_optInfo_X_Group1 ( compiler, op ) ;
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ttt, n, 3 ) ; // not less than 0 == greater than 0
        if ( compiler->optInfo->Optimize_Rm != DSP ) // if the result is to a reg and not tos
        {
            _Word_CompileAndRecord_PushReg ( Compiler_WordList ( 0 ), compiler->optInfo->Optimize_Reg ) ; //compiler->optInfo->Optimize_Rm ) ; // 0 : ?!? should be the exact variable 
        }
    }
    else
    {
        Compile_Pop_To_EAX ( DSP ) ;
        //_Compile_X_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
        _Compile_X_Group1 ( op, MEM, MEM, EAX, DSP, 0, 0, CELL ) ; // result is on TOS
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ttt, n, 3 ) ; // not less than 0 == greater than 0
        //_Word_CompileAndRecord_PushEAX ( Compiler_WordList ( 0 ) ) ;
    }
}

// first part of "combinator tookit"

void
_Compile_Jcc ( int32 bindex, int32 overwriteFlag, int32 nz, int32 ttt )
{
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( _Context_->Compiler0->CombinatorBlockInfoStack, bindex ) ; // -1 : remember - stack is zero based ; stack[0] is top
    if ( Compile_CheckReConfigureLogicInBlock ( bi, overwriteFlag ) )
    {
        //_Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( ! bi->NegFlag, bi->Ttt, 0 ) ; // we do need to store and get this logic set by various conditions by the compiler : _Compile_SET_tttn_REG
    }
    else
    {
        Compile_GetLogicFromTOS ( bi ) ; // after cmp we test our condition with setcc. if cc is true a 1 will be sign extended in EAX and pushed on the stack 
        // then in the non optimized|inline case we cmp the TOS with 0. If ZERO (zf is 1) we know the test was false (for IF), if N(ot) ZERO we know it was true 
        // (for IF). So, in the non optimized|inline case if ZERO we jmp if N(ot) ZERO we continue. In the optimized|inline case we check result of first cmp; if jcc sees
        // not true (with IF that means jcc N(ot) ZERO) we jmp and if true (with IF that means jcc ZERO) we continue. 
        // nb. without optimize|inline there is another cmp in Compile_GetLogicFromTOS which reverse the polarity of the logic 
        // ?? an open question ?? i assume it works the same in all cases we are using - exceptions ?? 
        // so adjust ...
        Compile_JCC ( Z, ttt, 0 ) ;
    }
}

#if 1
int32 *_Dsp_ ;
void
_Compile_Sync_EsiToDsp ( int32 thruReg )
{
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Dsp_, ESI, thruReg ) ; // esp //this won't be accurate for the runtime because it is called from C 
}
void
_Compile_Sync_DspToEsi ()
{
    _Compile_Get_FromCAddress_ToReg ( ESI, ( byte* ) & _Dsp_ ) ; 
}
#endif