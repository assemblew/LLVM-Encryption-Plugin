//===---------- Encryption.cpp - General Encryption of Variables -----------===//
//
// This defines the general encryption of all primitive types, including strings.
// Based on one simple substitution of instructions with XOR ones and another
// more complicated function-driven algorithms. 
//
//===-----------------------------------------------------------------------===//

#include "Encryption.h"

using namespace llvm;

Encryption::Encryption(GlobalVariable& GV)
{
  // The GV is guaranteed to correspond to a valid type that the class is likely able
  // to process, so the constructor must accept the GV as it is and the initializer must
  // be stored to further convert it to appropriate types.
  _GV = &GV;
  _CX = GV.getInitializer();
  _nbits = GV.getValueType()->getPrimitiveSizeInBits();
  _size = (_nbits + 7) / 8;
  _buffer = new uint8_t[_size];

  outs() << "Encrypting " << GV.getName() << " with key " << _time << "..." << '\n';
}

// Set the IRB's insertion point appropriately, which decides whether the next instructions
// should be inserted before or after the instruction I that corresponds to the instruction
// that holds the encrypted value.
void setupBuilder(IRBuilder<>& IRB, Instruction* I)
{
  auto It = BasicBlock::iterator(I);
  isa<LoadInst>(I) ? IRB.SetInsertPoint(++It) : IRB.SetInsertPoint(It);
}

// Overwrite the load instruction LI so that all its uses are substituted with Sub. The Ex
// value ensures the instruction itself remains unchanged.
void overwriteLoad(LoadInst* LI, Value* Sub, Value* Ex)
{
  LI->replaceUsesWithIf(Sub,
  [Ex](Use &U)
  {
    return U.getUser() != Ex;
  });
  outs() << "Overwritten a LOAD instruction successfully." << '\n';
}

// Overwrite the store instruction SI so that its value operand is substituted with Sub.
void overwriteStore(StoreInst* SI, Value* Sub)
{
  SI->replaceUsesOfWith(SI->getValueOperand(), Sub);
  outs() << "Overwritten a STORE instruction successfully." << '\n';
}

void Encryption::instEncryptBuffer(APInt& V)
{
  // Global Variable's value V retrieval and cast to uint8_t* buffer, in other words the
  // V's by-byte representation (generic). The timestamp key of 4 bytes can be less than
  // the processed value's size and therefore the buffer is encrypted byte-by-byte, resulting
  // in the key's last bytes being ignored in the case of encryption of smaller types and
  // repeated in the case of bigger types.
  StoreIntToMemory(V, _buffer, _size);
  for (size_t i = 0; i < _size; i++)
    _buffer[i] ^= ((uint8_t *)&_time)[i % sizeof(_time)];
  LoadIntFromMemory(V, _buffer, _size);
}

Value *Encryption::intToTypedFP(IRBuilder<>& IRB, Value* Int)
{
  Value *FP = nullptr;
  // Typecast to ConstantFP to get the type ID for the switch statement, while using cast<>
  // means an unreliable cast (no checks to ensure that _CX is a valid ConstantFP are performed),
  // but it's valid in this scope as instDecryptFP is guaranteed to be called inside the body of
  // a condition that does the necessary checks.
  switch (cast<ConstantFP>(_CX)->getType()->getTypeID())
  {
    case Type::HalfTyID:
      FP = IRB.CreateBitCast(Int, IRB.getHalfTy());
      break;
    case Type::BFloatTyID:
      FP = IRB.CreateBitCast(Int, IRB.getBFloatTy());
      break;
    case Type::FloatTyID:
      FP = IRB.CreateBitCast(Int, IRB.getFloatTy());
      break;
    case Type::DoubleTyID:
      FP = IRB.CreateBitCast(Int, IRB.getDoubleTy());
      break;
    default:
      // Output an error if the floating point variable is larger than 64 bits.
      outs() << "Unsupported floating point type. Returning..." << '\n';
  }
  return FP;
}

Value *Encryption::instDecryptFP(IRBuilder<>& IRB, Value* Enc, Value** toEx = nullptr)
{  
  // Pick an integer of N bits to flexibly convert float to int and backwards.
  Value *IV = IRB.CreateBitCast(Enc, IRB.getIntNTy(_nbits));
  Value *XI = IRB.CreateXor(IV, IRB.getIntN(_nbits, _time));
  Value *FV = intToTypedFP(IRB, XI);
  // In case of overwriting a load instruction, operands of the instruction are not being
  // overwritten. Instead, its uses are changed to the new decrypted instruction. Hence,
  // must ensure that the first use of the load (where the initial value is obtained) is
  // not being overwritten, which is done in overwriteLoad. However, the first use only
  // conveniently exists in this particular function, so better to pass it to be further
  // excluded via toEx reference, which is specified accordingly.
  if (toEx)
    *toEx = IV;
  return FV;
}

