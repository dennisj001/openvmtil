#include "../../include/cfrtil.h"
/* notation :
 * e = equal, s = store, o1 = op 1 arg, o2 = op 2 arg, ws = wordStack, f = fetch
 * v = variable, o = object, c = constant
 * o 1 arg = f | other o 1 arg
 * runtime structures :
 *      wordStack[64], dataStack[n], objectStack[32] or v|o|c stack, code memory
 * ws word struct { codeStart; codeEnd || codeSize ; }
 * void
 * SetupOptimize ()
 * {
 *      if ( ws[0] == o1 )
 *      {
 *          if (ws[1] == f ) 
 *          {
 *              ws [0] = w[1] o1 ; // rpn apply
 *              compile push to eax and push to dataStack : this code could be rewritten 
 *          }
 *          else return : for compile of w[1] o1 => eax if first arg, ecx if second arg
 *      }
 *      else //if ws [0] == e|s|o2 ) 
 *      {
 *          if ( ( ws [1] != o) && ( ws [2] != o ) )
 *          {
 *              SetHere ( ws [2].code ) ; // stack push reg code
 *              rewrite ws [2] => eax (if it is compiled to ecx)
 *              rewrite ws [1] (if it is compiled to eax) or move existing ws[1].code => ecx
 *              return | switch : for compile of ws[1] ws[2] o.ws[0] || null ;
 *              // all o compiled to EAX and pushed on dataStack :: op eax, ecx ; add dsp, 4 ; mov dsp[0], eax
 *              // all e|s is :: e : mov eax, [ecx] || s :: mov ecx, [eax]
 *          }
 *      }
 * }
 * // separate logic ops from 'cmp/test/set' - conditional - 'if', etc. compiles 'cmp/test/set' with WD_LogicCodeToEaxToStack
 * struct optInfo
 * {
 *      flag eax ; // if eax acc arg is compiled
 * }
 */