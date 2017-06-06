#include <iostream>
#include <array>
#include "class.h"
#include "class.c"
#include "print.c"
#include "portable_endian.h"
#include "FrameStack.h"

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG(x) { std::cerr << x << std::endl; }
#else
#define DEBUG(x) {  }
#endif

std::string conv_class_string(String &class_string) {
    return std::string(class_string.value, class_string.length);
}

namespace jvm_op {

    instFunc iconst_n(int i) {
        // Function generator iconst(i)
        return [i](uint8_t *&pc, FrameStack &frame) -> int {
            frame.stack_push((int32_t) i);
            pc += 1;
            return 0;
        };
    }

    int getstatic(uint8_t *&pc, FrameStack &frame) {
        uint16_t index;
        memcpy(&index, pc + 1, sizeof(uint16_t));
        index = be16toh(index);

        DEBUG(index) //This is a index in the constant pool
        frame.stack_push((int32_t) index);
        pc += 3;

        return 0;
    }

    int invokevirtual(uint8_t *&pc, FrameStack &frame) {
        uint16_t index;
        memcpy(&index, pc + 1, sizeof(uint16_t));
        index = be16toh(index);

        String &classStr = frame.const_pool(
                frame.const_pool(frame.const_pool(index).value.ref.class_idx).value.ref.class_idx).value.string;
        std::string className = conv_class_string(classStr);

        String &methodStr = frame.const_pool(
                frame.const_pool(frame.const_pool(index).value.ref.name_idx).value.ref.class_idx).value.string;
        std::string methodName = conv_class_string(methodStr);

        String &signatureStr = frame.const_pool(
                frame.const_pool(frame.const_pool(index).value.ref.name_idx).value.ref.name_idx).value.string;
        std::string signatureName = conv_class_string(signatureStr);

        //char *ret = p[p[p[index - 1].value.ref.name_idx - 1].value.ref.name_idx - 1].value.string.value;


        DEBUG (className);
        DEBUG (methodName);
        DEBUG (signatureName);


        if (className == "java/io/PrintStream" && methodName == "println") {
            if (signatureName == "(Ljava/lang/String;)V") {
                std::cout <<
                          frame.const_pool(
                                  frame.const_pool(frame.stack_pop()).value.ref.class_idx).value.string.value
                          << std::endl;
            } else if (signatureName == "(I)V") {
                std::cout << frame.stack_pop() << std::endl;
            }
            frame.stack_shrink(1);
        }

        pc += 3;
        return 0;
    }

    int ldc(uint8_t *&pc, FrameStack &frame) {
        //DO stuff
        int32_t index = pc[1]; //Jump over the op code (1 byte)
        DEBUG(index);
        frame.stack_push(index);

        pc += 2;
        return 0;
    }

    instFunc istore_n(size_t i) {
        return [i](uint8_t *&pc, FrameStack &frame) -> int {
            int32_t val = frame.stack_pop();
            frame.set_local_var(i, val);
            pc += 1;
            return 0;
        };
    }

    int istore(uint8_t *&pc, FrameStack &frame) {

        uint8_t i = *(pc + 1);
        int32_t val = frame.stack_pop();
        frame.set_local_var(i, val);
        pc += 2;
        return 0;

    }


    instFunc iload_n(size_t i) {

        return [i](uint8_t *&pc, FrameStack &frame) -> int {
            int32_t val = frame.get_local_var(i);
            frame.stack_push(val);
            pc += 1;
            return 0;
        };
    }

    int iload(uint8_t *&pc, FrameStack &frame) {
        uint8_t i = *(pc + 1);
        int32_t val = frame.get_local_var(i);
        frame.stack_push(val);

        pc += 2;

        return 0;
    }

    int bipush(uint8_t *&pc, FrameStack &frame) {
        int8_t val = *(pc + 1);

        //Sign extend int8 to int32 and treat it as an unsigned int32
        int32_t temp = static_cast<int32_t>(val);
        frame.stack_push(temp);

        pc += 2;

        return 0;
    }

    int return_(uint8_t *&pc, FrameStack &frame) {
        pc += 1;
        return -1; //Some value so ww know when to return
    }

    int pop(uint8_t *&pc, FrameStack &frame) {
        frame.stack_pop();
        pc += 1;
        return 0;
    }

    int goto_(uint8_t *&pc, FrameStack &frame) {

        int16_t offset = be16toh(*reinterpret_cast<int16_t *>(pc + 1));
        pc += offset;

        return 0;
    }

    instFunc if_icmp_cond(std::function<bool(int, int)> predicate) {
        return [predicate](uint8_t *&pc, FrameStack &frame) -> int {
            int32_t rhs = static_cast<int32_t >(frame.stack_pop());
            int32_t lhs = static_cast<int32_t >(frame.stack_pop());
            int16_t offset = be16toh(*reinterpret_cast<int16_t *>(pc + 1));

            if (predicate(lhs, rhs)) {
                pc += offset;
            } else {
                pc += 3;
            }

            return 0; //Some value so ww know when to return
        };
    }


    instFunc if_cond(std::function<bool(int)> predicate) {
        return [predicate](uint8_t *&pc, FrameStack &frame) -> int {
            int32_t value = static_cast<int32_t >(frame.stack_pop());
            int16_t offset = be16toh(*reinterpret_cast<int16_t *>(pc + 1));

            if (predicate(value)) {
                pc += offset;
            } else {
                pc += 3;
            }

            return 0; //Some value so ww know when to return
        };
    }