void Encryption::runInstLevel()
{
  outs() << "Running the instruction-based level of encryption..." << '\n';
  if (ConstantInt *CI = dyn_cast<ConstantInt>(_CX))
  {
    APInt V = CI->getValue();
    instEncryptBuffer(V);

    // Create and substitute with a new GlobalVariable initializer (value) XORed with the
    // generated key. It's basically just changing the initial value by utilizing XOR
    // (Exclusive OR) operation, where the final value is the initial value XORed with the
    // key value bit by bit.
    _GV->setInitializer(ConstantInt::get(CI->getIntegerType(), V));

    // Create the key constant to use as an operand to the XOR instruction.
    ConstantInt *K = ConstantInt::get(CI->getIntegerType(), _time);

    // Retrieve all users who interact with the global variable in any way.
    for (User *U : _GV->users())
    {
      // Check if the global variable is used by the Load instruction (LoadInst).
      // This is required to decrypt the initialized value with our key while reading from the
      // memory. In other words, it takes the initial (substituted) encrypted value and performs
      // XOR with the key, to make the original value of the variable known for places where it is
      // used, such as in method invocations or other variables.
      // Basically, the ASM process of decrypting the initialized variable looks like this:
      //
      // mov     esi, cs:__ZZ2aaiE8int32_ex ; aa(int)::int32_ex
      // xor     esi, 0B7B634A7h
      // 
      // where it first loads the variable from data segment and then XORs it with the key,
      // meanwhile the variable in data looks like this:
      //
      // __ZZ2aaiE8int32_ex dd 0B7B63443h
      //
      // representing the value initially XORed with the key.
      // If we attempt to generate pseudocode of the aa(int) function, the variable called will look like this:
      //
      // printf("%i\n", aa(int)::int32_ex ^ 0xB7B634A7);
      //
      // meaning the same decryption idea as in the ASM.
      if (LoadInst *LI = dyn_cast<LoadInst>(U))
      {
        Instruction *XI = BinaryOperator::Create(Instruction::Xor, LI, K,
                                                 "", ++BasicBlock::iterator(LI));
        overwriteLoad(LI, XI, XI);
      }
      // Check if the global variable is used by the Store instruction (StoreInst).
      // Required to decrypt our variable while storing to the memory by overriding
      // the SI's value operand.
      else if (StoreInst *SI = dyn_cast<StoreInst>(U))
        overwriteStore(SI, BinaryOperator::Create(Instruction::Xor,
                                                  SI->getValueOperand(),
                                                  K, "", BasicBlock::iterator(SI)));
    }
  }
  else if (ConstantFP *CFP = dyn_cast<ConstantFP>(_CX))
  {
    // Temporarily cast to an integer representation to encrypt conveniently and then
    // cast back when setting the new initializer.
    APInt V = CFP->getValue().bitcastToAPInt();
    instEncryptBuffer(V);

    Type *T = CFP->getType();
    _GV->setInitializer(ConstantFP::get(T, APFloat(T->getFltSemantics(), V)));

    for (User *U : _GV->users())
    {
      if (LoadInst *LI = dyn_cast<LoadInst>(U))
      {
        IRBuilder<> IRB(LI->getContext());
        Value *Ex;

        setupBuilder(IRB, LI);
        overwriteLoad(LI, instDecryptFP(IRB, LI, &Ex), Ex);
      }
      else if (StoreInst *SI = dyn_cast<StoreInst>(U))
      {
        IRBuilder<> IRB(SI->getContext());

        setupBuilder(IRB, SI);
        overwriteStore(SI, instDecryptFP(IRB, SI->getValueOperand()));
      }
    }
  }
  else
  {
    outs() << "Unsupported type passed to instruction level. Returning..." << '\n';
    return;
  }
  // Update the initializer in the case if another level of encryption will then use it.
  _CX = _GV->getInitializer();
  outs() << "Done." << '\n';
}

