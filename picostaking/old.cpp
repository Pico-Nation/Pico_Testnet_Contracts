#include <string>
#include <optional>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>


using namespace eosio;
using namespace std;

class [[eosio::contract("picostaking")]] picostaking : public contract {
public:
    using contract::contract;

    
  //  picostaking( name receiver, name code, eosio::datastream<const char*> ds )
  //     : contract( receiver, code, ds )
  // {}



  //--------------------Pico Staking Contract------------------------------

[[eosio::action]]
void stake(name staker, asset quantity {

    require_auth(staker);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must stake a positive quantity");

    // look up the staker's balance
    balances_table balances(_self, _self.value);
    auto itr = balances.find(staker.value);
    if (itr == balances.end()) {
      // create a new balance for the staker
      balances.emplace(staker, [&](auto& row) {
        row.staker = staker;
        row.balance = quantity;
        row.last_reward_time = current_time_point();
      });
    } else {
      // add the stake to the existing balance
      balances.modify(itr, staker, [&](auto& row) {
        row.balance += quantity;
      });
    }
    
}


[[eosio::action]]
    void unstake(name staker, asset quantity) {

    require_auth(staker);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must unstake a positive quantity");

    // look up the staker's balance
    balances_table balances(_self, _self.value);
    auto itr = balances.find(staker.value);
    check(itr != balances.end(), "cannot unstake tokens that are not staked");
    check(itr->balance >= quantity, "insufficient staked balance");

    // subtract the unstake from the balance
    balances.modify(itr, staker, [&](auto& row) {
      row.balance -= quantity;
    });

}

[[eosio::action]]
   void claimrewards(name staker) {
   
    require_auth(staker);

    // look up the staker's balance
    balances_table balances(_self, _self.value);
    auto itr = balances.find(staker.value);
    check(itr != balances.end(), "cannot claim rewards for an unstaked balance");

    // calculate the rewards earned since the last claim
    auto reward_rate = calculate_reward_rate(itr->balance);
    auto reward_amount = calculate_rewards(itr->last_reward_time, reward_rate);

    // distribute the rewards
    distribute_rewards(staker, reward_amount);

    // update the last reward time
    balances.modify(itr, staker, [&](auto& row) {
      row.last_reward_time = current_time_point();
    });
  }
  
  [[eosio::action]]
     void distribute(name staker, asset reward) {
    require_auth(_self);
    check(reward.is_valid(), "invalid reward");
    check(reward.amount > 0, "must distribute a positive reward");

    // send the rewards to the staker by creating a transaction and pushing it to the pico blockchain
    action(
      permission_level{_self, "active"_n},
      reward_token_contract, "transfer"_n,
      std::make_tuple(_self, staker, reward, std::string("reward for staking"))
    ).send();
  }

private:
TABLE balance_t {
    name staker;
    asset balance;
    time_point_sec last_reward_time;

    uint64_t primary_key() const { return staker.value; }
  };
  typedef multi_index<"balances"_n, balance_t> balances_table;

  name reward_token_contract = "rewardtoken"_n;

  // calculates the daily reward rate for a given staked balance
  double calculate_reward_rate(asset balance) {
    // APY = 24%
    return (0.24 / 365) * balance.amount;
  }

  // calculates the rewards earned since the given time
  asset calculate_rewards(time_point_sec last_reward_time, double reward_rate) {
    // calculate the time elapsed since the last reward
    auto elapsed_time = current_time_point().sec_since_epoch() - last_reward_time.sec_since_epoch();
    // convert elapsed time to days
    auto elapsed_days = elapsed_time / (60 * 60 * 24);
    // calculate the reward amount
    auto reward_amount = reward_rate * elapsed_days;
    return asset{static_cast<int64_t>(reward_amount), reward_token_contract};
  }
};



