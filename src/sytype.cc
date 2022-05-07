#include "sytype.h"

#include "utils.h"

// let the C++ compiler do the magic
template <typename LHS_TYPE, typename RHS_TYPE>
static auto basicTypeAlu(LHS_TYPE lhs_val, RHS_TYPE rhs_val, SyAstType op)
    -> decltype(lhs_val + rhs_val) {
    switch (op) {
        case SyAstType::ALU_ADD:
            return lhs_val + rhs_val;
            break;
        case SyAstType::ALU_SUB:
            return lhs_val - rhs_val;
            break;
        case SyAstType::ALU_MUL:
            return lhs_val * rhs_val;
            break;
        case SyAstType::ALU_DIV:
            return lhs_val / rhs_val;
            break;
        case SyAstType::ALU_MOD:
            return ((int)lhs_val) % ((int)rhs_val);
            break;
        case SyAstType::EQ:
            return lhs_val == rhs_val;
            break;
        case SyAstType::NEQ:
            return lhs_val != rhs_val;
            break;
        case SyAstType::LNE:
            return lhs_val < rhs_val;
            break;
        case SyAstType::LE:
            return lhs_val <= rhs_val;
            break;
        case SyAstType::GNE:
            return lhs_val > rhs_val;
            break;
        case SyAstType::GE:
            return lhs_val >= rhs_val;
            break;
        case SyAstType::LOGIC_AND:
            return lhs_val && rhs_val;
            break;
        case SyAstType::LOGIC_OR:
            return lhs_val || rhs_val;
            break;
        default:
            DEBUG_ASSERT_NOT_REACH
            break;
    }
}

Value ValueAlu(Value lhs_val, SyAstType lhs_type, Value rhs_val,
               SyAstType rhs_type, SyAstType op) {
    // this code is not good :(
    if (lhs_type == SyAstType::VAL_TYPE_INT &&
        rhs_type == SyAstType::VAL_TYPE_INT) {
        return Value(basicTypeAlu(lhs_val.i32, rhs_val.i32, op));
    } else if (lhs_type == SyAstType::VAL_TYPE_FLOAT &&
               rhs_type == SyAstType::VAL_TYPE_FLOAT) {
        return Value(basicTypeAlu(lhs_val.f32, rhs_val.f32, op));
    } else if (lhs_type == SyAstType::VAL_TYPE_INT &&
               rhs_type == SyAstType::VAL_TYPE_FLOAT) {
        return Value(basicTypeAlu(lhs_val.i32, rhs_val.f32, op));
    } else if (lhs_type == SyAstType::VAL_TYPE_FLOAT &&
               rhs_type == SyAstType::VAL_TYPE_INT) {
        return Value(basicTypeAlu(lhs_val.f32, rhs_val.i32, op));
    } else {
        DEBUG_ASSERT_NOT_REACH
        return Value(0);
    }
}

SyAstType valTypeToValArrayType(SyAstType type) {
    switch (type) {
        case SyAstType::VAL_TYPE_INT:
            return SyAstType::VAL_TYPE_INT_ARRAY;
        case SyAstType::VAL_TYPE_FLOAT:
            return SyAstType::VAL_TYPE_FLOAT_ARRAY;
        default:
            DEBUG_ASSERT_NOT_REACH
            return SyAstType::END_OF_ENUM;
    }
}

SyAstType valArrayTypeToValType(SyAstType type) {
    switch (type) {
        case SyAstType::VAL_TYPE_INT_ARRAY:
            return SyAstType::VAL_TYPE_INT;
        case SyAstType::VAL_TYPE_FLOAT_ARRAY:
            return SyAstType::VAL_TYPE_FLOAT;
        default:
            DEBUG_ASSERT_NOT_REACH
            return SyAstType::END_OF_ENUM;
    }
}