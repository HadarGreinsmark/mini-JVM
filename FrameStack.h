#ifndef VM_FRAMESTACK_H
#define VM_FRAMESTACK_H

#include <cstdint>
#include <cassert>
#include <vector>
#include <stack>
#include "class.h"

struct Frame {
    std::vector<int32_t> local_vars;
    std::stack<int32_t> operand_stack;
};

/**
 *  Maintain a stack of frames, each containing a local variable array and an operand stack
 *
 *  This implementation only gives access to the frame at the top, requiring pop() to access those below.
 */
class FrameStack {
private:
    std::stack<Frame> stack;

public:
    Item *const_pool_ptr; // TODO:  Can't be sure this is still allocated


    void frame_push(size_t local_var_size, size_t stack_size) {
        Frame new_frame = Frame();
        new_frame.local_vars = std::vector<int32_t>(local_var_size);

        stack.push(new_frame);
    }

    void frame_pop() {
        stack.pop();
    }

    int32_t get_local_var(size_t index) {
        return stack.top().local_vars.at(index);
    }

    void set_local_var(size_t index, int32_t value) {
        stack.top().local_vars.at(index) = value;
    }


    int32_t stack_pop() {
        assert(stack.top().operand_stack.size() != 0);
        int32_t ret = stack.top().operand_stack.top();
        stack.top().operand_stack.pop();
        return ret;
    }

    void stack_push(int32_t val) {
        stack.top().operand_stack.push(val);
    }

    void stack_shrink(size_t len) {
        for (int i = 0; i < len; ++i) {
            assert(stack.top().operand_stack.size() != 0);
            stack.top().operand_stack.pop();
        }
    }

    String const_pool_string(size_t index) {
        Item &item = const_pool_ptr[index];
        assert(item.tag == STRING_UTF8);
        return item.value.string;
    }

    int32_t const_pool_int(size_t index) {
        Item &item = const_pool_ptr[index];
        assert(item.tag == INTEGER);
        return item.value.integer;
    }

    bool stack_is_empty() {
        return stack.top().operand_stack.size() == 0;
    }

    Item &const_pool(int index) {
        assert(index > 0);
        return const_pool_ptr[index - 1];
    }

};

#endif //VM_FRAMESTACK_H
