#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "search.h"

// 简单起见，限制最多64个根目录和64个忽略模式，方便定义数组
#define MAX_ROOTS 64
#define MAX_IGNORE 64


static void print_usage(const char * prog) {
    // 打印使用说明
    fprintf(stderr,
        "Usage:\n"
        "  %s roots... [options]\n"
        "Options:\n"
        "  -p, --pattern <glob>        File name glob (default: *)\n"
        "  -i, --ignore <glob>        Ignore glob (can repeat)\n"
        "  --max-results <N>          Stop after finding N results (0 = no limit, default: 0)\n"
        "  --workers <N>              Max worker threads (default: 8)\n"
        "  --concurrent               Use concurrent traversal\n"
        "  --contains <text>          Only keep files whose content contains this text\n"
        "  --encoding <enc>          Encoding for --contains (default: utf-8)\n",
        prog
    );
}


static bool parse_int(const char * s, int * out) {
    // 将字符串 s 转换为整数，并存储在 out 中
    char * end = NULL;
    // strtol函数将字符串转换为长整型，s是待转换的字符串，end是转换后的字符串的结束位置，10表示十进制
    // 注意，由于strtol需要修改end指针的指向，所以其参数类型是char **
    long v = strtol(s, &end, 10);
    // 这里确保s不是空字符串，end指向了转换后的字符串的结束位置，并且end指向的是\0字符(说明我们没有传入非数字字符)
    if (s[0] == '\0' || end == NULL || *end != '\0') return false;
    if (v < -2147483648L || v > 2147483647L) return false;  // 将max_results的取值范围限制在int的范围内
    *out = (int)v;  // 将转换后的长整型值转换为int类型，并存储在out中
    return true;
}


static bool starts_with(const char * s, const char * prefix) {
    // 检查字符串 s 是否以 prefix 开头
    return strncmp(s, prefix, strlen(prefix)) == 0;
}


int main(int argc, char ** argv) {
    // argv[i]中保存着字符串形式的命令行参数，argv本身是一个指针数组，我们也可以写成char * argv[]的形式
    // 优先通过 argv[0] 获取程序名，如果 argv[0] 为空，则使用 "easydocsfinder"
    const char * prog = (argc > 0 && argv[0]) ? argv[0] : "easydocsfinder";

    const char * pattern = "*";  // 默认的匹配模式
    const char * encoding = "utf-8";  // 默认的编码
    int max_results = 0;  // 默认输出所有结果
    int workers = 8;  // 默认使用 8 个线程
    bool concurrent = false;  // 默认不使用并发遍历

    const char * contains = NULL;  // 默认不使用内容过滤

    const char * roots[MAX_ROOTS];  // 存储要遍历的根目录
    int roots_count = 0;

    const char * ignore_patterns[MAX_IGNORE];  // 存储对应要忽略的目录或文件的模式
    int ignore_count = 0;

    for (int i = 1; i < argc; i++) {
        // 遍历所有传入的命令行参数
        const char * arg = argv[i];  // 获取当前参数

        if (arg[0] == '-' && arg[1] != '\0') {
            // 处理-p和--pattern选项
            if (strcmp(arg, "-p") == 0 || strcmp(arg, "--pattern") == 0) {
                // 检查是否提供了模式参数，如果没有则打印使用说明并返回错误
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                pattern = argv[++i];
                continue;
            }
            if (starts_with(arg, "--pattern=")) {
                // 如果用户是以带=的形式提供模式参数，则通过移动指针获取模式参数
                pattern = arg + strlen("--pattern=");
                continue;
            }
            
            // 处理-i和--ignore选项
            if (strcmp(arg, "-i") == 0 || strcmp(arg, "--ignore") == 0) {
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                // 检查是否超过了最大忽略模式数量
                if (ignore_count >= MAX_IGNORE) { fprintf(stderr, "Too many --ignore\n"); return 2; }
                ignore_patterns[ignore_count++] = argv[++i];  // 把忽略模式添加到ignore_patterns数组中
                continue;
            }
            if (starts_with(arg, "--ignore=")) {
                if (ignore_count >= MAX_IGNORE) { fprintf(stderr, "Too many --ignore\n"); return 2; }
                ignore_patterns[ignore_count++] = arg + strlen("--ignore=");
                continue;
            }

            // 处理--max-results选项
            if (strcmp(arg, "--max-results") == 0) {
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                if (!parse_int(argv[++i], &max_results)) {
                    // 解析传入的参数，并校验
                    fprintf(stderr, "Invalid --max-results\n");
                    return 2;
                }
                continue;
            }
            if (starts_with(arg, "--max-results=")) {
                if (!parse_int(arg + strlen("--max-results="), &max_results)) {
                    fprintf(stderr, "Invalid --max-results\n");
                    return 2;
                }
                continue;
            }

            // 处理--workers选项
            if (strcmp(arg, "--workers") == 0) {
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                if (!parse_int(argv[++i], &workers)) {
                    fprintf(stderr, "Invalid --workers\n");
                    return 2;
                }
                continue;
            }
            if (starts_with(arg, "--workers=")) {
                if (!parse_int(arg + strlen("--workers="), &workers)) {
                    fprintf(stderr, "Invalid --workers\n");
                    return 2;
                }
                continue;
            }

            // 处理--concurrent选项
            if (strcmp(arg, "--concurrent") == 0) {
                concurrent = true;
                continue;
            }

            // 处理--contains选项
            if (strcmp(arg, "--contains") == 0) {
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                contains = argv[++i];
                continue;
            }
            if (starts_with(arg, "--contains=")) {
                contains = arg + strlen("--contains=");
                continue;
            }

            // 处理--encoding选项
            if (strcmp(arg, "--encoding") == 0) {
                if (i + 1 >= argc) { print_usage(prog); return 2; }
                encoding = argv[++i];
                continue;
            }
            if (starts_with(arg, "--encoding=")) {
                encoding = arg + strlen("--encoding=");
                continue;
            }

            // 处理未知选项
            fprintf(stderr, "Unknown option: %s\n", arg);
            print_usage(prog);
            return 2;
        }

        // 前缀没有-的参数，就是位置参数，即要遍历的根目录
        if (roots_count >= MAX_ROOTS) { fprintf(stderr, "Too many roots\n"); return 2; }
        roots[roots_count++] = arg;
    }

    if (roots_count == 0) {
        // 必须至少提供一个根目录
        print_usage(prog);
        return 2;
    }

    size_t found = search_files_single_thread(
        roots,
        (size_t)roots_count,
        pattern,
        ignore_patterns,
        (size_t)ignore_count,
        (size_t)(max_results < 0 ? 0 : max_results)
    );
    
    if (found == 0) {
        fprintf(stderr, "No matching files found.\n");
    }

    return 0;
}
