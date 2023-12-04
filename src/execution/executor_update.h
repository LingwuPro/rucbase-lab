/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class UpdateExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;
    std::vector<Condition> conds_;
    RmFileHandle *fh_;
    std::vector<Rid> rids_;
    std::string tab_name_;
    std::vector<SetClause> set_clauses_;
    SmManager *sm_manager_;

    std::vector<ColMeta> cols_;

   public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;

        cols_ = tab_.cols;
    }
    std::unique_ptr<RmRecord> Next() override {
        // Update each rid from record file and index file
        for (auto &rid : rids_) {
            auto rec = fh_->get_record(rid, context_);
            for (auto &set_clause : set_clauses_) {
                auto lhs_col = tab_.get_col(set_clause.lhs.col_name);
                memcpy(rec->data + lhs_col->offset, set_clause.rhs.raw->data, lhs_col->len);
            }
            // Remove old entry from index
            for (size_t i = 0; i < tab_.indexes.size(); ++i) {
                auto &index = tab_.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (size_t j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->delete_entry(key, context_->txn_);
            }
            // Update record in record file
            fh_->update_record(rid, rec->data, context_);
            // Insert new index into index
            for (size_t i = 0; i < tab_.indexes.size(); ++i) {
                auto &index = tab_.indexes[i];
                auto ih =
                    sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (size_t j = 0; j < index.col_num; ++j) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->insert_entry(key, rid, context_->txn_);
            }
        }
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }

    void setData(const Rid &rid, std::unique_ptr<RmRecord> rec) {
        char *buf = rec->data;
        for (SetClause set_clause : set_clauses_) {
            auto col = get_col(cols_, set_clause.lhs);  // 获取对应列元数据
            if (col->type != set_clause.rhs.type) {
                if (col->type == TYPE_INT && set_clause.rhs.type == TYPE_FLOAT) {  // float转int
                    set_clause.rhs.type = TYPE_INT;
                    set_clause.rhs.set_int(set_clause.rhs.float_val);
                } else if (col->type == TYPE_FLOAT && set_clause.rhs.type == TYPE_INT) {  // int转float
                    set_clause.rhs.type = TYPE_FLOAT;
                    set_clause.rhs.set_float(set_clause.rhs.int_val);
                } else {
                    throw IncompatibleTypeError(coltype2str(col->type), coltype2str(set_clause.rhs.type));
                }
            }
            set_clause.rhs.init_raw(col->len);
            memcpy(buf + col->offset, set_clause.rhs.raw->data, col->len);
        }
        fh_->update_record(rid, buf, context_);
    }

    bool condCheck(const RmRecord *l_record) {
        char *l_val_buf, *r_val_buf;
        const RmRecord *r_record;

        for (auto &condition : conds_) {  // 条件判断
            CompOp op = condition.op;
            int cmp;

            // record和col确定数据位置
            auto l_col = get_col(cols_, condition.lhs_col);  // 左列元数据
            l_val_buf = l_record->data + l_col->offset;      // 确定左数据起点

            if (condition.is_rhs_val) {  // 值
                r_record = condition.rhs_val.raw.get();
                r_val_buf = r_record->data;

                cmp = cond_compare(l_val_buf, r_val_buf, condition.rhs_val.type, l_col->len);
            } else {  // 列
                auto r_col = get_col(cols_, condition.rhs_col);
                r_val_buf = l_record->data + r_col->offset;

                cmp = cond_compare(l_val_buf, r_val_buf, r_col->type, l_col->len);
            }
            if (!op_compare(op, cmp))  // 不满足条件
                return false;
        }
        return true;
    }

    int cond_compare(const char *l_val_buf, const char *r_val_buf, ColType type, int col_len) const {
        int cmp = ix_compare(l_val_buf, r_val_buf, type, col_len);
        return cmp;
    }

    bool op_compare(CompOp op, int cmp) const {
        if (op == OP_EQ) {
            return cmp == 0;
        } else if (op == OP_NE) {
            return cmp != 0;
        } else if (op == OP_LT) {
            return cmp < 0;
        } else if (op == OP_GT) {
            return cmp > 0;
        } else if (op == OP_LE) {
            return cmp <= 0;
        } else if (op == OP_GE) {
            return cmp >= 0;
        } else {
            throw InternalError("Invalid CompOp");
        }
    }

    size_t tupleLen() const override { return 0; };
    const std::vector<ColMeta> &cols() const override { return cols_; };
    void beginTuple() override{};
    void nextTuple() override{};
    bool is_end() const override { return true; };
    std::string getType() override { return "UpdateExecutor"; };
    ColMeta get_col_offset(const TabCol &target) override { return ColMeta(); };
};