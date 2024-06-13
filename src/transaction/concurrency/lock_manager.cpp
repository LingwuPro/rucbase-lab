// /* Copyright (c) 2023 Renmin University of China
// RMDB is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//         http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details. */

// #include "lock_manager.h"

// /**
//  * @description: 申请行级共享锁
//  * @return {bool} 加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
//  * @param {int} tab_fd
//  */
// bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
//     // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) {
//         return false;
//     } else if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, rid, LockDataType::RECORD);
//     // 3. 查找当前事务是否已经申请了目标数据项上的锁，如果存在则根据锁类型进行操作，否则执行下一步操
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, rid, LockDataType::RECORD)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;
//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     while ((lock_table_[lock_id].group_lock_mode_ != GroupLockMode::S &&
//             lock_table_[lock_id].group_lock_mode_ != GroupLockMode::IS &&
//             lock_table_[lock_id].group_lock_mode_ != GroupLockMode::NON_LOCK)) {
//         lock_table_[lock_id].cv_.wait(lock);
//     }
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 申请行级排他锁
//  * @return {bool} 加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {Rid&} rid 加锁的目标记录ID
//  * @param {int} tab_fd 记录所在的表的fd
//  */
// bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid,
//                                            int tab_fd) {  // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) return false;
//     if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, rid, LockDataType::RECORD);
//     // 3. 查找当前事务是否已经申请了目标数据项上的锁，如果存在则根据锁类型进行操作，否则执行下一步操
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, rid, LockDataType::RECORD)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;
//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     while (lock_table_[lock_id].group_lock_mode_ != GroupLockMode::NON_LOCK) {
//         lock_table_[lock_id].cv_.wait(lock);
//     }
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     lock_table_[lock_id].group_lock_mode_ = GroupLockMode::X;
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 申请表级读锁
//  * @return {bool} 返回加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {int} tab_fd 目标表的fd
//  */
// bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {  // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) return false;
//     if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, LockDataType::TABLE);
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, LockDataType::TABLE)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;
//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     while (lock_table_[lock_id].group_lock_mode_ != GroupLockMode::S &&
//            lock_table_[lock_id].group_lock_mode_ != GroupLockMode::IS &&
//            lock_table_[lock_id].group_lock_mode_ != GroupLockMode::NON_LOCK)
//         lock_table_[lock_id].cv_.wait(lock);
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     if (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::IX)
//         lock_table_[lock_id].group_lock_mode_ = GroupLockMode::SIX;
//     else if (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::IS ||
//              lock_table_[lock_id].group_lock_mode_ == GroupLockMode::NON_LOCK)
//         lock_table_[lock_id].group_lock_mode_ = GroupLockMode::S;
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 申请表级写锁
//  * @return {bool} 返回加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {int} tab_fd 目标表的fd
//  */
// bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {  // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) return false;
//     if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, LockDataType::TABLE);
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, LockDataType::TABLE)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;
//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     while (lock_table_[lock_id].group_lock_mode_ != GroupLockMode::NON_LOCK) lock_table_[lock_id].cv_.wait(lock);
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     lock_table_[lock_id].group_lock_mode_ = GroupLockMode::X;
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 申请表级意向读锁
//  * @return {bool} 返回加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {int} tab_fd 目标表的fd
//  */
// bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {  // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) return false;
//     if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, LockDataType::TABLE);
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, LockDataType::TABLE)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;
//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     while (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::X) lock_table_[lock_id].cv_.wait(lock);
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     if (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::NON_LOCK)
//         lock_table_[lock_id].group_lock_mode_ = GroupLockMode::IS;
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 申请表级意向写锁
//  * @return {bool} 返回加锁是否成功
//  * @param {Transaction*} txn 要申请锁的事务对象指针
//  * @param {int} tab_fd 目标表的fd
//  */
// bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {  // 1. 通过mutex申请访问全局锁表
//     std::unique_lock<std::mutex> lock{latch_};
//     // 2. 检查事务的状态
//     if (txn->get_state() == TransactionState::ABORTED) return false;
//     if (txn->get_state() == TransactionState::SHRINKING) {
//         txn->set_state(TransactionState::ABORTED);
//         return false;
//     }
//     txn->set_state(TransactionState::GROWING);
//     LockDataId lock_id(tab_fd, LockDataType::TABLE);
//     // 3. 查找当前事务是否已经申请了目标数据项上的锁，如果存在则根据锁类型进行操作，否则执行下一步操
//     LockRequestQueue* lock_queue = &lock_table_[LockDataId(tab_fd, LockDataType::TABLE)];
//     auto iter = lock_queue->request_queue_.begin();
//     for (; iter != lock_queue->request_queue_.end(); ++iter)
//         if (iter->txn_id_ == txn->get_transaction_id()) break;