void Encryption::funcEncryptBuffer(APInt& V)
{
  StoreIntToMemory(V, _buffer, _size);
  uint8_t *time_bytes = (uint8_t *)&_time;

  for (size_t i = 0; i < _size / 2; i++) 
  {
    size_t j = _size - i - 1;

    _buffer[i] ^= _buffer[j];
    _buffer[j] ^= _buffer[i];
    _buffer[i] ^= _buffer[j];

    _buffer[i] ^= time_bytes[i % 4];
    _buffer[j] ^= time_bytes[j % 4];
  }
  if (_size % 2 != 0) 
  {
    size_t mid = _size / 2;
    _buffer[mid] ^= time_bytes[mid % 4];
  }

  uint8_t salt[] = { 0x22, 0x80, 0xDE, 0xA4, 0xB2, 0xEF };
  for (int r = 0; r < 3; r++) 
  {
    for (size_t i = 0; i < _size; i++) 
    {
      _buffer[i] ^= time_bytes[(i + r) % 4];
      _buffer[i] ^= salt[i % sizeof(salt)];
      _buffer[i] = (_buffer[i] << 1) | (_buffer[i] >> 7);
      _buffer[i] ^= (uint8_t)(_time >> (8 * (i % 4)));
    }
  }
  LoadIntFromMemory(V, _buffer, _size);
}

Value *Encryption::funcDecryptInteger(IRBuilder<>& IRB, Value* Enc, Value** toEx = nullptr)
{
  // The type here is guaranteed to be an integer type, however, it must correspond
  // to a type of a valid number of bytes, so just get the provided value's type for 
  // simplicity. Also, getting the type straight through the encrypted value's pointer
  // allows for use of the function as a wrapper in float decryption algorithm.
  AllocaInst *Copy = IRB.CreateAlloca(Enc->getType());
  StoreInst *SI = IRB.CreateStore(Enc, Copy);
  IRB.CreateCall(FunctionCallee(_GV->getParent()->getFunction("decrypt")),
                                {Copy, IRB.getIntN(_nbits, sizeof(_time))});

  if (toEx)
    *toEx = SI;
  return IRB.CreateLoad(Enc->getType(), Copy);
}

Value *Encryption::funcDecryptFP(IRBuilder<>& IRB, Value* Enc, Value** toEx = nullptr)
{
  // Bitcast to an integer, which is decrypted using the integer decryption function and
  // then cast back, whereas the resulting FV will be the decrypted float value.
  Value *IV = IRB.CreateBitCast(Enc, IRB.getIntNTy(_nbits));
  Value *DI = funcDecryptInteger(IRB, IV);
  Value *FV = intToTypedFP(IRB, DI);

  if (toEx)
    *toEx = IV;
  return FV;
}

void Encryption::runFuncLevel()
{
  outs() << "Running the function-based level of encryption..." << '\n';

  if (ConstantInt *CI = dyn_cast<ConstantInt>(_CX))
  {
    APInt V = CI->getValue();
    funcEncryptBuffer(V);

    _GV->setInitializer(ConstantInt::get(CI->getIntegerType(), V));

    for (User *U : _GV->users())
    {
      if (LoadInst *LI = dyn_cast<LoadInst>(U))
      {
        IRBuilder<> IRB(LI->getContext());
        Value *Ex;

        setupBuilder(IRB, LI);
        overwriteLoad(LI, funcDecryptInteger(IRB, LI, &Ex), Ex);
      }
      else if (StoreInst *SI = dyn_cast<StoreInst>(U))
      {
        IRBuilder<> IRB(SI->getContext());

        setupBuilder(IRB, SI);
        overwriteStore(SI, funcDecryptInteger(IRB, SI->getValueOperand()));
      }
    }
  }
  else if (ConstantFP *CFP = dyn_cast<ConstantFP>(_CX))
  {
    APInt V = CFP->getValue().bitcastToAPInt();
    funcEncryptBuffer(V);

    Type *T = CFP->getType();
    _GV->setInitializer(ConstantFP::get(T, APFloat(T->getFltSemantics(), V)));
    
    for (User *U : _GV->users())
    {
      if (LoadInst *LI = dyn_cast<LoadInst>(U))
      {
        IRBuilder<> IRB(LI->getContext());
        Value *Ex;

        setupBuilder(IRB, LI);
        overwriteLoad(LI, funcDecryptFP(IRB, LI, &Ex), Ex);
      }
      else if (StoreInst *SI = dyn_cast<StoreInst>(U))
      {
        IRBuilder<> IRB(SI->getContext());

        setupBuilder(IRB, SI);
        overwriteStore(SI, funcDecryptFP(IRB, SI->getValueOperand()));
      }
    }
  }
  else
  {
    outs() << "Unsupported type passed to function level. Returning..." << '\n';
    return;
  }
  _CX = _GV->getInitializer();
  outs() << "Done." << '\n';
}

