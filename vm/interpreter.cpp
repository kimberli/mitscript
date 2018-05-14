/*
 * interpreter.cpp
 *
 * Implements an Interpreter object which can be used to step through
 * bytecode instructions and store interpreter state
 */
#include "interpreter.h"
#include <algorithm>
#include <stack>

using namespace std;

Interpreter::Interpreter(Function* mainFunc, int maxmem, bool callAsm) {
    // initialize the garbage collector
    // note that mainFunc is not included in the gc's allocated list because
    // we never have to deallocate it
    collector = new CollectedHeap(maxmem, mainFunc->getSize(), &frames);

    // initialize a static none
    NONE = make_ptr(new None());

    // initialize the root frame
    int numLocals = mainFunc->names_.size();
    int numRefs = 0;
    if (mainFunc->local_reference_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero ref vars");
    }
    if (mainFunc->free_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero free vars");
    }
    if (mainFunc->local_vars_.size() != 0) {
        throw RuntimeException("can't initialize root frame w/ nonzero local vars");
    }
    mainFunc->local_vars_ = mainFunc->names_;
    Frame* frame = collector->allocate<Frame>(mainFunc);
    frame->collector = collector;
    globalFrame = frame;
    frames.push_back(frame);
    finished = false;
    shouldCallAsm = callAsm;

    // set up native functions at the beginning of functions array
	vector<Function*> functions_;
    vector<tagptr_t> constants_;
	vector<string> args0 ;
	vector<string> args1 = { string("s") };
    vector<string> local_reference_vars_;
    vector<string> free_vars_;
	vector<string> names_;
    BcInstructionList instructions;
	vector<shared_ptr<Function>> frameFuncs;
	frame->func->functions_[0] = collector->allocate<PrintNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[1] = collector->allocate<InputNativeFunction>(functions_, constants_, 0, args0, local_reference_vars_, free_vars_, names_, instructions);
	frame->func->functions_[2] = collector->allocate<IntcastNativeFunction>(functions_, constants_, 1, args1, local_reference_vars_, free_vars_, names_, instructions);
};