//     // 4. 将要申请的锁放入到全局锁表中，并通过组模式来判断是否可以成功授予锁
//     txn->get_lock_set()->insert(lock_id);
//     LockRequest* need_lock = new LockRequest(txn->get_transaction_id(), LockMode::SHARED);
//     lock_table_[lock_id].request_queue_.push_back(*need_lock);
//     // 5. 如果成功，更新目标数据项在全局锁表中的信息，否则阻塞当前操作
//     if (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::IS ||
//         lock_table_[lock_id].group_lock_mode_ == GroupLockMode::NON_LOCK)
//         lock_table_[lock_id].group_lock_mode_ = GroupLockMode::IX;
//     else if (lock_table_[lock_id].group_lock_mode_ == GroupLockMode::S)
//         lock_table_[lock_id].group_lock_mode_ = GroupLockMode::SIX;
//     need_lock->granted_ = true;
//     lock_table_[lock_id].cv_.notify_all();
//     lock.unlock();
//     // 提示：步骤5中的阻塞操作可以通过条件变量来完成，所有加锁操作都遵循上述步骤，在下面的加锁操作中不再进行注释提示
//     return true;
// }

// /**
//  * @description: 释放锁
//  * @return {bool} 返回解锁是否成功
//  * @param {Transaction*} txn 要释放锁的事务对象指针
//  * @param {LockDataId} lock_data_id 要释放的锁ID
//  */
// bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
//     std::unique_lock<std::mutex> lock(latch_);
//     txn->set_state(TransactionState::SHRINKING);
//     // 未申请
//     if (txn->get_lock_set()->find(lock_data_id) == txn->get_lock_set()->end()) return false;
//     // 查找当前事务
//     auto it = lock_table_[lock_data_id].request_queue_.begin();
//     for (; it != lock_table_[lock_data_id].request_queue_.end(); it++) {
//         if (it->txn_id_ == txn->get_transaction_id()) break;
//     }
//     GroupLockMode temp_mode = GroupLockMode::NON_LOCK;
//     it = lock_table_[lock_data_id].request_queue_.begin();
//     // update lock_table_[lock_id].group_lock_mode_
//     for (; it != lock_table_[lock_data_id].request_queue_.end(); it++) {
//         if (it->granted_ == true) {
//             switch (it->lock_mode_) {
//                 case LockMode::SHARED:
//                     if (temp_mode == GroupLockMode::SIX) continue;
//                     temp_mode = temp_mode == GroupLockMode::IX ? GroupLockMode::SIX : GroupLockMode::S;
//                     break;
//                 case LockMode::EXLUCSIVE:
//                     temp_mode = GroupLockMode::X;
//                     break;
//                 case LockMode::INTENTION_SHARED:
//                     if (temp_mode == GroupLockMode::NON_LOCK || temp_mode == GroupLockMode::IS)
//                         temp_mode = GroupLockMode::IS;
//                     break;
//                 case LockMode::INTENTION_EXCLUSIVE:
//                     if (temp_mode == GroupLockMode::SIX) continue;
//                     temp_mode = temp_mode == GroupLockMode::S ? GroupLockMode::SIX : GroupLockMode::IX;
//                     break;
//                 case LockMode::S_IX:
//                     temp_mode = GroupLockMode::SIX;
//                     break;
//             }
//         }
//     }
//     lock_table_[lock_data_id].group_lock_mode_ = temp_mode;
//     lock_table_[lock_data_id].cv_.notify_all();
//     lock.unlock();
//     return true;
// }
/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "lock_manager.h"

