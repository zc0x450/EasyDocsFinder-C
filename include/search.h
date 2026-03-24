// 通过预处理指令#pragma once来防止头文件被重复包含，其等价于组合使用经典的#ifndef, #define和#endif
#pragma once
#include <stddef.h>  // 用于使用size_t类型

// 在C++中，因为支持函数重载，编译的时候会修改函数名，所以如果用C++编译器编译使用了C头文件的代码，需要显式地告诉编译器这是一个C函数
// 在C++中，如果一个函数被声明为extern "C"，会禁止C++的名称改编特性，确保函数名保持原样
#ifdef __cplusplus
extern "C" {
#endif

// Windows 单线程搜索：
// - 只匹配“文件名”（不匹配路径）
// - pattern 支持 '*'（任意长度）和 '?'（单字符）
// - ignore_patterns 同样按“文件名”做匹配：命中则跳过
// - max_results: 0 表示不限制，>0 表示最多找到 N 个
size_t search_files_single_thread(
    const char ** roots,
    size_t roots_count,
    const char * pattern,
    const char ** ignore_patterns,
    size_t ignore_count,
    size_t max_results
);

#ifdef __cplusplus
}
#endif
