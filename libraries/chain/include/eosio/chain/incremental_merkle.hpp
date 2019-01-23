
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/types.hpp>
#include <eosio/chain/merkle.hpp>
#include <fc/io/raw.hpp>

namespace eosio { namespace chain {

namespace detail {

/*
 *如果给定无符号整数，则返回最小的
 *2的幂，大于或等于给定数字
 *
 *@param value-无符号积分
 *@RETURN-2的最小功率，大于等于value
 **/

constexpr uint64_t next_power_of_2(uint64_t value) {
   value -= 1;
   value |= value >> 1;
   value |= value >> 2;
   value |= value >> 4;
   value |= value >> 8;
   value |= value >> 16;
   value |= value >> 32;
   value += 1;
   return value;
}

/*
 *给定-2的幂（假定正确）返回前导零的数目。
 *
 *这是一个经典的以零开头的计数，不需要
 *数学使它对任何还不是-2的力量的东西都安全。
 *
 *2的参数值和积分幂
 *@返回前导零的数目
 **/

constexpr int clz_power_2(uint64_t value) {
   int lz = 64;

   if (value) lz--;
   if (value & 0x00000000FFFFFFFFULL) lz -= 32;
   if (value & 0x0000FFFF0000FFFFULL) lz -= 16;
   if (value & 0x00FF00FF00FF00FFULL) lz -= 8;
   if (value & 0x0F0F0F0F0F0F0F0FULL) lz -= 4;
   if (value & 0x3333333333333333ULL) lz -= 2;
   if (value & 0x5555555555555555ULL) lz -= 1;

   return lz;
}

/*
 *给定多个节点返回存储它们所需的深度
 *在完全平衡的二叉树中。
 *
 *@param node_count-隐含树中的节点数
 *@返回存储它们的最小树的最大深度
 **/

constexpr int calcluate_max_depth(uint64_t node_count) {
   if (node_count == 0) {
      return 0;
   }
   auto implied_count = next_power_of_2(node_count);
   return clz_power_2(implied_count) + 1;
}

template<typename ContainerA, typename ContainerB>
inline void move_nodes(ContainerA& to, const ContainerB& from) {
   to.clear();
   to.insert(to.begin(), from.begin(), from.end());
}

template<typename Container>
inline void move_nodes(Container& to, Container&& from) {
   to = std::forward<Container>(from);
}


} ///细节

/*
 *一个平衡的merkle树，内置这样一组叶节点可以
 *在不触发内部节点重建的情况下附加到
 *表示以前节点的完整子集。
 *
 *要实现这一新节点，可能意味着一组未来节点
 *实现平衡树或实现其中一个未来节点。
 *
 *一旦一个子树只包含已实现的节点，它的子根将永远不会
 *改变。这使得基于此merkle的证明非常稳定
 *一段时间后，只需更新或添加一个
 *保持有效性的值。
 **/

template<typename DigestType, template<typename ...> class Container = vector, typename ...Args>
class incremental_merkle_impl {
   public:
      incremental_merkle_impl()
      :_node_count(0)
      {}

      incremental_merkle_impl( const incremental_merkle_impl& ) = default;
      incremental_merkle_impl( incremental_merkle_impl&& ) = default;
      incremental_merkle_impl& operator= (const incremental_merkle_impl& ) = default;
      incremental_merkle_impl& operator= ( incremental_merkle_impl&& ) = default;

      template<typename Allocator, std::enable_if_t<!std::is_same<std::decay_t<Allocator>, incremental_merkle_impl>::value, int> = 0>
      incremental_merkle_impl( Allocator&& alloc ):_active_nodes(forward<Allocator>(alloc)){}

      /*
      template<template<typename…>class othercontainer，typename…otherargs>
      增量\u merkle_impl（增量\u merkle_impl<digesttype，othercontainer，otherargs…>>其他）
      ：_node_count（其他_node_count）
      ，\u个活动的节点（其他.u个活动的节点.begin（），其他.active的节点.end（））
      {}

      增量“merkle”impl（增量“merkle”impl和其他）
      ：_node_count（其他_node_count）
      ，_active_nodes（std:：forward<decltype（_active_nodes）>（other._active_nodes））
      {}
      **/


