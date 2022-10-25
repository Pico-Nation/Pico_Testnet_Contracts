void faucet::request( const name owner )
{
    asset quantity = asset{ 500, symbol{"TEST", 4} };

    auto request_itr = _request.find( owner.value );

   /*   // update owner's timestamp
    if (request_itr == _request.end()) {
        _request.emplace( get_self(), [&]( auto & row ) {
            row.owner      = owner;
            row.timestamp  = current_time_point();
        });
    } else {
        // enforce 24 hour faucet limit
        check( request_itr->timestamp < time_point_sec(current_time_point()) - 60 * 60 * 24, "owner can only request faucet every 24 hours");
        _request.modify( request_itr, get_self(), [&]( auto & row ) {
            row.timestamp  = current_time_point();
        });
    } */

    // check existing balance of faucet
    asset balance = token::get_balance( "faucet"_n, get_self(), symbol_code{"TEST"} );
    check( balance.amount >= quantity.amount, "faucet is empty, please check again later" );

    // external actions
    token::transfer_action token_transfer( "faucet"_n, { get_self(), "active"_n });

    // issue & transfer XY tokens to user
    token_transfer.send( get_self(), owner, quantity, "eosio.token::faucet:request");
}




