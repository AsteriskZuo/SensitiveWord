//
//  dictionary.hpp
//  test_dictionary
//
//  Created by Asterisk on 11/29/19.
//  Copyright © 2019 Asterisk. All rights reserved.
//

#ifndef dictionary_hpp
#define dictionary_hpp

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace zy {

/**
 * 敏感词查询
 * 敏感词和查询内容必须是u8编码
 */
class sensitive_word
{
public:
    struct config
    {
        config() : letter_case_sensitive(false), traditional_sensitive(true) {}
        config(const bool letter_case, const bool traditional) : letter_case_sensitive(letter_case), traditional_sensitive(traditional) {}
        bool letter_case_sensitive;//字母大小写是否敏感 true 分别匹配 false 忽略大小写都匹配
        bool traditional_sensitive;//繁体是否敏感 true 分别匹配 false 忽略简体繁体都匹配
    };
public:
    sensitive_word();
    sensitive_word(const std::string& res_dir);
    virtual ~sensitive_word();
    
    /**
     * 基本配置
     * @param cfg 配置信息
     */
    void set_config(const sensitive_word::config& cfg);
    
    /**
     * 初始化字典树
     * @param words 敏感词列表 utf8编码
     */
    void init(const std::vector<std::string>& words);
    
    /**
     * 是否包含敏感词
     * @param content 内容
     * @return true 匹配 false 不匹配
     */
    bool match(const std::string& content);
    
private:
    struct node
    {
        char16_t c;//unicode test
        bool is_end;//is word end
        std::unordered_map<char16_t, node*> children;
    };
    
private:
    void append(const std::u16string& word);
    void append_u8(const std::string& word);
    void release(node* n);
    void trim(std::u16string& str);
    void trim_u8(std::string& str);
    void convert_letter_case(std::u16string& str);
    void convert_traditional(std::u16string& str);
    std::string convert_traditional_u8(const std::string& str);
    std::u16string convert_from_u8_to_u16(const std::string& u8);
    std::string conver_from_u16_to_u8(const std::u16string& u16);
    
private:
    node* root_;//字典树
    config* cfg_;
    void* opencc_;
};

void test_minganci();
void test_minganci_zhongwen();
void test_minganci_yingwen_config_false_false();
void test_minganci_zhongwen_config_true_true();
void test_minganci_zhongwen_fanti_config_false_false();
void test_minganci_res(const std::string& t2sjson);

} // zy

#endif /* dictionary_hpp */
