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

Interpreter::Interpreter(vptr<Function> mainFunc, int maxmem) {
    // initialize the garbage collector
    // note that mainFunc is not included in the gc's allocated list because
    // we never have to deallocate it
    collector = new CollectedHeap(maxmem, mainFunc->getSize(), &frames);

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
    fptr frame = collector->allocate<Frame, vptr<Function>>(mainFunc);
    frame->collector = collector;
    globalFrame = frame;
    frames.push_back(frame);
    finished = false;

    // set up native functions at the beginning of functions array
	vector<vptr<Function>> functions_;
    vector<vptr<Constant>> constants_;
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
    fptr frame = frames.back();
    BcInstruction& inst = frame->getCurrInstruction();
    LOG("executing instruction " + to_string(frame->instructionIndex));
    LOG("from frame" + to_string(frames.size()));
    switch (inst.operation) {
        case BcOp::LoadConst:
            {
                auto constant = frame->getConstantByIndex(inst.operand0.value());
                frame->opStackPush(constant);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::LoadFunc:
            {
                auto func = frame->getFunctionByIndex(inst.operand0.value());
                frame->opStackPush(func);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::LoadLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                frame->opStackPush(frame->getLocalVar(name));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::StoreLocal:
            {
                string name = frame->getLocalByIndex(inst.operand0.value());
                vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreLocal");
                }
                frame->setLocalVar(name, value);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::LoadGlobal:
            {
                int index = inst.operand0.value();
                string name = frame->getNameByIndex(index);
                frame->opStackPush(loadGlobal(name));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::StoreGlobal:
            {
                int index = inst.operand0.value();
                string name = frame->getNameByIndex(index);
                storeGlobal(name, frame->opStackPop());
                frame->instructionIndex ++;
                break;
            }
        case BcOp::PushReference:
            {
                string name = frame->getRefByIndex(inst.operand0.value());
                auto valWrapper = frame->getRefVar(name);
                frame->opStackPush(valWrapper);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::LoadReference:
            {
                vptr<ValWrapper> valWrapper = dynamic_cast<ValWrapper*>(frame->opStackPop());
                if (valWrapper == NULL) {
                    throw RuntimeException("expected ValWrapper on the stack for LoadReference");
                }
                frame->opStackPush(valWrapper->ptr);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::StoreReference:
            {
                vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for StoreReference");
                }
                vptr<ValWrapper> valWrapper = dynamic_cast<ValWrapper*>(frame->opStackPop());
                if (valWrapper == NULL) {
                    throw RuntimeException("expected ValWrapper on the stack for StoreReference");
                }
				*valWrapper->ptr = *value;
                frame->instructionIndex ++;
                break;
            }
        case BcOp::AllocRecord:
            {
				frame->opStackPush(collector->allocate<Record>());
                frame->instructionIndex ++;
                break;
            }
        case BcOp::FieldLoad:
            {
				vptr<Record> record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                if (record->value.count(field) == 0) {
                    vptr<Value> val = collector->allocate<None>();
                    record->set(field, val, *collector);
                }
				frame->opStackPush(record->get(field));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::FieldStore:
            {
				vptr<Constant> value = dynamic_cast<Constant*>(frame->opStackPop());
                if (value == NULL) {
                    throw RuntimeException("expected Constant on the stack for FieldStore");
                }
				vptr<Record> record = frame->opStackPop()->cast<Record>();
				string field = frame->getNameByIndex(inst.operand0.value());
                record->set(field, value, *collector);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::IndexLoad:
            {
				string index = frame->opStackPop()->toString();
				vptr<Record> record = frame->opStackPop()->cast<Record>();
                if (record->value.count(index) == 0) {
                    vptr<Value> val = collector->allocate<None>();
                    record->set(index, val, *collector);
                }
				frame->opStackPush(record->get(index));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::IndexStore:
            // items are popped off in this order: value to store,
            // index
            // record
            {
				auto value = frame->opStackPop();
				string index = frame->opStackPop()->toString();
				vptr<Record> record = frame->opStackPop()->cast<Record>();
                record->set(index, value, *collector);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::AllocClosure:
            {
                // read num free vars, ref vars, and function off the stack
                int numFreeVars = inst.operand0.value();
                vector<vptr<ValWrapper>> refList;
                for (int i = 0; i < numFreeVars; i++) {
                    auto top = frame->opStackPop();
                    vptr<ValWrapper> value = dynamic_cast<ValWrapper*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected ValWrapper on the stack for AllocClosure");
                    }
                    refList.push_back(value);
                }
                vptr<Function> func = dynamic_cast<Function*>(frame->opStackPop());
                if (func == NULL) {
                    throw RuntimeException("expected Function on the stack for AllocClosure");
                }
                if (numFreeVars != func->free_vars_.size()) {
                    throw RuntimeException("expected " + to_string(func->free_vars_.size()) + " reference variables but got " + to_string(numFreeVars));
                }
                // push new closure onto the stack
                frame->opStackPush(collector->allocate<Closure>(refList, func));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Call:
            {
                // read num arguments, argument values, and closure off the stack
                int numArgs = inst.operand0.value();
                vector<vptr<Constant>> argsList;
                for (int i = 0; i < numArgs; i++) {
                    auto top = frame->opStackPop();
                    vptr<Constant> value = dynamic_cast<Constant*>(top);
                    if (value == NULL) {
                        throw RuntimeException("expected Constant on the stack for Call");
                    }
                    argsList.push_back(value);
                }
                reverse(argsList.begin(), argsList.end());

                vptr<Value> closure = frame->opStackPop();
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
                if (frames.empty()) {
                    finished = true;
                    return;
                }
                frame = frames.back();
                // push return val to top of new parent frame
                frame->opStackPush(returnVal);
                //frame->instructionIndex ++;
                break;
            }
        case BcOp::Add:
            {
                auto right = frame->opStackPop();
                auto left = frame->opStackPop();
                vptr<Value> result = add(left, right);
                frame->opStackPush(result);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Sub:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(left - right));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Mul:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                       collector->allocate<Integer>(left * right));
                frame->instructionIndex ++;
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
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Neg:
            {
                int top = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Integer>(-top));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Gt:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left > right));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Geq:
            {
                int right = frame->opStackPop()->cast<Integer>()->value;
                int left = frame->opStackPop()->cast<Integer>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left >= right));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Eq:
            {
                vptr<Value> right = frame->opStackPop();
                auto left = frame->opStackPop();
                frame->opStackPush(
                        collector->allocate<Boolean>(left->equals(right)));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::And:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left && right));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Or:
            {
                bool right = frame->opStackPop()->cast<Boolean>()->value;
                bool left = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(left || right));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Not:
            {
                bool top = frame->opStackPop()->cast<Boolean>()->value;
                frame->opStackPush(
                        collector->allocate<Boolean>(!top));
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Goto:
            {
                frame->instructionIndex += inst.operand0.value();
                break;
            }
        case BcOp::If:
            {
                auto e = frame->opStackPop()->cast<Boolean>();
                if (e->value) {
                    frame->instructionIndex += inst.operand0.value();
                } else {
                    frame->instructionIndex ++;
                }
                break;
            }
        case BcOp::Dup:
            {
                auto top = frame->opStackPeek();
                frame->opStackPush(top);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Swap:
            {
                auto top = frame->opStackPop();
                auto next = frame->opStackPop();
                frame->opStackPush(top);
                frame->opStackPush(next);
                frame->instructionIndex ++;
                break;
            }
        case BcOp::Pop:
            {
                frame->opStackPop();
                frame->instructionIndex ++;
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
        vptr<Value> returnVal = collector->allocate<None>();
        frames.pop_back();
        frame = frames.back();
        // push return val to top of new parent frame
        frame->opStackPush(returnVal);
    }
    frame->checkLegalInstruction();
};

void Interpreter::run() {
    // runs program until termination (early return, end of statements)
    if (shouldCallAsm) {
        // create a closure objec to wrap the main function 
        vector<vptr<ValWrapper>> emptyRefs;
        vector<vptr<Constant>> emptyArgs;
        vptr<Function> mainFunc = globalFrame->func;
        // TODO: this is not on the stack anywhere. it could get spontaneously
        // garbage-collected. How do we handle this? 
        vptr<Closure> mainClosure = collector->allocate<Closure>(emptyRefs, mainFunc);
        callAsm(emptyArgs, mainClosure);
    } else {
        // the global frame has already been created; you're ready to go 
        while (!finished) {
            executeStep();
        }
    }
};

vptr<Value> Interpreter::call(vector<vptr<Constant>> argsList, vptr<Value> closure) {
    // this function takes care of figuring out whether to dispatch to
    // the vm or assembly
    vptr<Closure> clos = dynamic_cast<Closure*>(closure);
    if (clos == NULL) {
        throw RuntimeException("expected Closure on operand stack for function call");
    }
    if (argsList.size() != clos->func->parameter_count_) {
        throw RuntimeException("expected " + to_string(clos->func->parameter_count_) + " arguments, got " + to_string(argsList.size()));
    }
    if (shouldCallAsm) {
        callAsm(argsList, clos);
    } else {
        callVM(argsList, clos);
    }
}

// Different call methods for vm execution and compilation to asm
vptr<Value> Interpreter::callVM(vector<vptr<Constant>> argsList, vptr<Closure> clos) {
    // process local refs and local vars
    int numLocals = clos->func->local_vars_.size();
    int numRefs = clos->func->free_vars_.size();
    fptr newFrame = collector->allocate<Frame>(clos->func);
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
    vptr<NativeFunction> nativeFunc = dynamic_cast<NativeFunction*>(clos->func);
    if (nativeFunc != NULL) {
        vptr<Constant> val = nativeFunc->evalNativeFunction(*newFrame, *collector);
        frames.back()->opStackPush(val);
    } else if (newFrame->numInstructions() != 0) {
        frames.push_back(newFrame);
    } else {
        vptr<Value> returnVal = collector->allocate<None>();
        frames.back()->opStackPush(returnVal);
    }
}

vptr<Value> Interpreter::callAsm(vector<vptr<Constant>> argsList, vptr<Closure> clos) {
    // convert the bc function to the ir
    IrCompiler irc = IrCompiler(clos->func, this);
    IrFunc irf = irc.toIr();
    // convert the ir to assembly
    IrInterpreter iri = IrInterpreter(&irf, this);
    x64asm::Function asmFunc = iri.run();
    // create a MachineCodeFunction object
    MachineCodeFunction mcf = MachineCodeFunction(clos->func->parameter_count_, asmFunc);
    // call mcf.call(args) and it will run properly in assembly presumably
    // TODO: be more intelligent about how you're compiling stuff
    // TODO: this obeys NO conventions about passing refs
    mcf.compile();
    // we need to pass the args as Values
    vector<vptr<Value>> argsVals (argsList.begin(), argsList.end());
    vptr<Value> result = mcf.call(argsVals);
    return result;
}

// Asm helpers
vptr<Value> Interpreter::add(vptr<Value> left, vptr<Value> right) {
    // try adding strings if left or right is a string
    vptr<String> leftStr = dynamic_cast<String*>(left);
    if (leftStr != NULL) {
        vptr<Value> ret = collector->allocate<String>(leftStr->value + right->toString());
        return ret;
    }
    vptr<String> rightStr = dynamic_cast<String*>(right);
    if (rightStr != NULL) {
        vptr<Value> ret = collector->allocate<String>(left->toString() + rightStr->value);
        return ret;
    }
    // try adding integers if left is an int
    vptr<Integer> leftInt = dynamic_cast<Integer*>(left);
    if (leftInt != NULL) {
        int leftI = leftInt->value;
        int rightI = right->cast<Integer>()->value;
        vptr<Value> ret = collector->allocate<Integer>(leftI + rightI);
        return ret;
    }
};

void Interpreter::storeGlobal(string name, vptr<Value> val) {
    vptr<Constant> value = dynamic_cast<Constant*>(val);
    if (value == NULL) {
        throw RuntimeException("expected Constant on the stack for StoreGlobal");
    }
    globalFrame->setLocalVar(name, value);
};

vptr<Value> Interpreter::loadGlobal(string name) {
    return globalFrame->getLocalVar(name);
};
