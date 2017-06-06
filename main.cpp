#include <iostream>
#include <array>
#include "class.h"
#include "class.c"
#include "print.c"
#include "portable_endian.h"
#include "FrameStack.h"


std::string conv_class_string(String &class_string) {
    return std::string(class_string.value, class_string.length);
}

namespace jvm_op {

instFunc iconst_n(int i) {
    // Function generator iconst(i)
    return [i](uint8_t *&pc, FrameStack &frame) -> int {
      frame.stack_push((uint32_t) i);
      pc += 1;
      return 0;
    };
}

int getstatic(uint8_t *&pc, FrameStack &frame) {
    uint16_t index;
    memcpy(&index, pc + 1, sizeof(uint16_t));
    index = be16toh(index);

    std::cout << index << std::endl; //This is a index in the constant pool
    frame.stack_push((uint32_t) index);
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


    std::cout << className << std::endl;
    std::cout << methodName << std::endl;
    std::cout << signatureName << std::endl;
    //std::cout << ret << std::endl;

    if (className == "java/io/PrintStream" && methodName == "println") {
        if (signatureName == "(Ljava/lang/String;)V") {
            std::cout
                << frame.const_pool(frame.const_pool(frame.stack_value(-1)).value.ref.class_idx).value.string.value
                << std::endl;
        } else if (signatureName == "(I)V") {
            std::cout << frame.stack_value(-1) << std::endl;
        }
        frame.stack_shrink(1);
    }

    pc += 3;
    return 0;
}

int ldc(uint8_t *&pc, FrameStack &frame) {
    //DO stuff
    uint32_t index = pc[1]; //Jump over the op code (1 byte)
    std::cout << index << std::endl;
    frame.stack_push(index);

    pc += 2;
    return 0;
}
int aload_0(uint8_t *&pc, FrameStack &frame) {
    frame.stack_push(frame.get_local_var(0));
    pc += 1;
    return 0;
}

int retur(uint8_t *&pc, FrameStack &frame) {
    pc += 1;
    return -1; //Some value so ww know when to return
}

instFunc if_cond(std::function<bool(int)> predicate) {
    return [predicate](uint8_t *&pc, FrameStack &frame) -> int {
      uint32_t value = frame.stack_value(-1);
      int16_t offset = be16toh(*reinterpret_cast<int16_t *>(pc + 1));

      if (predicate(value)) {
          pc += offset;
      } else {
          pc += 3;
      }

      return 0; //Some value so ww know when to return
    };
}

}

byteCode byteCodes[] = {
    {"iconst_0", 0x3, 1, jvm_op::iconst_n(0)},
    {"iconst_1", 0x4, 1, jvm_op::iconst_n(1)},
    {"iconst_2", 0x5, 1, jvm_op::iconst_n(2)},
    {"iconst_3", 0x6, 1, jvm_op::iconst_n(3)},
    {"iconst_4", 0x7, 1, jvm_op::iconst_n(4)},
    {"iconst_5", 0x8, 1, jvm_op::iconst_n(5)},
    {"getstatic", 0xB2, 3, jvm_op::getstatic},
    {"invokespecial", 0xB7, 3, jvm_op::invokevirtual}, // TODO: Own impl?
    {"invokevirtual", 0xB6, 3, jvm_op::invokevirtual},
    {"ldc", 0x12, 2, jvm_op::ldc},
    {"aload_0", 0x2A, 1, jvm_op::aload_0},
    {"return", 0xB1, 1, jvm_op::retur},
    {"ifeq", 0x99, 3, jvm_op::if_cond([](int a) -> bool { return a == 0; })},
    {"ifne", 0x9A, 3, jvm_op::if_cond([](int a) -> bool { return a != 0; })},
    {"iflt", 0x9B, 3, jvm_op::if_cond([](int a) -> bool { return a < 0; })},
    {"ifge", 0x9C, 3, jvm_op::if_cond([](int a) -> bool { return a >= 0; })},
    {"ifgt", 0x9D, 3, jvm_op::if_cond([](int a) -> bool { return a > 0; })},
    {"ifle", 0x9E, 3, jvm_op::if_cond([](int a) -> bool { return a <= 0; })}
};

instFunc findOpCode(unsigned char op) {
    for (int i = 0; i < 13; i++) {
        if (byteCodes[i].op == op) {
            std::cout << "# " << byteCodes[i].name << std::endl;
            return byteCodes[i].func;
        }
    }
    return 0;
}

int main() {


    Class *cls =
        read_class_from_file_name((char *) "/Users/hadar/Documents/kompkon17/mini-VM/HelloWorld.class");

    print_class(stdout, cls);
    fflush(stdout);

    CodeAttribute *code_attribute = (CodeAttribute *) malloc(sizeof(CodeAttribute));

    for (int i = 0; i < cls->methods[0].attrs_count; i++) {

        attributeToCode(code_attribute, &cls->methods[1].attrs[i]);

    }

    uint8_t *pc = code_attribute->code;
    FrameStack frameStack;
    frameStack.const_pool_ptr = cls->items;

    //IMPORTANT need to be +1 I think don't really get it !
    if (strcmp(cls->items[cls->methods[0].name_idx + 1].value.string.value, "Code") != 0) {
        std::cout << "Not a code segment die !!!" << std::endl;
        return 0;
    }
    frameStack.frame_push(16, 16); // TODO: Set dynamically
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

    return 0;
}

