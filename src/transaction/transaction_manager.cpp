/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "transaction_manager.h"

#include "record/rm_file_handle.h"
#include "system/sm_manager.h"

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};

/**
 * @description: 事务的开始方法
 * @return {Transaction*} 开始事务的指针
 * @param {Transaction*} txn 事务指针，空指针代表需要创建新事务，否则开始已有事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
Transaction *TransactionManager::begin(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 判断传入事务参数是否为空指针
    // 2. 如果为空指针，创建新事务
    // 3. 把开始事务加入到全局事务表中
    // 4. 返回当前事务指针

    std::scoped_lock lock(latch_);

    if (txn == nullptr) {
        txn = new Transaction(next_txn_id_++);
    }
    txn_map[txn->get_transaction_id()] = txn;
    return txn;
}

/**
 * @description: 事务的提交方法
 * @param {Transaction*} txn 需要提交的事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
void TransactionManager::commit(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 如果存在未提交的写操作，提交所有的写操作
    // 2. 释放所有锁
    // 3. 释放事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态

    std::scoped_lock lock(latch_);

    auto write_set = txn->get_write_set();
    write_set->clear();
    auto lock_set = txn->get_lock_set();
    for (auto lock : *lock_set) {
        lock_manager_->unlock(txn, lock);
    }
    lock_set->clear();

    txn->set_state(TransactionState::COMMITTED);
}

/**
 * @description: 事务的终止（回滚）方法
 * @param {Transaction *} txn 需要回滚的事务
 * @param {LogManager} *log_manager 日志管理器指针
 */
void TransactionManager::abort(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 回滚所有写操作
    // 2. 释放所有锁
    // 3. 清空事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态

    std::scoped_lock lock(latch_);

    auto write_set = txn->get_write_set();
    Context *context = new Context(lock_manager_, log_manager, txn);
    while (!write_set->empty()) {
        auto &item = write_set->back();
        WType type = item->GetWriteType();
        if (type == WType::INSERT_TUPLE) {
            auto &tab_name = item->GetTableName();
            auto &rid = item->GetRid();
            auto tab = sm_manager_->db_.get_table(tab_name);
            auto rec = sm_manager_->fhs_.at(tab_name)->get_record(rid, context);
            // Delete index
            for (size_t i = 0; i < tab.indexes.size(); ++i) {
                auto &index = tab.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (int j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->delete_entry(key, context->txn_);
            }
            // Delete record file
            sm_manager_->fhs_.at(tab_name).get()->delete_record(rid, context);
        } else if (type == WType::DELETE_TUPLE) {
            auto &tab_name = item->GetTableName();
            auto &rec = item->GetRecord();
            auto tab = sm_manager_->db_.get_table(tab_name);
            // Insert into record file
            auto rid = sm_manager_->fhs_.at(tab_name)->insert_record(rec.data, context);
            // Insert into index
            for (size_t i = 0; i < tab.indexes.size(); ++i) {
                auto &index = tab.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (int j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->insert_entry(key, rid, context->txn_);
            }
        } else if (type == WType::UPDATE_TUPLE) {
            auto &tab_name = item->GetTableName();
            auto &rid = item->GetRid();
            auto &record = item->GetRecord();
            auto tab = sm_manager_->db_.get_table(tab_name);
            auto rec = sm_manager_->fhs_.at(tab_name)->get_record(rid, context);
            // Delete index
            for (size_t i = 0; i < tab.indexes.size(); ++i) {
                auto &index = tab.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (int j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->delete_entry(key, context->txn_);
            }
            // Insert into index
            for (size_t i = 0; i < tab.indexes.size(); ++i) {
                auto &index = tab.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (int j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, record.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->insert_entry(key, rid, context->txn_);
            }
            sm_manager_->fhs_.at(tab_name)->update_record(rid, record.data, context);
        }
        write_set->pop_back();
    }

    auto lock_set = txn->get_lock_set();
    for (auto lock : *lock_set) {
        lock_manager_->unlock(txn, lock);
    }
    lock_set->clear();
    txn->set_state(TransactionState::ABORTED);
}