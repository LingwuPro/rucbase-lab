/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_scan.h"

#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle *file_handle) : file_handle_(file_handle) {
    // Todo:
    // 初始化file_handle和rid（指向第一个存放了记录的位置）
    int maxpage = file_handle_->file_hdr_.num_pages;
    int pageno = 1;
    if (maxpage > 1) {
        for (pageno = 1; pageno < maxpage; pageno++) {
            if (file_handle_->fetch_page_handle(pageno).page_hdr->num_records > 0) {
                int i = Bitmap::first_bit(1, file_handle_->fetch_page_handle(pageno).bitmap,
                                          file_handle_->file_hdr_.num_records_per_page);
                rid_.page_no = pageno;
                rid_.slot_no = i;
                return;
            }
        }
    }
    rid_ = Rid{-1, -1};
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用rid_来指向这个位置
    int maxpage = file_handle_->file_hdr_.num_pages;
    int pageno = rid_.page_no;
    int slotno = rid_.slot_no;
    for (; pageno < maxpage; pageno++) {
        int i = Bitmap::next_bit(1, file_handle_->fetch_page_handle(pageno).bitmap,
                                 file_handle_->file_hdr_.num_records_per_page, slotno);
        if (i == file_handle_->file_hdr_.num_records_per_page) {
            slotno = -1;
            continue;
        } else {
            rid_.page_no = pageno;
            rid_.slot_no = i;
            return;
        }
    }
    rid_ = Rid{-1, -1};
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    if (rid_.page_no == -1 && rid_.slot_no == -1) return true;
    return false;
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const { return rid_; }