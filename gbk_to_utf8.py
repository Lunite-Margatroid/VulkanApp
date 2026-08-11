#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import chardet

def detect_encoding(file_path):
    """检测文件编码"""
    try:
        with open(file_path, 'rb') as f:
            raw_data = f.read()
            result = chardet.detect(raw_data)
            return result['encoding']
    except Exception as e:
        print(f"检测编码时出错 {file_path}: {e}")
        return None

def convert_gbk_to_utf8(file_path):
    """将GBK编码的文件转换为UTF-8"""
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='gbk') as f:
            content = f.read()
        
        # 写入UTF-8格式
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return True
    except UnicodeDecodeError:
        # 如果无法用GBK解码，可能不是GBK编码
        return False
    except Exception as e:
        print(f"转换文件时出错 {file_path}: {e}")
        return False

def should_convert(file_path):
    """判断文件是否应该转换"""
    # 先检测文件编码
    encoding = detect_encoding(file_path)
    
    if encoding is None:
        return False
    
    # 如果编码是GBK或GB2312，需要转换
    if encoding.lower() in ['gbk', 'gb2312', 'gb18030']:
        return True
    
    # 如果检测到的是UTF-8或ASCII，不需要转换
    if encoding.lower() in ['utf-8', 'ascii']:
        return False
    
    # 对于其他编码，尝试用GBK读取，如果成功则转换
    try:
        with open(file_path, 'r', encoding='gbk') as f:
            f.read()
        return True
    except:
        return False

def main():
    """主函数"""
    # 定义需要处理的文件扩展名
    extensions = {'.txt', '.cpp', '.h', '.c', '.hpp', '.cmake'}
    
    # 统计信息
    total_files = 0
    converted_files = 0
    skipped_files = 0
    
    print("开始扫描文件...")
    
    # 遍历当前目录及其子目录
    for root, dirs, files in os.walk('.'):
        # 跳过隐藏目录和常见不需要的目录
        dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['build', 'cmake-build-debug', 'cmake-build-release']]
        
        for file in files:
            # 获取文件扩展名
            ext = os.path.splitext(file)[1].lower()
            
            # 检查是否为需要处理的文件类型
            if ext not in extensions:
                continue
            
            file_path = os.path.join(root, file)
            
            # 跳过符号链接
            if os.path.islink(file_path):
                continue
            
            total_files += 1
            
            print(f"正在处理: {file_path}")
            
            # 判断是否需要转换
            if should_convert(file_path):
                if convert_gbk_to_utf8(file_path):
                    converted_files += 1
                    print(f"  ✓ 已转换: {file_path}")
                else:
                    skipped_files += 1
                    print(f"  ✗ 转换失败: {file_path}")
            else:
                skipped_files += 1
                # print(f"  - 跳过: {file_path} (非GBK编码)")
    
    # 输出统计信息
    print("\n" + "="*50)
    print(f"处理完成！")
    print(f"总文件数: {total_files}")
    print(f"转换文件数: {converted_files}")
    print(f"跳过文件数: {skipped_files}")
    print("="*50)

if __name__ == "__main__":
    main()