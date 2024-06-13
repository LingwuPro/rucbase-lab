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

class NestedLoopJoinExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> left_;    // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_;   // 右儿子节点（需要join的表）
    size_t len_;                                // join后获得的每条记录的长度
    std::vector<ColMeta> cols_;                 // join后获得的记录的字段

    std::vector<Condition> fed_conds_;          // join条件
    bool isend;

   public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right, 
                            std::vector<Condition> conds) {
        left_ = std::move(left);
        right_ = std::move(right);
        len_ = left_->tupleLen() + right_->tupleLen();
        cols_ = left_->cols();
        auto right_cols = right_->cols();
        for (auto &col : right_cols) {
            col.offset += left_->tupleLen();
        }

        cols_.insert(cols_.end(), right_cols.begin(), right_cols.end());
        isend = false;
        fed_conds_ = std::move(conds);

    }

    bool is_end() const override { return left_->is_end(); }

    size_t tupleLen() const override { return len_; }

    const std::vector<ColMeta> &cols() const override { return cols_; }

	void beginTuple() override {
        left_->beginTuple();
        if (left_->is_end()) {
            return;
        }
        right_->beginTuple();
        while (!is_end()) {
            if (eval_conds(cols_, fed_conds_, left_->Next().get(), right_->Next().get())) {
                break;
            }
            right_->nextTuple();
            if (right_->is_end()) {
                left_->nextTuple();
                right_->beginTuple();
            }
        }
    }

    void nextTuple() override {
        assert(!is_end());
        right_->nextTuple();
        if (right_->is_end()) {
            left_->nextTuple();
            right_->beginTuple();
        }
        while (!is_end()) {
            if (eval_conds(cols_, fed_conds_, left_->Next().get(), right_->Next().get())) {
                break;
            }
            right_->nextTuple();
            if (right_->is_end()) {
                left_->nextTuple();
                right_->beginTuple();
            }
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        assert(!is_end());
        auto record = std::make_unique<RmRecord>(len_);
        auto left_record = left_->Next();
        auto right_record = right_->Next();
        memcpy(record->data, left_record->data, left_record->size);
        memcpy(record->data + left_record->size, right_record->data, right_record->size);
        return record;
    }

    Rid &rid() override { return _abstract_rid; }

	bool eval_cond(const std::vector<ColMeta> &rec_cols, const Condition &cond, const RmRecord *lrec,
                   const RmRecord *rrec) {
        auto lhs_col = get_col(rec_cols, cond.lhs_col);
        char *lhs = lrec->data + lhs_col->offset;
        char *rhs;
        ColType rhs_type;
        if (cond.is_rhs_val) {
            rhs_type = cond.rhs_val.type;
            rhs = cond.rhs_val.raw->data;
        } else {
            // rhs is a column
            auto rhs_col = get_col(rec_cols, cond.rhs_col);
            rhs_type = rhs_col->type;
            rhs = rrec->data + rhs_col->offset - left_->tupleLen();
        }
        assert(rhs_type == lhs_col->type);
        int cmp = ix_compare(lhs, rhs, rhs_type, lhs_col->len);
        if (cond.op == OP_EQ) {
            return cmp == 0;
        } else if (cond.op == OP_NE) {
            return cmp != 0;
        } else if (cond.op == OP_LT) {
            return cmp < 0;
        } else if (cond.op == OP_GT) {
            return cmp > 0;
        } else if (cond.op == OP_LE) {
            return cmp <= 0;
        } else if (cond.op == OP_GE) {
            return cmp >= 0;
        } else {
            throw InternalError("Unexpected op type");
        }
    }

    bool eval_conds(const std::vector<ColMeta> &rec_cols, const std::vector<Condition> &conds, const RmRecord *lrec,
                    const RmRecord *rrec) {
        return std::all_of(conds.begin(), conds.end(),
                           [&](const Condition &cond) { return eval_cond(rec_cols, cond, lrec, rrec); });
    }
};