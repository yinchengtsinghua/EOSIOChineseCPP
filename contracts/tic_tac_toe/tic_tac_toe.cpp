
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件
 *@eos/license中定义的版权
 **/

#include "tic_tac_toe.hpp"

using namespace eosio;

/*
 *@brief检查单元格是否为空
 *@param cell-单元格的值（应为0、1或2）
 *@如果单元格为空，则返回true
 **/

bool is_empty_cell(const uint8_t& cell) {
   return cell == 0;
}

/*
 *@简短检查有效移动
 *@detail movement被认为是有效的，如果它在板内并且在空单元上完成。
 *@param row-玩家的移动行
 *@param column-玩家的移动列
 *@param board-正在进行移动的板
 *@如果移动有效，返回真
 **/

bool is_valid_movement(const uint16_t& row, const uint16_t& column, const vector<uint8_t>& board) {
   uint16_t board_width = tic_tac_toe::game::board_width;
   uint16_t board_height = tic_tac_toe::game::board_height;
   uint32_t movement_location = row * board_width + column;
   bool is_valid = column < board_width && row < board_height && is_empty_cell(board[movement_location]);
   return is_valid;
}

/*
 *@brief获得比赛冠军
 *@detail本场比赛的赢家是第一个连续三次齐头并进的选手
 *@param current_游戏-我们要确定的游戏
 *@返回游戏的获胜者（可以是无/抽奖/主机帐户名/挑战者帐户名）
 **/

account_name get_winner(const tic_tac_toe::game& current_game) {
   auto& board = current_game.board;

   bool is_board_full = true;
   
   

//使用按位与运算符确定每列、行和对角线的连续值
//因为3==0b11，2==0b10，1=0b01，0=0b00
   vector<uint32_t> consecutive_column(tic_tac_toe::game::board_width, 3 );
   vector<uint32_t> consecutive_row(tic_tac_toe::game::board_height, 3 );
   uint32_t consecutive_diagonal_backslash = 3;
   uint32_t consecutive_diagonal_slash = 3;
   for (uint32_t i = 0; i < board.size(); i++) {
      is_board_full &= !is_empty_cell(board[i]);
      uint16_t row = uint16_t(i / tic_tac_toe::game::board_width);
      uint16_t column = uint16_t(i % tic_tac_toe::game::board_width);

//计算连续的行和列值
      consecutive_row[column] = consecutive_row[column] & board[i]; 
      consecutive_column[row] = consecutive_column[row] & board[i];
//计算连续对角线值
      if (row == column) {
         consecutive_diagonal_backslash = consecutive_diagonal_backslash & board[i];
      }
//计算连续对角线/值
      if ( row + column == tic_tac_toe::game::board_width - 1) {
         consecutive_diagonal_slash = consecutive_diagonal_slash & board[i]; 
      }
   }

//检查所有连续行、列和对角线的值，并确定获胜者。
   vector<uint32_t> aggregate = { consecutive_diagonal_backslash, consecutive_diagonal_slash };
   aggregate.insert(aggregate.end(), consecutive_column.begin(), consecutive_column.end());
   aggregate.insert(aggregate.end(), consecutive_row.begin(), consecutive_row.end());
   for (auto value: aggregate) {
      if (value == 1) {
         return current_game.host;
      } else if (value == 2) {
         return current_game.challenger;
      }
   }
//如果董事会已满，则抽签，否则尚未确定胜者。
   return is_board_full ? N(draw) : N(none);
}

/*
 *@brief应用创建操作
 **/

void tic_tac_toe::create(const account_name& challenger, const account_name& host) {
   require_auth(host);
   eosio_assert(challenger != host, "challenger shouldn't be the same as host");

//检查游戏是否已存在
   games existing_host_games(_self, host);
   auto itr = existing_host_games.find( challenger );
   eosio_assert(itr == existing_host_games.end(), "game already exists");

   existing_host_games.emplace(host, [&]( auto& g ) {
      g.challenger = challenger;
      g.host = host;
      g.turn = host;
   });
}

/*
 *@brief应用重新启动操作
 **/

void tic_tac_toe::restart(const account_name& challenger, const account_name& host, const account_name& by) {
   require_auth(by);

//检查游戏是否存在
   games existing_host_games(_self, host);
   auto itr = existing_host_games.find( challenger );
   eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

//检查此游戏是否属于动作发送者
   eosio_assert(by == itr->host || by == itr->challenger, "this is not your game!");

//重置游戏
   existing_host_games.modify(itr, itr->host, []( auto& g ) {
      g.reset_game();
   });
}

/*
 *@brief应用关闭操作
 **/

void tic_tac_toe::close(const account_name& challenger, const account_name& host) {
   require_auth(host);

//检查游戏是否存在
   games existing_host_games(_self, host);
   auto itr = existing_host_games.find( challenger );
   eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

//删除游戏
   existing_host_games.erase(itr);
}

/*
 *@brief应用移动动作
 **/

void tic_tac_toe::move(const account_name& challenger, const account_name& host, const account_name& by, const uint16_t& row, const uint16_t& column ) {
   require_auth(by);

//检查游戏是否存在
   games existing_host_games(_self, host);
   auto itr = existing_host_games.find( challenger );
   eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

//检查此游戏是否尚未结束
   eosio_assert(itr->winner == N(none), "the game has ended!");
//检查此游戏是否属于动作发送者
   eosio_assert(by == itr->host || by == itr->challenger, "this is not your game!");
//检查是否轮到动作发送者
   eosio_assert(by == itr->turn, "it's not your turn yet!");


//检查用户是否进行了有效的移动
   eosio_assert(is_valid_movement(row, column, itr->board), "not a valid movement!");

//填充单元格，1表示主机，2表示挑战者
   const uint8_t cell_value = itr->turn == itr->host ? 1 : 2;
   const auto turn = itr->turn == itr->host ? itr->challenger : itr->host;
   existing_host_games.modify(itr, itr->host, [&]( auto& g ) {
      g.board[row * tic_tac_toe::game::board_width + column] = cell_value;
      g.turn = turn;
      g.winner = get_winner(g);
   });
}


EOSIO_ABI( tic_tac_toe, (create)(restart)(close)(move))