static inline bool check_lock(Transaction* txn) {
    // 事务已经结束，不能再获取锁
    if (txn->get_state() == TransactionState::COMMITTED || txn->get_state() == TransactionState::ABORTED) {
        return false;
    }
    // 收缩状态不允许加锁
    if (txn->get_state() == TransactionState::SHRINKING) {
        // 抛出异常，在rmdb里abort
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    // 如果之前没加过锁，更新锁模式，开始2PL第一阶段
    if (txn->get_state() == TransactionState::DEFAULT) {
        txn->set_state(TransactionState::GROWING);
    }
    return true;
}

/**
 * @description: 申请行级共享锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
 * @param {int} tab_fd
 */
bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录的锁ID
    LockDataId lockDataId(tab_fd, rid, LockDataType::RECORD);
    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    // 事务上已经有这个记录的共享锁了，判断为加锁成功
    // select 加IS和S锁，对于S锁的申请直接通过，而不是升级为S
    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            return true;
        }
    }

    // 如果其他事务持有X锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ == GroupLockMode::X ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::IX ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::SIX) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }

    // 设置队列锁模式为共享锁
    lockRequestQueue.group_lock_mode_ = GroupLockMode::S;
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::SHARED);
    ++lockRequestQueue.shared_lock_num_;
    // 成功申请共享锁
    lockRequestQueue.request_queue_.back().granted_ = true;
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录的锁ID
    LockDataId lockDataId(tab_fd, rid, LockDataType::RECORD);

    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            // select后修改
            // 多次修改
            // 修改再select
            if (lockRequest.lock_mode_ == LockMode::EXLUCSIVE) {
                return true;
            }
            // 如果事务要升级为写锁，该记录不能有其他事务在读
            else if (lockRequest.lock_mode_ == LockMode::SHARED && lockRequests.size() == 1) {
                lockRequest.lock_mode_ = LockMode::EXLUCSIVE;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::X;
                return true;
            }
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }
    }

    // 如果其他事务持有其他锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ != GroupLockMode::NON_LOCK) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }
    // 设置队列锁模式为排他锁
    lockRequestQueue.group_lock_mode_ = GroupLockMode::X;
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::EXLUCSIVE);
    // 成功申请排他锁
    lockRequestQueue.request_queue_.back().granted_ = true;
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录所在锁的请求队列
    LockDataId lockDataId(tab_fd, LockDataType::TABLE);

    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            // 在获取S锁之前，先获取IS锁或更高级别的锁
            // 如果已经有S锁或更高级的锁，申请成功
            if (lockRequest.lock_mode_ == LockMode::SHARED || lockRequest.lock_mode_ == LockMode::EXLUCSIVE ||
                lockRequest.lock_mode_ == LockMode::S_IX) {
                return true;
            }
            // 事务有IS锁，升级S需要其他事务不持有X锁
            else if (lockRequest.lock_mode_ == LockMode::INTENTION_SHARED &&
                     (lockRequestQueue.group_lock_mode_ == GroupLockMode::S ||
                      lockRequestQueue.group_lock_mode_ == GroupLockMode::IS)) {
                lockRequest.lock_mode_ = LockMode::SHARED;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::S;
                ++lockRequestQueue.shared_lock_num_;
                return true;
            }
            // 如果事务有IX锁，申请SIX需要其他事务不持有IX锁
            else if (lockRequest.lock_mode_ == LockMode::INTENTION_EXCLUSIVE && lockRequestQueue.IX_lock_num_ == 1) {
                lockRequest.lock_mode_ = LockMode::S_IX;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::SIX;
                ++lockRequestQueue.shared_lock_num_;
                return true;
            }
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }
    }

    // 如果其他事务持有排他锁或者意向排它锁或SIX锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ == GroupLockMode::X ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::IX ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::SIX) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }

    // 设置队列锁模式为共享锁
    lockRequestQueue.group_lock_mode_ = GroupLockMode::S;
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::SHARED);
    ++lockRequestQueue.shared_lock_num_;
    // 成功申请共享锁
    lockRequestQueue.request_queue_.back().granted_ = true;
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录所在锁的请求队列
    LockDataId lockDataId(tab_fd, LockDataType::TABLE);

    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            // 如果已经有排他锁，申请成功
            if (lockRequest.lock_mode_ == LockMode::EXLUCSIVE) {
                return true;
            }
            // 只允许存在一个事务才能升级表写锁
            else if (lockRequests.size() == 1) {
                lockRequest.lock_mode_ = LockMode::EXLUCSIVE;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::X;
                return true;
            }
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }
    }

    // 如果其他事务持有排他锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ != GroupLockMode::NON_LOCK) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }
    // 设置队列锁模式为排他锁
    lockRequestQueue.group_lock_mode_ = GroupLockMode::X;
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::EXLUCSIVE);
    lockRequestQueue.request_queue_.back().granted_ = true;
    // 成功申请共享锁
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 申请表级意向读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录所在锁的请求队列
    LockDataId lockDataId(tab_fd, LockDataType::TABLE);

    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    // 如果队列中已经有这个事务
    // 没有比IS锁更低级，直接申请成功
    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            return true;
        }
    }

    // 如果其他事务持有排他锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ == GroupLockMode::X) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }

    // 如果队列没有锁才设置队列锁模式为共享锁
    if (lockRequestQueue.group_lock_mode_ == GroupLockMode::NON_LOCK) {
        lockRequestQueue.group_lock_mode_ = GroupLockMode::IS;
    }
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::INTENTION_SHARED);
    lockRequestQueue.request_queue_.back().granted_ = true;
    // 成功申请共享锁
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 申请表级意向写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    std::unique_lock<std::mutex> lock(latch_);

    if (!check_lock(txn)) return false;

    // 得到记录所在锁的请求队列
    LockDataId lockDataId(tab_fd, LockDataType::TABLE);

    // 如果锁表中不存在，使用分段构造构造map
    if (lock_table_.count(lockDataId) == 0) {
        lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(lockDataId), std::forward_as_tuple());
    }
    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lockDataId);
    auto& lockRequests = lockRequestQueue.request_queue_;

    for (auto& lockRequest : lockRequests) {
        if (lockRequest.txn_id_ == txn->get_transaction_id()) {
            // 如果已经有IX锁或更高级的锁，同一事务中多个写申请
            if (lockRequest.lock_mode_ == LockMode::INTENTION_EXCLUSIVE || lockRequest.lock_mode_ == LockMode::S_IX ||
                lockRequest.lock_mode_ == LockMode::EXLUCSIVE) {
                return true;
            }
            // 如果事务有且持有唯一共享锁(where)，则升级为SIX
            // 否则有多个事务持有共享锁，无法为其中的一个事务申请写锁，因为可能会影响其他事务读出来修改后的数据
            else if (lockRequest.lock_mode_ == LockMode::SHARED && lockRequestQueue.shared_lock_num_ == 1) {
                ++lockRequestQueue.IX_lock_num_;
                lockRequest.lock_mode_ = LockMode::S_IX;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::SIX;
                return true;
            } else if (lockRequest.lock_mode_ == LockMode::INTENTION_SHARED &&
                       (lockRequestQueue.group_lock_mode_ == GroupLockMode::IS ||
                        lockRequestQueue.group_lock_mode_ == GroupLockMode::IX)) {
                ++lockRequestQueue.IX_lock_num_;
                lockRequest.lock_mode_ = LockMode::INTENTION_EXCLUSIVE;
                lockRequestQueue.group_lock_mode_ = GroupLockMode::IX;
                return true;
            }
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }
    }

    // 得到记录所在锁的请求队列
    // 如果其他事务持有共享(或SIX)锁和排它锁，则加锁失败(no-wait)
    if (lockRequestQueue.group_lock_mode_ == GroupLockMode::S ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::SIX ||
        lockRequestQueue.group_lock_mode_ == GroupLockMode::X) {
        // no-wait!
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // lockRequestQueue.cv_.wait(lock);
    }
    // 设置队列锁模式为意向排他锁
    lockRequestQueue.group_lock_mode_ = GroupLockMode::IX;
    // 添加当前事务的锁请求到队列中
    lockRequestQueue.request_queue_.emplace_back(txn->get_transaction_id(), LockMode::INTENTION_EXCLUSIVE);
    ++lockRequestQueue.IX_lock_num_;
    // 成功申请意向排他锁
    lockRequestQueue.request_queue_.back().granted_ = true;
    txn->get_lock_set()->emplace(lockDataId);
    return true;
}