void buildFunc(Module& M)
{
  Function *Func = Function::Create(FunctionType::get(Type::getVoidTy(M.getContext()),
                                    {PointerType::getUnqual(Type::getInt8Ty(M.getContext())),
                                    Type::getInt64Ty(M.getContext())}, false),
                                    GlobalValue::PrivateLinkage, Twine("decrypt"), M);

  Function::arg_iterator Iter = Func->arg_begin();
  Value *Data = Iter;
  Value *Length = ++Iter;

  BasicBlock *Entry = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Init_1 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *True_1 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Init_2 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *True_2 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Incr_1 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Jump = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Decr = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *False_1 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Init_3 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *True_3 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Incr_2 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *False_2 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *True_4 = BasicBlock::Create(M.getContext(), "", Func);
  BasicBlock *Exit = BasicBlock::Create(M.getContext(), "", Func);

  IRBuilder<> IRB(Entry);

  // Time allocation and value storage
  Value *T = IRB.CreateAlloca(IRB.getInt32Ty());
  IRB.CreateStore(IRB.getInt32(Encryption::_time), T);

  // Time casted to bytes (ptr) allocation and value storage
  Value *TB = IRB.CreateAlloca(PointerType::getUnqual(IRB.getInt8Ty()));
  IRB.CreateStore(T, TB);

  // Salt array allocation and value storage
  Value *S = IRB.CreateAlloca(ArrayType::get(IRB.getInt8Ty(), 6));
  IRB.CreateStore(ConstantDataArray::get(M.getContext(), ArrayRef<uint8_t>({0x22, 0x80, 0xDE, 0xA4, 0xB2, 0xEF})), S);

  // Time bytes value loaded
  Value *TBV = IRB.CreateLoad(PointerType::getUnqual(IRB.getInt8Ty()), TB);

  // 'i' loop variable allocation and value storage
  Value *I = IRB.CreateAlloca(IRB.getInt8Ty());
  IRB.CreateStore(IRB.getInt8(2), I);
  IRB.CreateBr(Init_1);

  IRB.SetInsertPoint(Init_1);
  // 'i' loop variable value loaded
  Value *IV = IRB.CreateLoad(IRB.getInt8Ty(), I);
  IRB.CreateCondBr(IRB.CreateICmpSGE(IV, IRB.getInt8(0)), True_1, False_1);

  IRB.SetInsertPoint(True_1);
  // 'j' loop variable allocation and value storage
  Value *J = IRB.CreateAlloca(IRB.getInt64Ty());
  IRB.CreateStore(IRB.getInt64(0), J);
  IRB.CreateBr(Init_2);

  IRB.SetInsertPoint(Init_2);
  // 'j' loop variable value loaded
  Value *JV = IRB.CreateLoad(IRB.getInt64Ty(), J);
  IRB.CreateCondBr(IRB.CreateICmpULT(JV, Length), True_2, Jump);

  IRB.SetInsertPoint(True_2);
  Value *DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateTrunc(IRB.CreateLShr(IRB.CreateLoad(IRB.getInt32Ty(), T), IRB.CreateTrunc(IRB.CreateMul(IRB.CreateURem(JV, IRB.getInt64(4)), IRB.getInt64(8)), IRB.getInt32Ty())), IRB.getInt8Ty()), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  Value *DSVZE = IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty());
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateOr(IRB.CreateAShr(DSVZE, IRB.getInt32(1)), IRB.CreateShl(DSVZE, IRB.getInt32(7))), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(ArrayType::get(IRB.getInt8Ty(), 6), S, {IRB.getInt64(0), IRB.CreateURem(JV, IRB.getInt64(6))})), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), TBV, IRB.CreateURem(IRB.CreateAdd(JV, IRB.CreateSExt(IV, IRB.getInt64Ty())), IRB.getInt64(4)))), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);
  IRB.CreateBr(Incr_1);

  IRB.SetInsertPoint(Incr_1);
  IRB.CreateStore(IRB.CreateAdd(JV, IRB.getInt64(1)), J);
  IRB.CreateBr(Init_2);

  IRB.SetInsertPoint(Jump);
  IRB.CreateBr(Decr);

  IRB.SetInsertPoint(Decr);
  IRB.CreateStore(IRB.CreateNSWAdd(IV, IRB.getInt8(-1)), I);
  IRB.CreateBr(Init_1);

  IRB.SetInsertPoint(False_1);
  I = IRB.CreateAlloca(IRB.getInt64Ty());
  IRB.CreateStore(IRB.getInt64(0), I);
  IRB.CreateBr(Init_3);

  IRB.SetInsertPoint(Init_3);
  IV = IRB.CreateLoad(IRB.getInt64Ty(), I);
  IRB.CreateCondBr(IRB.CreateICmpULT(IV, IRB.CreateUDiv(Length, IRB.getInt64(2))), True_3, False_2);

  IRB.SetInsertPoint(True_3);
  J = IRB.CreateAlloca(IRB.getInt64Ty());
  IRB.CreateStore(IRB.CreateSub(IRB.CreateSub(Length, IV), IRB.getInt64(1)), J);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, IV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), TBV, IRB.CreateURem(IV, IRB.getInt64(4)))), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  JV = IRB.CreateLoad(IRB.getInt64Ty(), J);
  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), TBV, IRB.CreateURem(JV, IRB.getInt64(4)))), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, IV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV)), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, IV)), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, IV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, JV)), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);
  IRB.CreateBr(Incr_2);

  IRB.SetInsertPoint(Incr_2);
  IRB.CreateStore(IRB.CreateAdd(IV, IRB.getInt64(1)), I);
  IRB.CreateBr(Init_3);

  IRB.SetInsertPoint(False_2);
  Value *R = IRB.CreateURem(Length, IRB.getInt64(2));
  Value *Cond = IRB.CreateICmpNE(R, IRB.getInt64(0));
  IRB.CreateCondBr(Cond, True_4, Exit);

  IRB.SetInsertPoint(True_4);
  Value *_M = IRB.CreateAlloca(IRB.getInt64Ty());
  IRB.CreateStore(IRB.CreateUDiv(Length, IRB.getInt64(2)), _M);
  Value *MV = IRB.CreateLoad(IRB.getInt64Ty(), _M);

  DS = IRB.CreateInBoundsGEP(IRB.getInt8Ty(), Data, MV);
  IRB.CreateStore(IRB.CreateTrunc(IRB.CreateXor(IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), DS), IRB.getInt32Ty()), IRB.CreateZExt(IRB.CreateLoad(IRB.getInt8Ty(), IRB.CreateInBoundsGEP(IRB.getInt8Ty(), TBV, IRB.CreateURem(MV, IRB.getInt64(4)))), IRB.getInt32Ty())), IRB.getInt8Ty()), DS);
  IRB.CreateBr(Exit);

  IRB.SetInsertPoint(Exit);
  IRB.CreateRetVoid();

  outs() << "Decrypt function built successfully." << '\n';
}

