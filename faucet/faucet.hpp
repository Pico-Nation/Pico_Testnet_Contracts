#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>

#include <string>
#include <optional>

#include </home/user/Desktop/contracts/try/contracts/eos/faucet/eosio.token/include/eosio.token/eosio.token.hpp>
#include </home/user/Desktop/contracts/try/contracts/eos/faucet/eosio.system/include/eosio.system/exchange_state.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("faucet")]] faucet : public contract {
public:
    using contract::contract;

    
    faucet( name receiver, name code, eosio::datastream<const char*> ds )
        : contract( receiver, code, ds )
    {}


[[eosio::action]]
void reverse(name to)
{
    //----------Faucet-------------
    // The faucet will be one of the key features of Pico. 

    //--------Debuging-Code---------
    //string faucet = "faucet";
    //symbol TESTsym = static_cast<eosio::symbol>(uint64_t("TEST"));
    //asset quantity = reinterpret_cast<asset>(500.0000, TEST);
    
    symbol TESTsym = symbol("TEST", 4);                                     // Contains the symbol of the currency to faucet as well as the percision.
    name from = static_cast<eosio::name>("faucet");                         // Name of the faucet contract account that will send the coins out.
    
    // Need to up the amount thats given from the faucet.
    
    asset quantity = asset(500.0000, TESTsym );                             // Creates the amount sent out per interaction and what it's symbol is.
    string memo = "This is a test!";                                        // Memo - maybe will be a timestamp of when the account claimed it.
    token::transfer_action transfer("eosio.token"_n, {from, "active"_n});   // Sets up the transfer from the faucet to the account that requested.
    transfer.send(from, to, quantity, memo);                                // Pushes transaction to the chain.
    
}


[[eosio::action]]
    void faucetbalance( const name owner ){

    const symbol sym(symbol_code("TEST"), 4);
    const auto my_balance = eosio::token::get_balance("eosio.token"_n, get_self(), sym.code());
    print("my balance is ", my_balance);

}

  

private:


};
