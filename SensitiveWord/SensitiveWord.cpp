//
//  dictionary.cpp
//  test_dictionary
//
//  Created by Asterisk on 11/29/19.
//  Copyright © 2019 Asterisk. All rights reserved.
//

#include "SensitiveWord.hpp"
#include <opencc/opencc.h>
#include <opencc/BundleToolsCpp.hpp>

#include <codecvt>
#include <cassert>
#include <locale>
#include <sstream>
#include <iostream>

void test11111()
{
    opencc_t opencc_object = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
    std::string u8_str = u8" 中國人";
    std::string output;
    output.reserve(u8_str.size());
    char* char_output = new char[u8_str.size()]{0};
    
    
//    std::size_t ret = opencc_convert_utf8_to_buffer(opencc_object, u8_str.data(), u8_str.size(), const_cast<char*>(output.data()));
    std::size_t ret = opencc_convert_utf8_to_buffer(opencc_object, u8_str.data(), u8_str.size(), char_output);
    if (std::string::npos != ret) {
        std::string sss = output;
    } else {
        int a = ret;
    }
    int ret2 = opencc_close(opencc_object);
    int b = ret2;
}

void test_bundle()
{
    BundleToolsCpp* p = new BundleToolsCpp();
    CFBundleRef bundle = p->getBundle();
    CFRelease(bundle);
}

namespace zy {

sensitive_word::sensitive_word()
    : root_(new sensitive_word::node())
    , cfg_(new config())
    , opencc_(opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP))
{
    
}
sensitive_word::sensitive_word(const std::string& res_dir)
    : root_(new sensitive_word::node())
    , cfg_(new config())
{
    opencc_config(res_dir.c_str());
    opencc_ = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
}
sensitive_word::~sensitive_word()
{
    release(root_);
    delete cfg_; cfg_ = nullptr;
    int ret = opencc_close(opencc_);
    opencc_ = nullptr;
}

void sensitive_word::release(node* n)
{
    if (n) {
        for (auto iter = n->children.begin(); iter != n->children.end(); ++iter) {
            release(iter->second);
        }
        node* tmp = n;
        delete n;
        if (tmp == root_) {
            root_ = nullptr;
        }
    }
}

void sensitive_word::trim(std::u16string& str)
{
    std::u16string::size_type start = str.find_first_not_of(u" ");
    std::u16string::size_type end = str.find_last_not_of(u" ");
    str = str.substr(start, end - start + 1);
}

void sensitive_word::trim_u8(std::string& str)
{
    std::string::size_type start = str.find_first_not_of(u8" ");
    std::string::size_type end = str.find_last_not_of(u8" ");
    str = str.substr(start, end - start + 1);
}

void sensitive_word::convert_letter_case(std::u16string& str)
{
    for (auto iter = str.begin(); iter != str.end(); ++iter) {
        int value = *iter;
        if (0x41 <= value && 0x5A >= value) {
            *iter = (value + 0x20);//大写变成小写
        }
    }
}

void sensitive_word::convert_traditional(std::u16string& str)
{
    
}

std::string sensitive_word::convert_traditional_u8(const std::string& str)
{
    std::string ret(str);
    char* output = new char[ret.size()]{0};
    std::size_t output_size = opencc_convert_utf8_to_buffer(opencc_, ret.data(), ret.size(), output);
    if (std::string::npos != output_size) {
        ret = std::string(output, output_size);
    }
    delete [] output; output = nullptr;
    return ret;
}

void sensitive_word::set_config(const sensitive_word::config& cfg)
{
    if (cfg_) {
        cfg_->letter_case_sensitive = cfg.letter_case_sensitive;
        cfg_->traditional_sensitive = cfg.traditional_sensitive;
    }
}

std::u16string sensitive_word::convert_from_u8_to_u16(const std::string& u8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8);
}
std::string sensitive_word::conver_from_u16_to_u8(const std::u16string& u16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
}

void sensitive_word::init(const std::vector<std::string>& words)
{
    for (auto iter = words.begin(); iter != words.end(); ++iter) {
        if (iter->empty()) {
            continue;
        }
        append_u8(*iter);
    }
}

