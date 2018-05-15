#include "opt_tag_ptr.h"

bool check_tag(tagptr_t ptr, int tag) {
    return (ptr & ALL_TAG) == tag;
}
bool is_tagged(tagptr_t ptr) {
    // TODO: think about gc for strings?
    return !check_tag(ptr, PTR_TAG);
}
Value* get_val(tagptr_t ptr) {
    if (is_tagged(ptr)) {
        throw IllegalCastException("expected Value, got " + get_type(ptr));
    }
    return (Value*) ptr;
}
string get_type(tagptr_t ptr) {
    if (check_tag(ptr, INT_TAG)) {
        return "int";
    }
    if (check_tag(ptr, BOOL_TAG)) {
        return "bool";
    }
    if (check_tag(ptr, STR_TAG)) {
        return "string";
    }
    Value* v = get_val(ptr);
    return v->type();
}

tagptr_t make_ptr(int val) {
    tagptr_t result = (val << 2) | INT_TAG;
    LOG("  TAGPTR INT: " << hex << result << " // " << val);
    return result;
}
tagptr_t make_ptr(bool val) {
    tagptr_t result = (val << 2) | BOOL_TAG;
    LOG("  TAGPTR BOOL: " << hex << result << " // " << val);
    return result;
}
tagptr_t make_ptr(string* val) {
    tagptr_t result = ((tagptr_t) val << 2) | STR_TAG;
    LOG("  TAGPTR STR: " << hex << result << " // " << val);
    return result;
}
tagptr_t make_ptr(Constant* val) {
    tagptr_t result = (tagptr_t) val;
    LOG("  TAGPTR CONSTANT: " << hex << result << " // " << val->type());
    return result;
}
tagptr_t make_ptr(Function* val) {
    tagptr_t result = (tagptr_t) val;
    LOG("  TAGPTR FUNCTION: " << hex << result << " // " << val);
    return result;
}
tagptr_t make_ptr(ValWrapper* val) {
    tagptr_t result = (tagptr_t) val;
    LOG("  TAGPTR VALWRAPPER: " << hex << result << " // " << val);
    return result;
}

int get_int(tagptr_t ptr) {
    if (!check_tag(ptr, INT_TAG)) {
        throw IllegalCastException("expected int, got " + get_type(ptr));
    }
    return (ptr & CLEAR_TAG) >> 2;  // ptr needs to be signed for an arithmetic shift
}
bool get_bool(tagptr_t ptr) {
    if (!check_tag(ptr, BOOL_TAG)) {
        throw IllegalCastException("expected bool, got " + get_type(ptr));
    }
    return (ptr & CLEAR_TAG) >> 2;  // ptr needs to be signed for an arithmetic shift
}
string* get_str(tagptr_t ptr) {
    if (!check_tag(ptr, STR_TAG)) {
        throw IllegalCastException("expected string, got " + get_type(ptr));
    }
    return (string*) ((ptr & CLEAR_TAG) >> 2);
}
Collectable* get_collectable(tagptr_t ptr) {
    if (is_tagged(ptr)) {
        throw IllegalCastException("expected Value, got " + get_type(ptr));
    }
    return (Collectable*) ptr;
}

template<typename T>
T* cast_val(tagptr_t ptr) {
    Value* v = get_val(ptr);
    T* result = dynamic_cast<T*>(v);
    if (result == NULL) {
        // TODO: change to IllegalCast?
        throw RuntimeException("expected " + T::typeS + ", got " + v->type());
    }
    return result;
}
string ptr_to_str(tagptr_t ptr) {
    if (check_tag(ptr, INT_TAG)) {
        return to_string(get_int(ptr));
    }
    if (check_tag(ptr, BOOL_TAG)) {
        bool val = get_bool(ptr);
        if (val) {
            return string("true");
        } else {
            return string("false");
        }
    }
    if (check_tag(ptr, STR_TAG)) {
        string* val = get_str(ptr);
        string replaced = string(*val);
        auto pos = replaced.find("\\");
        while (pos != string::npos) {
            if (replaced.at(pos + 1) == 'n') {
                replaced.replace(pos, 2, "\n");
            } else if (replaced.at(pos + 1) == 't') {
                replaced.replace(pos, 2, "\t");
            } else if (replaced.at(pos + 1) == '\\') {
                replaced.replace(pos, 2, "\\");
            } else if (replaced.at(pos + 1) == '"') {
                replaced.replace(pos, 2, "\"");
            }
            pos = replaced.find("\\", pos + 1);
        }
        return replaced;
    }
    auto c = get_val(ptr);
    return c->toString();
}
tagptr_t ptr_equals(tagptr_t left, tagptr_t right) {
    if ((left & ALL_TAG) == (right & ALL_TAG)) {  // same tag
        if (check_tag(left, INT_TAG) || check_tag(left, BOOL_TAG)) {
            return make_ptr(left == right);
        }
        if (check_tag(left, STR_TAG)) {
            return make_ptr(get_str(left)->compare(*get_str(right)) == 0);
        }
        Value* leftV = get_val(left);
        Value* rightV = get_val(right);
        return make_ptr(leftV->equals(rightV));
    } else {
        return make_ptr(false);
    }
}
tagptr_t ptr_add(tagptr_t left, tagptr_t right) {
    // try adding strings if left or right is a string
    if (check_tag(left, STR_TAG) || check_tag(right, STR_TAG)) {
        string* result = new string(ptr_to_str(left) + ptr_to_str(right));
        return make_ptr(result);
    }
    // try adding integers if left is an int
    int leftI = get_int(left);
    int rightI = get_int(right);
    return make_ptr(leftI + rightI);
}

// same template issue as in gc.cpp
// can't put the implementation of cast_val in header, because that would cause
// a circular dependency
template Constant* cast_val<Constant>(tagptr_t);
template ValWrapper* cast_val<ValWrapper>(tagptr_t);
template Record* cast_val<Record>(tagptr_t);
template Function* cast_val<Function>(tagptr_t);
template Closure* cast_val<Closure>(tagptr_t);
template None* cast_val<None>(tagptr_t);
