/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
#include <iostream>
#include <stdio.h>
#include <iomanip>

float a(const unsigned char* hex, int size) {
    union{
        unsigned char bytes[4];
        float value;
    } data;
    
    if(size < 4) {
        return 0.0f;
    }
    
    for (int i = 0; i < 4; i++){
        data.bytes[i] = hex[size-1-i];
    }
    
    return data.value;
}

void floatToUnsignedChar(float floatValue, unsigned char* hex) {
    union {
        float value;
        unsigned char bytes[4];
    } data;

    data.value = floatValue;

    for (int i = 0; i < 4; ++i) {
        hex[i] = data.bytes[3 - i];
    }
}

// Convert 4-byte unsigned char array to hexadecimal number
uint32_t unsignedCharToHex(const unsigned char* bytes) {
    uint32_t result = 0;

    // Combine the bytes into a single 32-bit integer
    for (int i = 0; i < 4; ++i) {
        result |= static_cast<uint32_t>(bytes[i]) << (8 * (3 - i));
    }

    return result;
}

int main()
{
    printf("Hello World\n");
    
    // 定义一个包含 4 个字节的 unsigned char 数组
    // unsigned char hexBytes[4] = {0xBC, 0xF5, 0xc2, 0x8f}; // Example hexadecimal representation of a float: 3.14f
    
    // 调用函数 a，并传递 unsigned char 数组和数组大小作为参数
    // float floatValue = a(hexBytes, 4);
    
    // 输出转换后的 float 值
    // std::cout << "Converted float value: " << floatValue << std::endl;
    
    float floatValue = -1.00000000f;
    unsigned char hexBytes[4];
    
    while(floatValue < 1){
        floatValue = floatValue + 0.0000001;
        floatToUnsignedChar(floatValue, hexBytes);
        
        // Convert unsigned char array to hexadecimal number
        uint32_t hexNumber = unsignedCharToHex(hexBytes);
    
        // Output the hexadecimal number using printf
        printf("Float value: %f --> Unsigned char array to hexadecimal number: 0x%08X\n", floatValue, hexNumber);
    
        // Output the converted unsigned char array
        std::cout << "Float value: " << floatValue << " --> Hexadecimal representation: ";
        for (int i = 0; i < 4; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hexBytes[i]);
        }
        std::cout << std::endl;
    }
    return 0;
}
