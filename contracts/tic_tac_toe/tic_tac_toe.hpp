
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

#include <eosiolib/eosio.hpp>

/*
 *@defgroup tictactoecontact tic tac toe合同
 *@brief定义了pvp tic-tac-toe契约示例
 *@ingroup示例合同
 *
 *@细节
 *对于以下井字游戏：
 *每对玩家可以有两个独特的游戏，一个是玩家1成为主机，玩家2成为挑战者，反之亦然。
 *-游戏数据存储在“主机”范围内，并使用“挑战者”作为密钥。
 *
 *（0,0）坐标在板的左上角
 *@代码
 *（0,2）
 *（0,0）-o_x，其中-=空单元格
 *-x-x=按主机移动
 *（2,0）x o o=挑战者移动
 *@终结码
 *
 *董事会由以下数字代表：
 *-0表示空单元格
 *-1表示主机填充的单元格
 *2表示挑战者填写的单元格
 *因此，假设x是主机，上面的板将具有以下表示形式：[0，2，1，0，1，0，1，2，2]在游戏对象内
 *
 *为了部署本合同：
 *-创建一个名为tic.tac.toe的帐户
 *-在钱包中添加tic.tac.toe密钥
 *-在tic.tac.toe帐户上设置合同
 *
 *如何玩游戏：
 *-使用“创建”操作创建游戏，您作为主机，其他帐户作为挑战者。
 *-第一次移动需要由主机完成，请使用“move”操作指定要填充的行和列进行移动。
 *-然后让挑战者移动，然后再次回到主回合，重复此操作直到确定胜利者。
 *-如果要重新启动游戏，请使用“重新启动”操作
 *-如果要在游戏结束后从数据库中清除游戏以节省一些空间，请使用“关闭”操作
 *@
 **/


class tic_tac_toe : public eosio::contract {
   public:
      tic_tac_toe( account_name self ):contract(self){}
      /*
       *@与游戏相关的简要信息
       *@abi桌上游戏i64
       **/

      struct game {
         static const uint16_t board_width = 3;
         static const uint16_t board_height = board_width;
         game() { 
            initialize_board(); 
         }
         account_name          challenger;
         account_name          host;
account_name          turn; //=主机/挑战者的帐户名
account_name          winner = N(none); //=无/抽签/主持人姓名/挑战者姓名
         std::vector<uint8_t>  board;

//用空单元初始化板
         void initialize_board() {
            board = std::vector<uint8_t>(board_width * board_height, 0);
         }

//重置游戏
         void reset_game() {
            initialize_board();
            turn = host;
            winner = N(none);
         }

         auto primary_key() const { return challenger; }
         EOSLIB_SERIALIZE( game, (challenger)(host)(turn)(winner)(board))
      };

      /*
       *@brief表定义，用于存储现有游戏及其当前状态
       **/

      typedef eosio::multi_index< N(games), game> games;

///ABI行动
///创建新游戏
      void create(const account_name& challenger, const account_name& host);

///ABI行动
///重新开始游戏
///@param按要重新启动游戏的帐户
      void restart(const account_name& challenger, const account_name& host, const account_name& by);

///ABI行动
///关闭现有游戏并将其从存储中删除
      void close(const account_name& challenger, const account_name& host);

///ABI行动
//使运动
///@param按要移动的帐户
      void move(const account_name& challenger, const account_name& host, const account_name& by, const uint16_t& row, const uint16_t& column);
      
};
///@