void Interpreter::executeStep() {
    // executes a single instruction and updates state of interpreter
    Frame* frame = frames.back();
    BcInstruction& inst = frame->getCurrInstruction();
    LOG("executing instruction " + to_string(frame->instructionIndex) + " from frame " + to_string(frames.size()));
    switch (inst.operation) {
        case BcOp::LoadConst:
            {
                auto constant = frame->getConstantByIndex(inst.operand0.value());
                frame->opStackPush(constant);
                frame->instructionIndex++;
                break;
            }
        case BcOp::LoadFunc:
            {
                auto func = frame->getFunctionByIndex(inst.operand0.value());
                frame->opStackPush(make_ptr(func));
                frame->instructionIndex++;
                break;
            }
        case BcOp::LoadLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                frame->opStackPush(frame->getLocalVar(name));
                frame->instructionIndex++;
                break;
            }
        case BcOp::StoreLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                tagptr_t ptr = frame->opStackPop();
                frame->setLocalVar(name, ptr);
                frame->instructionIndex++;
                break;
            }
        case BcOp::LoadGlobal:
            {
                int index = inst.operand0.value();
                string name = frame->getNameByIndex(index);
                frame->opStackPush(loadGlobal(name));
                frame->instructionIndex++;
                break;
            }
        case BcOp::StoreGlobal:
            {
                int index = inst.operand0.value();
                string name = frame->getNameByIndex(index);
                storeGlobal(name, frame->opStackPop());
                frame->instructionIndex++;
                break;
            }
        case BcOp::PushReference:
            {
                string name = frame->getRefByIndex(inst.operand0.value());
                auto valWrapper = frame->getRefVar(name);
                frame->opStackPush(make_ptr(valWrapper));
                frame->instructionIndex++;
                break;
            }
        case BcOp::LoadReference:
            {
                ValWrapper* v = cast_val<ValWrapper>(frame->opStackPop());  // will raise exception if invalid type
                frame->opStackPush(v->ptr);
                frame->instructionIndex++;
                break;
            }
        case BcOp::AllocRecord:
            {
				frame->opStackPush(collector->allocate<Record>());
                frame->instructionIndex++;
                break;
            }
        case BcOp::FieldLoad:
            {
                Record* record = cast_val<Record>(frame->opStackPop());
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    tagptr_t val = NONE;
                    record->set(field, val, *collector);
                }
				frame->opStackPush(record->get(field));
                frame->instructionIndex++;
                break;
            }
        case BcOp::FieldStore:
            {
                tagptr_t ptr = frame->opStackPop();
                Record* record = cast_val<Record>(frame->opStackPop());
				string field = frame->getNameByIndex(inst.operand0.value());
                record->set(field, ptr, *collector);
                frame->instructionIndex++;
                break;
            }
        case BcOp::IndexLoad:
            {
                string index = *ptr_to_str(frame->opStackPop());
                Record* record = cast_val<Record>(frame->opStackPop());
                if (record->value.count(index) == 0) {
                    tagptr_t val = NONE;
                    record->set(index, val, *collector);
                }
				frame->opStackPush(record->get(index));
                frame->instructionIndex++;
                break;
            }
        case BcOp::IndexStore:
            // items are popped off in this order: value to store,
            // index
            // record
            {
				auto value = frame->opStackPop();
				string index = *ptr_to_str(frame->opStackPop());
				Record* record = cast_val<Record>(frame->opStackPop());
                record->set(index, value, *collector);
                frame->instructionIndex++;
                break;
            }
        case BcOp::AllocClosure:
            {
                // read num free vars, ref vars, and function off the stack
                int numFreeVars = inst.operand0.value();
                vector<ValWrapper*> refList;
                for (int i = 0; i < numFreeVars; i++) {
                    auto top = frame->opStackPop();
                    ValWrapper* value = cast_val<ValWrapper>(top);
                    refList.push_back(value);
                }
                Function* func = cast_val<Function>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on the stack for AllocClosure");
                }
                if (numFreeVars != func->free_vars_.size()) {
                    throw RuntimeException("expected " + to_string(func->free_vars_.size()) + " reference variables but got " + to_string(numFreeVars));
                }
                // push new closure onto the stack
                auto clos = collector->allocate(refList, func);
                frame->opStackPush(make_ptr(clos));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Call:
            {
                // read num arguments, argument values, and closure off the stack
                int numArgs = inst.operand0.value();
                vector<tagptr_t> argsList;
                for (int i = 0; i < numArgs; i++) {
                    auto top = frame->opStackPop();
                    if (!is_tagged(top)) {
                        cast_val<Constant>(top);  // will raise exception if not Constant
                    }
                    argsList.push_back(top);
                }
                reverse(argsList.begin(), argsList.end());

                Closure* closure = cast_val<Closure>(frame->opStackPop());
                frame->instructionIndex++;
                call(argsList, make_ptr(closure));
				frame = frames.back();
                break;
            }
        case BcOp::Return:
            {
                // take return val from top of stack & discard current frame
                auto returnVal = frame->opStackPeek();
                frames.pop_back();
                if (frames.empty()) {
                    finished = true;
                    return;
                }
                frame = frames.back();
                // push return val to top of new parent frame
                frame->opStackPush(returnVal);
                break;
            }
        case BcOp::Add:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = ptr_add(left, right);
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Sub:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_int(left) - get_int(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Mul:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_int(left) * get_int(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Div:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                if (get_int(right) == 0) {
                    throw IllegalArithmeticException("cannot divide by 0");
                }
                tagptr_t result = make_ptr(get_int(left) / get_int(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Neg:
            {
                auto top = frame->opStackPop();
                frame->opStackPush(make_ptr(-get_int(top)));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Gt:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_int(left) > get_int(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Geq:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_int(left) >= get_int(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Eq:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                frame->opStackPush(ptr_equals(left, right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::And:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_bool(left) && get_bool(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Or:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                tagptr_t result = make_ptr(get_bool(left) || get_bool(right));
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Not:
            {
                auto top = frame->opStackPop();
                frame->opStackPush(make_ptr(!get_bool(top)));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Goto:
            {
                int labelIndex = inst.operand0.value();
                frame->instructionIndex = frame->func->labels_[labelIndex];
                break;
            }
        case BcOp::If:
            {
                auto expr = frame->opStackPop();
                int labelIndex = inst.operand0.value();
                if (get_bool(expr)) {
                    frame->instructionIndex = frame->func->labels_[labelIndex];
                } else {
                    frame->instructionIndex++;
                }
                break;
            }
        case BcOp::Label:
            {
                frame->instructionIndex++;
                break;
            }
        case BcOp::Dup:
            {
                auto top = frame->opStackPeek();
                frame->opStackPush(top);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Swap:
            {
                auto top = frame->opStackPop();
                auto next = frame->opStackPop();
                frame->opStackPush(top);
                frame->opStackPush(next);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Pop:
            {
                frame->opStackPop();
                frame->instructionIndex++;
                break;
            }
        default:
            throw RuntimeException("should never get here - invalid instruction");
    }
	collector->gc();
    LOG("  inst index = " << frame->instructionIndex <<
            " || num insts = " << frame->numInstructions() <<
            " || opstack len = " << frame->opStack.size() <<
            " || frame num = " << frames.size());
    if (frames.size() == 1 && frame->instructionIndex == frame->numInstructions()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionIndex == frame->numInstructions()) {
        // last instruction of current function
        frames.pop_back();
        frame = frames.back();
        // push return val to top of new parent frame
        frame->opStackPush(NONE);
    }
    frame->checkLegalInstruction();
};

void Interpreter::run() {
    // catch exceptions thrown in helpers called from asm
    set_terminate([](){
        try {
            exception_ptr eptr = current_exception();
            if (eptr) {
                rethrow_exception(eptr);
            }
        } catch (InterpreterException& exception) {
            cout << exception.toString() << endl;
            exit(1);
        }
    });
    // runs program until termination (early return, end of statements)
    if (shouldCallAsm) {
        // create a closure objec to wrap the main function
        vector<ValWrapper*> emptyRefs;
        vector<tagptr_t> emptyArgs;
        Function* mainFunc = globalFrame->func;
        // TODO: this is not on the stack anywhere. it could get spontaneously
        // garbage-collected. How do we handle this?
        Closure* mainClosure = collector->allocate(emptyRefs, mainFunc);
        callAsm(emptyArgs, make_ptr(mainClosure));
    } else {
        // the global frame has already been created; you're ready to go
        while (!finished) {
            executeStep();
        }
    }
};

tagptr_t Interpreter::call(vector<tagptr_t> argsList, tagptr_t clos_ptr) {
    // this function takes care of figuring out whether to dispatch to
    // the vm or assembly
    Closure* clos = cast_val<Closure>(clos_ptr);
    if (argsList.size() != clos->func->parameter_count_) {
        throw RuntimeException("expected " + to_string(clos->func->parameter_count_) + " arguments, got " + to_string(argsList.size()));
    }
    if (shouldCallAsm) {
        // should still check for native functions
        NativeFunction* nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
        if (nativeFunc != NULL) {
            return callVM(argsList, clos_ptr);
        } else {
            return callAsm(argsList, clos_ptr);
        }
    } else {
        return callVM(argsList, clos_ptr);
    }
}

// Different call methods for vm execution and compilation to asm
tagptr_t Interpreter::callVM(vector<tagptr_t> argsList, tagptr_t clos_ptr) {
    // process local refs and local vars
    Closure* clos = cast_val<Closure>(clos_ptr);
    int numLocals = clos->func->local_vars_.size();
    int numRefs = clos->func->free_vars_.size();
    Frame* newFrame = collector->allocate<Frame>(clos->func);
    newFrame->collector = collector;
    for (int i = 0; i < numLocals; i++) {
        if (i < argsList.size()) {
            string name = clos->func->local_vars_[i];
            newFrame->setLocalVar(name, argsList[i]);
        } else {
            string name = clos->func->local_vars_[i];
            newFrame->setLocalVar(name, NONE);
        }
    }
    for (int i = 0; i < numRefs; i++) {
        string name = clos->func->free_vars_[i];
        newFrame->setRefVar(name, make_ptr(clos->refs[i]));
    }
    NativeFunction* nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
    if (nativeFunc != NULL) {
        tagptr_t val = nativeFunc->evalNativeFunction(*newFrame, *collector);
        frames.back()->opStackPush(val);
		return val;
    } else if (newFrame->numInstructions() != 0) {
        frames.push_back(newFrame);
        // TODO: do we need to return something here?
    } else {
        tagptr_t returnVal = NONE;
        frames.back()->opStackPush(returnVal);
		return returnVal;
    }
}

tagptr_t Interpreter::callAsm(vector<tagptr_t> argsList, tagptr_t clos_ptr) {
    Interpreter* self = this;
    Closure* clos = cast_val<Closure>(clos_ptr);
    if (!(clos->func->mcf)) {
        // convert the bc function to the ir
        IrCompiler irc = IrCompiler(clos->func, self);
        IrFunc irf = irc.toIr();
        //TODO make optimization toggleable?
        RegOpt reg = RegOpt();
		irf.temp_count = reg.optimize(&irf);
        // convert the ir to assembly
        IrInterpreter iri = IrInterpreter(&irf, self, irc.isLocalRef);
        x64asm::Function asmFunc = iri.run();
        // create a MachineCodeFunction object
        clos->func->mcf = new MachineCodeFunction(2, asmFunc);
        clos->func->mcf->compile();
        LOG("done compiling mcf");
    } // else, already compiled and should be there! 
    // put the args in an array
    tagptr_t* argsArray = new tagptr_t[argsList.size()];
    for (int i = 0; i < argsList.size(); i++) {
        argsArray[i] = argsList[i];
    }
    // put the refs in an array 
    vector<ValWrapper*> refs = clos->refs;
    tagptr_t* refsArray = new tagptr_t[refs.size()];
    for (int i = 0; i < refs.size(); i++) {
        refsArray[i] = make_ptr(refs[i]);
    }
    vector<tagptr_t*> mcfArgs = {argsArray, refsArray};
    tagptr_t result = clos->func->mcf->call(mcfArgs);
    LOG("done calling mcf");
    return result;
}

void Interpreter::storeGlobal(string name, tagptr_t val) {
    globalFrame->setLocalVar(name, val);
};

tagptr_t Interpreter::loadGlobal(string name) {
    return globalFrame->getLocalVar(name);
};