    instFunc i_bin_op(std::function<int(int, int)> operation) {
        return [operation](uint8_t *&pc, FrameStack &frame) -> int {
            int32_t rhs = static_cast<int32_t >(frame.stack_pop());
            int32_t lhs = static_cast<int32_t >(frame.stack_pop());


            frame.stack_push(operation(lhs, rhs));

            pc += 1;


            return 0; //Some value so ww know when to return
        };
    }

}


byteCode byteCodes[] = {
        {"aconst_null",   0x1,  1, jvm_op::iconst_n(0)},
        {"iconst_0",      0x3,  1, jvm_op::iconst_n(0)},
        {"iconst_1",      0x4,  1, jvm_op::iconst_n(1)},
        {"iconst_2",      0x5,  1, jvm_op::iconst_n(2)},
        {"iconst_3",      0x6,  1, jvm_op::iconst_n(3)},
        {"iconst_4",      0x7,  1, jvm_op::iconst_n(4)},
        {"iconst_5",      0x8,  1, jvm_op::iconst_n(5)},
        {"bipush",        0x10, 2, jvm_op::bipush},


        //Store int into local variable
        {"istore_0",      0x3B, 1, jvm_op::istore_n(0)},
        {"istore_1",      0x3C, 1, jvm_op::istore_n(1)},
        {"istore_2",      0x3D, 1, jvm_op::istore_n(2)},
        {"istore_3",      0x3E, 1, jvm_op::istore_n(3)},
        {"istore",        0x36, 2, jvm_op::istore},


        {"getstatic",     0xB2, 3, jvm_op::getstatic},
        {"invokespecial", 0xB7, 3, jvm_op::invokevirtual}, // TODO: Own impl?
        {"invokevirtual", 0xB6, 3, jvm_op::invokevirtual},
        {"ldc",           0x12, 2, jvm_op::ldc},

        //Load int from local variable
        {"iload_0",       0x1A, 1, jvm_op::iload_n(0)},
        {"iload_1",       0x1B, 1, jvm_op::iload_n(1)},
        {"iload_2",       0x1C, 1, jvm_op::iload_n(2)},
        {"iload_3",       0x1D, 1, jvm_op::iload_n(3)},
        {"iload",         0x15, 2, jvm_op::iload},

        {"return",        0xB1, 1, jvm_op::return_},
        {"goto",          0xA7, 3, jvm_op::goto_},
        //Branch if int comparison with zero succeeds
        {"ifeq",          0x99, 3, jvm_op::if_cond([](int a) -> bool { return a == 0; })},
        {"ifne",          0x9A, 3, jvm_op::if_cond([](int a) -> bool { return a != 0; })},
        {"iflt",          0x9B, 3, jvm_op::if_cond([](int a) -> bool { return a < 0; })},
        {"ifge",          0x9C, 3, jvm_op::if_cond([](int a) -> bool { return a >= 0; })},
        {"ifgt",          0x9D, 3, jvm_op::if_cond([](int a) -> bool { return a > 0; })},
        {"ifle",          0x9E, 3, jvm_op::if_cond([](int a) -> bool { return a <= 0; })},
        //Branch if int comparison succeeds
        {"if_icmpeq",     0x9F, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a == b; })},
        {"if_icmpne",     0xA0, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a != b; })},
        {"if_icmplt",     0xA1, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a < b; })},
        {"if_icmpge",     0xA2, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a >= b; })},
        {"if_icmpgt",     0xA3, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a > b; })},
        {"if_icmplt",     0xA4, 3, jvm_op::if_icmp_cond([](int a, int b) -> bool { return a <= b; })},
        {"pop",           0x57, 1, jvm_op::pop},

        {"iadd",          0x60, 1, jvm_op::i_bin_op([](int a, int b) -> int { return a + b; })},
        {"isub",          0x64, 1, jvm_op::i_bin_op([](int a, int b) -> int { return a - b; })},
        {"imul",          0x68, 1, jvm_op::i_bin_op([](int a, int b) -> int { return a * b; })},
        {"idiv",          0x6C, 1, jvm_op::i_bin_op([](int a, int b) -> int { return a / b; })}
};

instFunc findOpCode(unsigned char op) {
    for (int i = 0; i < 41; i++) {
        if (byteCodes[i].op == op) {

            DEBUG("#" << byteCodes[i].name);
            return byteCodes[i].func;
        }
    }
    return 0;
}

int main(int argc,char** argv) {


    if(argc != 2){
        printf("Usage: mini-jvm <class-file>\n");
        return 1;
    }
    Class *cls = read_class_from_file_name(argv[1]);

    if (cls == NULL){
        return 1;
    }

#ifdef ENABLE_DEBUG
    print_class(stdout, cls);
#endif

    CodeAttribute *code_attribute = (CodeAttribute *) malloc(sizeof(CodeAttribute));

    for (int i = 0; i < cls->methods[0].attrs_count; i++) {

        attributeToCode(code_attribute, &cls->methods[0].attrs[i]);

    }

    uint8_t *pc = code_attribute->code;
    FrameStack frameStack;
    frameStack.const_pool_ptr = cls->items;

    //IMPORTANT need to be +1 I think don't really get it !
    if (strcmp(cls->items[cls->methods[0].name_idx + 1].value.string.value, "Code") != 0) {
        DEBUG("Not a code segment die !!!");
        return 0;
    }


    frameStack.frame_push(128, 128); // TODO: Set dynamically
    frameStack.set_local_var(0, 0);
    while (true) {

        instFunc func = findOpCode(pc[0]);
        if (func == nullptr) {
            fprintf(stderr, "Op %X not found\n", pc[0]);
            return 1;
        }
        int status = func(pc, frameStack);
        if (func == 0 || status == -1) {
            break;
        }

    };

    if (!frameStack.stack_is_empty()) {
        fprintf(stderr, "Stack not empty at end\n");
    }

    return 0;
}

