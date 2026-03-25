#include "search.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SEARCH_PATH_BUF 4096

static bool wildcard_match(const char * pattern, const char * text) {
    // 支持：* 任意长度；? 单字符；其他字符按字面匹配
    const char * p = pattern;  // 用p来遍历模式字符串
    const char * t = text;  // 用t来遍历待匹配的字符串

    const char * star = NULL;  // 记录模式字符串中*的位置
    const char * star_text = NULL;  // 记录待匹配字符串中第一个与*匹配的位置

    // 只要待匹配的字符串还没结束，就继续匹配
    while (*t) {
        // 如果模式字符串当前字符是?或与待匹配字符串当前字符相同，则继续匹配下一个字符
        if (*p == '?' || *p == *t) {
            p++;
            t++;
            continue;
        }

        if (*p == '*') {
            // 如果模式字符串当前字符是*，则记录当前位置和待匹配字符串的位置
            star = p++;
            star_text = t;
            continue;
        }

        if (star) {
            // 如果模式字符串中发现了*，并且之后的字符没有和待匹配字符串匹配，
            // 那么就让p固定在*的后一个位置，然后让t一直往后移动，消耗掉和*匹配的所有字符，直到找到下一个和*下一位匹配的字符
            p = star + 1;
            t = ++star_text;
            continue;
        }

        return false;  // 执行到这里说明匹配失败
    }

    // text全部匹配完后：如果pattern还没到尽头，要想匹配成功，那剩下的只能是若干个*
    while (*p == '*') p++;
    return *p == '\0';  // 把*都消耗完之后，如果pattern也到头了，说明匹配成功，否则匹配失败
}

static const char * basename_of(const char * path) {
    // 在path中找到最后一个斜杠或反斜杠的位置
    const char * s1 = strrchr(path, '\\');
    const char * s2 = strrchr(path, '/');
    const char * s = s1 > s2 ? s1 : s2;  // 取s1和s2中位置较后的那个
    return s ? (s + 1) : path;  // 如果s不为空，则返回s+1，否则返回path
}

static void join_path(char * out, size_t outsz, const char * a, const char * b) {
    // 将a和b拼接成一个完整的路径，并存储在out中
    size_t alen = strlen(a);
    if (alen > 0 && (a[alen - 1] == '\\' || a[alen - 1] == '/')) {
        // 如果a的末尾已经有斜杠，则直接拼接b
        snprintf(out, outsz, "%s%s", a, b);
    } else {
        // 如果a的末尾没有斜杠，则拼接\和b
        snprintf(out, outsz, "%s\\%s", a, b);
    }
}

static bool should_ignore_name(const char ** ignore_patterns, size_t ignore_count, const char *name) {
    // 遍历所有忽略模式，如果name与某个忽略模式匹配，则返回true
    for (size_t i = 0; i < ignore_count; i++) {
        if (ignore_patterns[i] && wildcard_match(ignore_patterns[i], name)) {
            return true;
        }
    }
    return false;
}

static size_t walk_dir(
    const char * dir,
    const char * pattern,
    const char ** ignore_patterns,
    size_t ignore_count,
    size_t max_results,
    size_t found
) {
    // 如果找到的结果数量达到最大限制，则停止搜索
    if (max_results > 0 && found >= max_results) return found;

    char search_path[SEARCH_PATH_BUF];
    // 通过snprintf函数将dir和\*拼接并写入search_path中
    snprintf(search_path, sizeof(search_path), "%s\\*", dir);

    WIN32_FIND_DATAA data;  // 用于存储每个文件或目录的详细信息的结构体
    // 通过FindFirstFileA函数按照search_path指定的路径查找第一个结果，将结果存储在data中
    HANDLE h = FindFirstFileA(search_path, &data);  // 如果成功，会返回一个有效句柄
    if (h == INVALID_HANDLE_VALUE) return found;  // 如果返回的句柄无效，就停止搜索

    do {
        const char * name = data.cFileName;  // 获取文件或目录的名称

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;  // 如果文件或目录的名称是.或..，则跳过

        bool is_dir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;  // 判断文件或目录是否是目录

        if (is_dir) {
            // 如果是目录，先看看是否需要忽略
            if (should_ignore_name(ignore_patterns, ignore_count, name)) continue;
            // 无需忽略，则调用join_path函数将dir和name拼接成子目录的完整路径，并递归遍历子目录
            char child[SEARCH_PATH_BUF];
            join_path(child, sizeof(child), dir, name);
            found = walk_dir(child, pattern, ignore_patterns, ignore_count, max_results, found);
            if (max_results > 0 && found >= max_results) break;
        } else {
            // 如果是文件，先看看是否需要忽略
            if (should_ignore_name(ignore_patterns, ignore_count, name)) continue;
            // 再看看是否匹配模式
            if (wildcard_match(pattern, name)) {
                char full[SEARCH_PATH_BUF];
                join_path(full, sizeof(full), dir, name);
                puts(full);
                found++;
                if (max_results > 0 && found >= max_results) break;
            }
        }
    } while (FindNextFileA(h, &data) != 0);

    FindClose(h);
    return found;
}

static size_t search_root(
    const char * root,
    const char * pattern,
    const char ** ignore_patterns,
    size_t ignore_count,
    size_t max_results,
    size_t found
) {
    // 该函数只处理根目录，根据找到的是目录还是文件，分别调用walk_dir或进行文件名匹配
    if (max_results > 0 && found >= max_results) return found;

    // 通过GetFileAttributesA函数获取文件属性
    DWORD attr = GetFileAttributesA(root);
    if (attr == INVALID_FILE_ATTRIBUTES) return found;

    const char * name = basename_of(root);
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        // 如果是目录，则先看看是否需要忽略
        if (should_ignore_name(ignore_patterns, ignore_count, name)) return found;
        // 无需忽略，则递归遍历目录
        return walk_dir(root, pattern, ignore_patterns, ignore_count, max_results, found);
    }

    // 如果是文件，则进行文件名匹配，看看是否需要忽略，并且是否匹配模式
    if (!should_ignore_name(ignore_patterns, ignore_count, name) && wildcard_match(pattern, name)) {
        puts(root);  // 输出文件路径
        found++;
    }
    return found;
}


size_t search_files_single_thread(
    const char ** roots,
    size_t roots_count,
    const char * pattern,
    const char ** ignore_patterns,
    size_t ignore_count,
    size_t max_results
) {
    // 入口函数，用于遍历所有根目录，并调用search_root函数进行搜索
    size_t found = 0;

    for (size_t i = 0; i < roots_count; i++) {
        found = search_root(roots[i], pattern, ignore_patterns, ignore_count, max_results, found);
        // 如果找到的结果数量达到最大限制，则停止搜索
        if (max_results > 0 && found >= max_results) break;
    }

    return found;
}