PreservedAnalyses EncryptionPass::run(Module& M, ModuleAnalysisManager& AM)
{
  outs() << "Running Encryption On Module " << M.getName() << "..." << '\n';

  // Build the decryption function to further invoke in runtime.
  //if (RunE2)
    buildFunc(M);

  // Iterate through all known global variables in the module using the class's iterator wrapper.
  // 'const' keyword prevents variables' belonging to GlobalVariable and therefore their presence
  // in GlobalList (Module class's private variable from which the global iterator is derived).
  for (auto& GV : M.globals())
  {
    // Check whether the global variable is of private linkage 
    // (only directly accessible by objects in the current module)
    // or internal (similar to private but corresponds to the 'static' keyword in C).
    // Also check for the initializer, which is the value with which
    // the global variable has been initialized. The keyword 'static'
    // leads the variable to have the initializer, unlike if the variable
    // has been initialized with no keywords (corresponding to a dynamic behaviour).
    // It is also important to note that global variables are not necessarily
    // initialized in the global scope and they can also be initialized inside a function.
    if (GV.hasInitializer() && (GV.hasPrivateLinkage() || GV.hasInternalLinkage()))
    {
      // TODO: Extend this to array variables and (maybe) all chars.
      if (GV.getValueType()->isIntegerTy() || GV.getValueType()->isFloatingPointTy()/*isa<ConstantInt>(GV.getInitializer()) ||
          isa<ConstantFP>(GV.getInitializer()) ||
          isa<ConstantDataArray>(GV.getInitializer()) ||
          isa<ConstantDataVector>(GV.getInitializer())*/)
      {
        Encryption encryption = Encryption(GV);
        //if (RunE1)
        //if (RunE2)
          encryption.runInstLevel();
          encryption.runFuncLevel();
      }
    }
  }
  return PreservedAnalyses::none();
}

extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
  return {LLVM_PLUGIN_API_VERSION, "EncryptionPass", LLVM_VERSION_STRING,
  [](PassBuilder &PB)
  {
    PB.registerPipelineParsingCallback(
    [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>)
    {
      if (Name == "encryption")
      {
        MPM.addPass(EncryptionPass());
        return true;
      }
      return false;
    });
    PB.registerPipelineStartEPCallback(
    [](ModulePassManager &MPM, OptimizationLevel Level)
    {
      MPM.addPass(EncryptionPass());
    });
  }};
}