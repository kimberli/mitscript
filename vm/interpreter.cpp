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

void Interpreter::addToRootset(Collectable* obj) {
    rootset->push_back(obj);
}

void Interpreter::removeFromRootset(Collectable* obj) {
    rootset->remove(obj);
}

Interpreter::Interpreter(Function* mainFunc, int maxmem, bool callAsm) {
    // initialize the garbage collector
    // note that mainFunc is not included in the gc's allocated list because
    // we never have to deallocate it
    rootset = new list<Collectable*>();
    collector = new CollectedHeap(maxmem, mainFunc->getSize(), rootset);

    // initialize a static none
    NONE = new None();

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
    Frame* frame = collector->allocate<Frame, Function*>(mainFunc);
    frame->collector = collector;
    globalFrame = frame;
    frames.push_back(frame);
    addToRootset(frame);
    finished = false;
    shouldCallAsm = callAsm;

    // set up native functions at the beginning of functions array
	vector<Function*> functions_;
    vector<Constant*> constants_;
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
    LOG("executing instruction " + to_string(frame->instructionIndex));
    LOG("from frame " + to_string(frames.size()));
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
                frame->opStackPush(func);
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
                Constant* value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreLocal");
                }
                frame->setLocalVar(name, value);
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
                frame->opStackPush(valWrapper);
                frame->instructionIndex++;
                break;
            }
        case BcOp::LoadReference:
            {
                ValWrapper* valWrapper = dynamic_cast<ValWrapper*>(frame->opStackPop());
                if (valWrapper == NULL) {
                    throw RuntimeException("expected ValWrapper on the stack for LoadReference");
                }
                frame->opStackPush(valWrapper->ptr);
                frame->instructionIndex++;
                break;
            }
        case BcOp::StoreReference:
            {
                Constant* value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreReference");
                }
                ValWrapper* valWrapper = dynamic_cast<ValWrapper*>(frame->opStackPop());
                if (valWrapper == NULL) {
                    throw RuntimeException("expected ValWrapper on the stack for StoreReference");
                }
				*valWrapper->ptr = *value;
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
				Record* record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    Value* val = collector->allocate<None>();
                    record->set(field, val, *collector);
                }
				frame->opStackPush(record->get(field));
                frame->instructionIndex++;
                break;
            }
        case BcOp::FieldStore:
            {
				Constant* value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for FieldStore");
                }
				Record* record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                record->set(field, value, *collector);
                frame->instructionIndex++;
                break;
            }
        case BcOp::IndexLoad:
            {
				string index = frame->opStackPop()->toString();
				Record* record = frame->opStackPop()->cast<Record>();
                if (record->value.count(index) == 0) {
                    Value* val = collector->allocate<None>();
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
				string index = frame->opStackPop()->toString();
				Record* record = frame->opStackPop()->cast<Record>();
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
                    ValWrapper* value = dynamic_cast<ValWrapper*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected ValWrapper on the stack for AllocClosure");
                    }
                    refList.push_back(value);
                }
                Function* func = dynamic_cast<Function*>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on the stack for AllocClosure");
                }
                if (numFreeVars != func->free_vars_.size()) {
                    throw RuntimeException("expected " + to_string(func->free_vars_.size()) + " reference variables but got " + to_string(numFreeVars));
                }
                // push new closure onto the stack
                frame->opStackPush(collector->allocate<Closure>(refList, func));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Call:
            {
                // read num arguments, argument values, and closure off the stack
                int numArgs = inst.operand0.value();
                vector<Constant*> argsList;
                for (int i = 0; i < numArgs; i++) {
                    auto top = frame->opStackPop();
                    Constant* value = dynamic_cast<Constant*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected Constant on the stack for Call");
                    }
                    argsList.push_back(value);
                }
                reverse(argsList.begin(), argsList.end());

                Value* closure = frame->opStackPop();
                frame->instructionIndex++;
                call(argsList, closure);
				frame = frames.back();
                break;
            }
        case BcOp::Return:
            {
                // take return val from top of stack & discard current frame
                auto returnVal = frame->opStackPeek();
                frames.pop_back();
                removeFromRootset(frame);
                if (frames.empty()) {
                    finished = true;
                    return;
                }
                frame = frames.back();
                // push return val to top of new parent frame
                frame->opStackPush(returnVal);
                //frame->instructionIndex++;
                break;
            }
        case BcOp::Add:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                Value* result = add(left, right);
                frame->opStackPush(result);
                frame->instructionIndex++;
                break;
            }
        case BcOp::Sub:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(left - right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Mul:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                       collector->allocate<Integer>(left * right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Div:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                if (right == 0) {
                    throw IllegalArithmeticException("cannot divide by 0");
                }
                frame->opStackPush(
                        collector->allocate<Integer>(left / right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Neg:
            {
                int top = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(-top));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Gt:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left > right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Geq:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left >= right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Eq:
            {
                Value* right = frame->opStackPop();
                auto left = frame->opStackPop();
                frame->opStackPush(
                        collector->allocate<Boolean>(left->equals(right)));
                frame->instructionIndex++;
                break;
            }
        case BcOp::And:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left && right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Or:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left || right));
                frame->instructionIndex++;
                break;
            }
        case BcOp::Not:
            {
                bool top = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(!top));
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
                auto e = frame->opStackPop()->cast<Boolean>();
                int labelIndex = inst.operand0.value();
                if (e->value) {
                    frame->instructionIndex = frame->func->labels_[labelIndex];
                } else {
                    frame->instructionIndex++;
                }
                break;
            }
        case BcOp::StartWhile:
            {
                frame->instructionIndex++;
                break;
            }
        case BcOp::EndWhile:
            {
                frame->instructionIndex++;
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
    LOG("instructionIndex = " + to_string(frame->instructionIndex));
    LOG("numInstructions = " + to_string(frame->numInstructions()));
    LOG("frames.size = " + to_string(frames.size()));
    if (frames.size() == 1 && frame->instructionIndex == frame->numInstructions()) {
        // last instruction of the whole program
        finished = true;
        return;
    }
    if (frame->instructionIndex == frame->numInstructions()) {
        // last instruction of current function
        Value* returnVal = collector->allocate<None>();
        frames.pop_back();
        removeFromRootset(frame);
        frame = frames.back();
        // push return val to top of new parent frame
        frame->opStackPush(returnVal);
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
        vector<Constant*> emptyArgs;
        Function* mainFunc = globalFrame->func;
        // TODO: this is not on the stack anywhere. it could get spontaneously
        // garbage-collected. How do we handle this?
        Closure* mainClosure = collector->allocate<Closure>(emptyRefs, mainFunc);
        callAsm(emptyArgs, mainClosure);
    } else {
        // the global frame has already been created; you're ready to go
        while (!finished) {
            executeStep();
        }
    }
};

