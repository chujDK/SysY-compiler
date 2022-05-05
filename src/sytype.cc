#include "sytype.h"

#include "utils.h"

template <>
Value ValueAlu<SyAstType::VAL_TYPE_INT, SyAstType::VAL_TYPE_INT>(Value lhs,
                                                                 Value rhs,
                                                                 SyAstType op) {
    int ans;
    int lhs_val = lhs.i32;
    int rhs_val = rhs.i32;
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
    return Value(ans);
}
