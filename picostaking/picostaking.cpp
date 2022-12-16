#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

//--------------------Pico Staking Contract---------------------------------

class [[eosio::contract("stake")]] stake : public contract {
 public:
  using contract::contract;

  stake(name receiver, name code, datastream<const char*> ds)
      : contract(receiver, code, ds),
        _stakes(receiver, receiver.value) {}

  [[eosio::action]] void deposit(name user, asset quantity){                             // Deposit function of the staking contract
    require_auth(user);                                                                   // checks for user auth
    check(quantity.is_valid(), "invalid quantity");                                       
    check(quantity.amount > 0, "must deposit a positive amount");
    check(quantity.symbol == symbol("PICO", 4), "invalid asset symbol");

    _stakes.emplace(get_self(), [&](auto& stake) {
      stake.user = user;
      stake.quantity = quantity;
      stake.timestamp = current_time_point();                                       // Places the start time stamp in the contract table 
    });
  }

  [[eosio::action]] void withdraw(name user, asset quantity) {
    require_auth(user);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must withdraw a positive amount");
    check(quantity.symbol == symbol("PICO", 4), "invalid asset symbol");

    auto itr = _stakes.find(user.value);
    check(itr != _stakes.end(), "stake not found");
    check(itr->quantity.amount >= quantity.amount, "insufficient stake balance");

    _stakes.modify(itr, get_self(), [&](auto& stake) {
      stake.quantity -= quantity;
    });

    action(
        permission_level{get_self(), "active"_n},
        get_self(),
        "transfer"_n,
        std::make_tuple(get_self(), user, quantity, std::string("withdraw stake")))
        .send();
  }

  [[eosio::action]] void claim(name user) {
    require_auth(user);

    auto itr = _stakes.find(user.value);
    check(itr != _stakes.end(), "stake not found");

    asset quantity = itr->quantity;
    int64_t duration = (current_time_point() - itr->timestamp).to_seconds();
    int64_t seconds_per_year = 60 * 60 * 24 * 365;
    int64_t years = duration / seconds_per_year;
    int64_t apy = 24; // initial APY
    for (int64_t i = 0; i < years / 3; i++) {                                          // apy halves every 3 years intill it hits 3%
      apy /= 2;
      if (apy < 3) {
        apy = 3;
        break;
      }
    }
    int64_t reward = (quantity.amount * apy * duration) / (seconds_per_year * 100);
    check(reward > 0, "no reward to claim");

action(
	permission_level{get_self(), "active"_n},
	get_self(),
	"transfer"_n,
	std::make_tuple(get_self(), user, asset(reward, quantity.symbol),
	std::string("claim reward")))
	.send();
	
    _stakes.modify(itr, get_self(), [&](auto& stake) {
      stake.quantity += asset(reward, quantity.symbol);
      });
      

 private:
  struct [[eosio::table]] stake_info {
    name user;
    asset quantity;
    time_point_sec timestamp;

    uint64_t primary_key() const { return user.value; }
  };
  typedef eosio::multi_index<"stakes"_n, stake_info> stakes_table;

  stakes_table _stakes;
};

}
//EOSIO_DISPATCH(stake, (deposit)(withdraw)(claim))