Value* Interpreter::call(vector<Constant*> argsList, Value* closure) {
    // this function takes care of figuring out whether to dispatch to
    // the vm or assembly
    Closure* clos = dynamic_cast<Closure*>(closure);
    if (clos == NULL) {
        throw RuntimeException("expected Closure on operand stack for function call");
    }
    if (argsList.size() != clos->func->parameter_count_) {
        throw RuntimeException("expected " + to_string(clos->func->parameter_count_) + " arguments, got " + to_string(argsList.size()));
    }
    if (shouldCallAsm) {
        // should still check for native functions
        NativeFunction* nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
        if (nativeFunc != NULL) {
            return callVM(argsList, clos);
        } else {
            return callAsm(argsList, clos);
        }
    } else {
        return callVM(argsList, clos);
    }
}

// Different call methods for vm execution and compilation to asm
Value* Interpreter::callVM(vector<Constant*> argsList, Closure* clos) {
    // process local refs and local vars
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
            newFrame->setLocalVar(name, collector->allocate<None>());
        }
    }
    for (int i = 0; i < numRefs; i++) {
        string name = clos->func->free_vars_[i];
        newFrame->setRefVar(name, clos->refs[i]);
    }
    NativeFunction* nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
    if (nativeFunc != NULL) {
        Constant* val = nativeFunc->evalNativeFunction(*newFrame, *collector);
        frames.back()->opStackPush(val);
		return val;
    } else if (newFrame->numInstructions() != 0) {
        frames.push_back(newFrame);
        addToRootset(newFrame);
        // TODO: do we need to return something here?
    } else {
        Value* returnVal = collector->allocate<None>();
        frames.back()->opStackPush(returnVal);
		return returnVal;
    }
}

void Interpreter::setFrameCollectables(int numTemps, Collectable** collectables) {
    Frame* frame = frames.back();
    for (int i = 0; i < numTemps; i++) {
        Collectable* temp = collectables[i];
        frame->collectables.push_back(temp);
    }
}

Value* Interpreter::callAsm(vector<Constant*> argsList, Closure* clos) {
    Interpreter* self = this;
    Frame* newFrame = collector->allocate<Frame>(clos->func);
    newFrame->collector = collector;
    frames.push_back(newFrame);
    addToRootset(newFrame);
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
    Value** argsArray = new Value*[argsList.size()];
    for (int i = 0; i < argsList.size(); i++) {
        argsArray[i] = argsList[i];
    }
    // put the refs in an array 
    vector<ValWrapper*> refs = clos->refs;
    Value** refsArray = new Value*[refs.size()];
    for (int i = 0; i < refs.size(); i++) {
        refsArray[i] = refs[i];
    }
    vector<Value**> mcfArgs = {argsArray, refsArray};
    Value* result = clos->func->mcf->call(mcfArgs);

    // clear things we added to rootset
    removeFromRootset(newFrame);
    frames.pop_back();

    LOG("done calling mcf");
    return result;
}

// Asm helpers
Value* Interpreter::add(Value* left, Value* right) {
    // try adding strings if left or right is a string
    String* leftStr = dynamic_cast<String*>(left);
    if (leftStr != NULL) {
        Value* ret = collector->allocate<String>(leftStr->value + right->toString());
        return ret;
    }
    String* rightStr = dynamic_cast<String*>(right);
    if (rightStr != NULL) {
        Value* ret = collector->allocate<String>(left->toString() + rightStr->value);
        return ret;
    }
    // try adding integers if left is an int
    int leftI = left->cast<Integer>()->value;
    int rightI = right->cast<Integer>()->value;
    Value* ret = collector->allocate<Integer>(leftI + rightI);
    return ret;
};

void Interpreter::storeGlobal(string name, Value* val) {
    Constant* value = dynamic_cast<Constant*>(val);
    if (value == NULL) {
        throw RuntimeException("expected Constant on the stack for StoreGlobal");
    }
    globalFrame->setLocalVar(name, value);
};

Value* Interpreter::loadGlobal(string name) {
    return globalFrame->getLocalVar(name);
};