      /*
       *将节点添加到增量树，并重新计算活动节点，以便
       *准备下一次追加。
       *
       *算法是从新节点开始，然后通过树后退。
       *对于完全实现的节点和部分实现的节点
       *实现节点我们必须在新的
       *激活节点，以便下一个附加可以获取它。完全实现的节点和
       *完全隐含的节点对活动节点没有影响。
       *
       *当节点数为-2的幂时，对于允许附加的约定，则
       *增量树的当前根始终附加到新树的末尾
       *活动节点。
       *
       *实际上，可以通过记录任何
       *与隐含节点结合。
       *
       *如果附加的节点是其对中的“左”节点，它将立即推送自己。
       *进入新的活动节点列表。
       *
       *如果新节点是“右”节点，它将开始在树中向上折叠，
       *读取并丢弃旧活动节点列表中的“左”节点数据，直到
       *变为“左”节点。然后，它必须推动当前崩溃的“顶部”
       *子树进入新的活动节点列表。
       *
       *将任何值添加到新的活动节点后，所有剩余的“左”节点
       *应按照先前活动节点中所需的顺序作为
       *上一个附加的工件。当从旧的活动节点读取它们时，
       *将需要复制到新的活动节点列表中，因为它们仍然是必需的。
       *用于将来的附录。
       *
       *因此，如果一个附加折叠整个树，而始终是“正确的”
       *节点，新的活动节点列表将为空，根据定义，树包含
       *2个节点的幂。
       *
       *无论新活动节点列表的内容如何，顶部的“折叠”值
       *已附加。如果此树不是2个节点的幂，则此节点将
       *不在下一个附加中使用，但仍作为常规访问位置
       *当前树的根。如果这是2个节点的幂，那么这个节点
       *将在下一个附加的随后折叠阶段中需要，因此，它提供双重服务
       *作为合法活动节点和根的常规存储位置的职责。
       *
       *
       *@param digest-要添加的节点
       *@return-新的根目录
       **/

      const DigestType& append(const DigestType& digest) {
         bool partial = false;
         auto max_depth = detail::calcluate_max_depth(_node_count + 1);
         auto current_depth = max_depth - 1;
         auto index = _node_count;
         auto top = digest;
         auto active_iter = _active_nodes.begin();
         auto updated_active_nodes = vector<DigestType>();
         updated_active_nodes.reserve(max_depth);

         while (current_depth > 0) {
            if (!(index & 0x1)) {
//我们正在从一个“左”值和一个隐含的“右”值崩溃，从而创建一个局部节点

//如果完全实现了这个节点，并且根据定义，我们只需要附加它。
//如果在折叠期间遇到了部分节点，则不能
//充分实现
               if (!partial) {
                  updated_active_nodes.emplace_back(top);
               }

//通过暗示“正确”值相同来计算部分实现的节点值
//到“左”值
               top = DigestType::hash(make_canonical_pair(top, top));
               partial = true;
            } else {
//我们正在从一个“右”值和一个完全实现的“左”值崩溃

//从以前的活动节点中拉一个“左”值
               const auto& left_value = *active_iter;
               ++active_iter;

//如果“right”值是一个部分节点，我们将需要复制“left”，因为将来的附录仍然需要它。
//否则，当我们折叠一个完全实现的节点时，它可以从活动节点集中删除。
               if (partial) {
                  updated_active_nodes.emplace_back(left_value);
               }

//计算节点
               top = DigestType::hash(make_canonical_pair(left_value, top));
            }

//向上移动树中的某个级别
            current_depth--;
            index = index >> 1;
         }

//附加倒塌树的顶部（又名merkle的根）
         updated_active_nodes.emplace_back(top);

//存储新的活动节点
         detail::move_nodes(_active_nodes, std::move(updated_active_nodes));

//更新节点计数
         _node_count++;

         return _active_nodes.back();

      }

      /*L
       *返回增量merkle的当前根
       *
       *@返回
       **/

      DigestType get_root() const {
         if (_node_count > 0) {
            return _active_nodes.back();
         } else {
            return DigestType();
         }
      }

//私人：
      uint64_t                         _node_count;
      Container<DigestType, Args...>   _active_nodes;
};

typedef incremental_merkle_impl<digest_type>               incremental_merkle;
typedef incremental_merkle_impl<digest_type,shared_vector> shared_incremental_merkle;

} } ///EOSIO：链

FC_REFLECT( eosio::chain::incremental_merkle, (_active_nodes)(_node_count) );