/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {Transaction*} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    std::unique_lock<std::mutex> lock(latch_);

    // 事务已经结束，不能再释放锁
    if (txn->get_state() == TransactionState::COMMITTED || txn->get_state() == TransactionState::ABORTED) {
        return false;
    }
    // 之前没有获得锁
    if (txn->get_state() == TransactionState::DEFAULT) {
        // 抛出异常，在rmdb里abort
        // throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    // 如果之前加过锁，更新锁模式，开始2PL第二阶段
    if (txn->get_state() == TransactionState::GROWING) {
        txn->set_state(TransactionState::SHRINKING);
    }

    // 找不到锁请求队列
    if (lock_table_.count(lock_data_id) == 0) {
        return true;
    }

    // 得到锁ID所在的锁请求队列和队列上的所有锁请求
    auto& lockRequestQueue = lock_table_.at(lock_data_id);
    auto& lockRequests = lockRequestQueue.request_queue_;

    auto it = lockRequests.begin();
    for (; it != lockRequests.end(); ++it) {
        if (it->txn_id_ == txn->get_transaction_id()) {
            break;
        }
    }
    // 找不到事务对应的锁请求
    // 先判断才能擦除，擦除后it会变成野指针
    if (it == lockRequests.end()) return true;

    // 更新队列S和IX数量
    if (it->lock_mode_ == LockMode::SHARED || it->lock_mode_ == LockMode::S_IX) {
        --lockRequestQueue.shared_lock_num_;
    }
    if (it->lock_mode_ == LockMode::INTENTION_EXCLUSIVE || it->lock_mode_ == LockMode::S_IX) {
        --lockRequestQueue.IX_lock_num_;
    }
    // 删除事务的锁请求
    lockRequests.erase(it);

    if (lockRequests.empty()) {
        lockRequestQueue.group_lock_mode_ = GroupLockMode::NON_LOCK;
        return true;
    }

    // 锁队列不为空则肯定有锁，从最低级的IS找
    LockMode new_mode = LockMode::INTENTION_SHARED;
    for (auto& lockRequest : lockRequests) {
        new_mode =
            static_cast<LockMode>(std::max(static_cast<int>(new_mode), static_cast<int>(lockRequest.lock_mode_)));
    }
    lockRequestQueue.group_lock_mode_ = static_cast<GroupLockMode>(static_cast<int>(new_mode) + 1);
    return true;
}