bool sensitive_word::match(const std::string& content)
{
    if (0 == root_->children.size()) {
        return false;
    }
    
    auto lambda_match = [this](const std::string& u8_content)->bool {
        
        std::u16string&& u16_content = convert_from_u8_to_u16(u8_content);
        
        if (cfg_ && !cfg_->letter_case_sensitive) {
            convert_letter_case(u16_content);
        }

        for (auto iter = u16_content.begin(); iter != u16_content.end(); ++iter) {
            node* head = root_;
            auto tmp_iter = iter;
            std::unordered_map<char16_t, node*>::const_iterator i;
            while (static_cast<void>(i = head->children.find(*tmp_iter)), i != head->children.end()) {
                node* tmp = i->second;
                if (true == tmp->is_end) {
                    return true;
                }
                head = tmp;
                ++tmp_iter;
            }
        }
        
        return false;
    };
    
    if (cfg_ && !cfg_->traditional_sensitive) {
        const std::string&& content_tmp = convert_traditional_u8(content);
        if (content_tmp.empty()) {
            return false;
        }
        return lambda_match(content_tmp);
    } else {
        return lambda_match(content);
    }
    
    return false;
}

void sensitive_word::append(const std::u16string& word)
{
    node* head = root_;
    for (auto iter = word.begin(); iter != word.end(); ++iter) {
        
        const char16_t& key = *iter;
        auto i = head->children.find(key);
        if (i != head->children.end()) {
            
            auto tmp = iter;
            if (++tmp == word.end()) {
                i->second->is_end = true;//modify
            }
            
            head = i->second;
            
        } else {
            node* n = new node();
            n->c = key;
            n->is_end = false;//init
            
            auto tmp = iter;
            if (++tmp == word.end()) {
                n->is_end = true;//modify
            }
            
            head->children.insert(std::make_pair(n->c, n));
            
            head = n;
        }
        
    }
}

void sensitive_word::append_u8(const std::string& word)
{
    std::string word_tmp = word;
    
    trim_u8(word_tmp);
    if (word_tmp.empty()) {
        return;
    }
    
    if (cfg_ && !cfg_->traditional_sensitive) {
        word_tmp = convert_traditional_u8(word_tmp);
        if (word_tmp.empty()) {
            return;
        }
    }
    
    std::u16string&& word_u16 = convert_from_u8_to_u16(word_tmp);
    
    if (cfg_ && !cfg_->letter_case_sensitive) {
        convert_letter_case(word_u16);
    }
    
    append(word_u16);
}

void test_minganci()
{
    sensitive_word* p_tree = new sensitive_word();
    std::vector<std::string> list = {u8" abc", u8"ab ", u8"ad"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"hahah  aabnow");
    delete p_tree;
    p_tree = nullptr;
    std::cout << ret << std::endl;
}
void test_minganci_zhongwen()
{
//    test11111();
    sensitive_word* p_tree = new sensitive_word();
    std::vector<std::string> list = {u8" 中国人", u8"中国 ", u8"中日"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"中央电视台播放，今天中国人民站起来了~");
    delete p_tree;
    p_tree = nullptr;
    std::cout << ret << std::endl;
}

void test_minganci_yingwen_config_false_false()
{
    sensitive_word* p_tree = new sensitive_word();
    p_tree->set_config(zy::sensitive_word::config{false, false});
    std::vector<std::string> list = {u8" abc", u8"aB ", u8"Ad"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"hahah  aabnow");
    delete p_tree;
    p_tree = nullptr;
    std::cout << ret << std::endl;
}

void test_minganci_zhongwen_config_true_true()
{
    sensitive_word* p_tree = new sensitive_word();
    p_tree->set_config(zy::sensitive_word::config{true, true});
    std::vector<std::string> list = {u8" 中国人", u8"中国 ", u8"中日"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"中央电视台播放，今天中国人民站起来了~");
    delete p_tree;
    p_tree = nullptr;
    std::cout << ret << std::endl;
}

void test_minganci_zhongwen_fanti_config_false_false()
{
    sensitive_word* p_tree = new sensitive_word();
    p_tree->set_config(zy::sensitive_word::config{false, false});
    std::vector<std::string> list = {u8" 中國人", u8"中國 ", u8"中日"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"中央电视台播放，今天中国人民站起来了~");
    delete p_tree;
    p_tree = nullptr;
    std::cout << ret << std::endl;
}

void test_minganci_res(const std::string& t2sjson)
{
//    test_bundle();
    std::string&& resdir = BundleToolsCpp::getResourceDirectory();
    sensitive_word* p_tree = new sensitive_word(resdir);
    p_tree->set_config(zy::sensitive_word::config{false, false});
    std::vector<std::string> list = {u8" 中國人", u8"中國 ", u8"中日", u8"龍"};
    p_tree->init(list);
    bool ret = p_tree->match(u8"中央电视台播放，今天中国人民站起来了~");
    std::cout << ret << std::endl;
    ret = p_tree->match(u8"中央电视台播放，今天中人民站起来了~");
    std::cout << ret << std::endl;
    delete p_tree;
    p_tree = nullptr;
    
}

} // zy


