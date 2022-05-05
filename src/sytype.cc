#include "sytype.h"

#include "utils.h"

template <typename T>
static T basicTypeAlu(T lhs_val, T rhs_val, SyAstType op) {
    T ans;
    switch (op) {
        case SyAstType::ALU_ADD:
            ans = lhs_val + rhs_val;
            break;
        case SyAstType::ALU_SUB:
            ans = lhs_val - rhs_val;
            break;
        case SyAstType::ALU_MUL:
            ans = lhs_val * rhs_val;
            break;
        case SyAstType::ALU_DIV:
            ans = lhs_val / rhs_val;
            break;
        case SyAstType::ALU_MOD:
            ans = lhs_val % rhs_val;
            break;
        case SyAstType::EQ:
            ans = lhs_val == rhs_val;
            break;
        case SyAstType::NEQ:
            ans = lhs_val != rhs_val;
            break;
        case SyAstType::LNE:
            ans = lhs_val < rhs_val;
            break;
        case SyAstType::LE:
            ans = lhs_val <= rhs_val;
            break;
        case SyAstType::GNE:
            ans = lhs_val > rhs_val;
            break;
        case SyAstType::GE:
            ans = lhs_val >= rhs_val;
            break;
        case SyAstType::LOGIC_AND:
            ans = lhs_val && rhs_val;
            break;
        case SyAstType::LOGIC_OR:
            ans = lhs_val || rhs_val;
            break;
        default:
            DEBUG_ASSERT_NOT_REACH
            break;
    }
    return ans;
}

namespace SYTYPEHELPER {

template <SyAstType, SyAstType>
Value ValueAluHelper(Value lhs_val, Value rhs_val, SyAstType op) {
    // unimplemented
    DEBUG_ASSERT_NOT_REACH
}

template <>
Value ValueAluHelper<SyAstType::VAL_TYPE_INT, SyAstType::VAL_TYPE_INT>(
    Value lhs, Value rhs, SyAstType op) {
    return Value(basicTypeAlu(lhs.i32, rhs.i32, op));
}

}  // namespace SYTYPEHELPER

Value ValueAlu(Value lhs_val, SyAstType lhs_type, Value rhs_val,
               SyAstType rhs_type, SyAstType op) {
    if (lhs_type == SyAstType::VAL_TYPE_INT &&
        rhs_type == SyAstType::VAL_TYPE_INT) {
        return SYTYPEHELPER::ValueAluHelper<SyAstType::VAL_TYPE_INT,
                                            SyAstType::VAL_TYPE_INT>(
            lhs_val, rhs_val, op);
    } else {
        DEBUG_ASSERT_NOT_REACH
    }
}
