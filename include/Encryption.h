//===----------- Encryption.h - General Encryption of Variables ------------===//
//
// This defines the general encryption of all primitive types, including strings.
// Based on one simple substitution of instructions with XOR ones and another
// more complicated function-driven algorithms. 
//
//===----------------------------------------------------------------------===//

#pragma once

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include <time.h>

namespace llvm
{

class Encryption
{
public:
  /*
   * The key represented by a simple timestamp used for further encryption.
   * Initialized globally so that functions outside of the class such as
   * buildFunc() have access to the key at any time without instantiating
   * the class.
   */
  inline static uint32_t _time = time(nullptr);
  /*
   * Constructor accepts a GV by reference and uses it as a pointer
   * through the _GV private variable. Conversion to pointer is needed
   * as the GlobalVariable class is set up in a way such that it avoids
   * copying any instances of GlobalVariable, and hence the GV is processed
   * as a pointer inside the class.
   *
   * NOTE: No destructor is needed, GVs are used after this pass
   * completes and deallocating this memory would result in a crash.
   */
  Encryption(GlobalVariable& GV);
  /*
   * This encryption level modifies the LOAD/STORE instructions that
   * invoke the GV so they decrypt the GV in runtime before it's actually
   * accessed. The GV is of course initialized with a value obfuscated with
   * the timestamp key.
   */
  void runInstLevel();
  /*
   * This encryption level runs a custom improvised enryption function on
   * the GV's initialized value and inserts a decryption function that
   * decrypts the initialized value in runtime into the module.
   */
  void runFuncLevel();

private:
  GlobalVariable *_GV;
  Constant *_CX;
  uint8_t *_buffer;
  size_t _nbits;
  size_t _size;

  /*
   * Dynamically convert an integer to an appropriate floating point variable.
   * Produces an error if the floating point GV that's being processed is larger
   * than 64 bits.
   */
  Value *intToTypedFP(IRBuilder<>& IRB, Value* Int);
  /*
   * Statically encrypt (XOR) the buffer corresponding to the GV's value V and
   * store the result (encrypted value) back to V. Used to then overwrite the
   * initializer with the new value.
   */
  void instEncryptBuffer(APInt& V);
  /*
   * Dynamically decrypt the floating point GV by simply converting it to an
   * integer, XORing with the timestamp key and converting back using intToTypedFP.
   */
  Value *instDecryptFP(IRBuilder<>& IRB, Value* Enc, Value** toEx);
  /*
   * Statically encrypt the buffer but based on a more complicated improvised
   * encryption algorithm instead of a single XOR.
   */
  void funcEncryptBuffer(APInt& V);
  /*
   * Dynamically decrypt the previously encrypted integer Enc with the timestamp
   * key. Returns the decrypted integer loaded value and allows to provide a toEx
   * argument voluntarily for further use.
   */
  Value *funcDecryptInteger(IRBuilder<>& IRB, Value* Enc, Value** toEx);
  /*
   * Dynamically decrypt the previously encrypted float Enc by converting the value
   * to an integer, invoking funcDecryptInteger and converting it back to a floating
   * point variable of the appropriate size. Returns the floating point loaded value,
   * also allowing to provide a toEx.
   */
  Value *funcDecryptFP(IRBuilder<>& IRB, Value* Enc, Value** toEx);
};

class EncryptionPass : public PassInfoMixin<EncryptionPass>
{
public:
  PreservedAnalyses run(Module& M, ModuleAnalysisManager& AM);
};

} // namespace llvm