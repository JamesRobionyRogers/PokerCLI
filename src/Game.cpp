#include "Game.hpp"
#include "GUI.hpp"

using namespace std;

Game::Game(std::vector<Player*>& players) {
    this->players = players;
    this->deck = Deck();
    this->deck.shuffle();
}

void Game::add_player(Player* player) {
    players.push_back(player);
}

void Game::deal_hands() {
    for (Player* player : players) {
        player->add_card_to_hand(deck.deal()); // Deal the first card to the player
        player->add_card_to_hand(deck.deal()); // Deal the second card to the player
    }
}

void Game::deal_flop() {
    deck.deal();    // Burn card
    for (int i = 0; i < 3; i++) {
        community_cards.push_back(deck.deal()); // Deal 3 cards for the flop 
    }
}

void Game::deal_turn() {
    deck.deal();    // Burn card
    community_cards.push_back(deck.deal()); // Deal 1 card for the turn
}

void Game::deal_river() {
    deck.deal();    // Burn card
    community_cards.push_back(deck.deal()); // Deal 1 card for the river
}

std::vector<Player*> Game::get_players() {
    return players;
}

void Game::awardPot(Player* winner) {
    winner->win(this->pot);
}

/**
 * Perform a move for a user based on the provided move string
 * @param move: string - The move to perform received from the user input
 * @param player: Player* - The player object to perform the move for
 * @param pot: int - The current pot size
 */
int Game::makeMoveForUser(string move, Player* player, int playerIndex, int largestBet) {

    Move currentMove = getCurrentMove(move);

    // TODO: If the player is raising, we need to check that the bet sizing is greater than the current bet plus minimum raise 
    // Also need to check that the player has enough money to make the bet

    // Perform correct move on the player's object
    switch (currentMove) {
        case Move::CALL: {

            int callAmount = largestBet - player->get_current_bet();

            player->bet(callAmount);
            
            return largestBet;
        }
        case Move::RAISE: {
            int raiseAmount = GUI::getBetSizing();
            player->bet(raiseAmount);
            return raiseAmount;
        }
        case Move::CHECK: {
            return player->get_current_bet();
        }
        case Move::FOLD: {
            player->clear_hand();
            return -1;  // Used to remove the player from the hand 
        }
    }
    
}


bool Game::test_game() {
    vector<Player*> players;
    Player player1 = Player("Jess", 1000);
    Player player2 = Player("James", 1000);

    players.push_back(&player1);
    players.push_back(&player2);

    Game game = Game(players);


    // Game should start with 52 cards
    if (game.deck.get_cards().size() != 52) {
        std::cout << "Deck does not have 52 cards" << std::endl;
        return false;
    }

    // Checking deal_hand
    game.deal_hands();
    for (Player* player : game.get_players()) {
        if (player->get_hand().size() != 2) {
            std::cout << "Player does not have 2 cards in hand" << std::endl;
            return false;
        }
    }
    if (game.deck.get_cards().size() != 48) {
        std::cout << "Deck does not have 48 cards" << std::endl;
        return false;
    }

    // Checking deal_flop
    game.deal_flop();
    if (game.community_cards.size() != 3) {
        std::cout << "Flop does not have 3 cards" << std::endl;
        return false;
    }
    if (game.deck.get_cards().size() != 44) {
        std::cout << "Deck does not have 45 cards" << std::endl;
        return false;
    }

    // Checking deal_turn
    game.deal_turn();
    if (game.community_cards.size() != 4) {
        std::cout << "Turn does not have 4 cards" << std::endl;
        return false;
    }
    if (game.deck.get_cards().size() != 42) {
        std::cout << "Deck does not have 44 cards" << std::endl;
        return false;
    }

    // Checking deal_river
    game.deal_river();
    if (game.community_cards.size() != 5) {
        std::cout << "River does not have 5 cards" << std::endl;
        return false;
    }
    if (game.deck.get_cards().size() != 40) {
        std::cout << "Deck does not have 42 cards" << std::endl;
        return false;
    }

    // Checking get_players
    if (game.get_players().size() != 2) {
        std::cout << "Game does not have 3 players" << std::endl;
        return false;
    }
    if (game.get_players()[0]->get_name() != "Jess") {
        std::cout << "Player 1 is not Jess" << std::endl;
        return false;
    }
    if (game.get_players()[1]->get_name() != "James") {
        std::cout << "Player 2 is not James" << std::endl;
        return false;
    }

    // Checking add_player
    Player player3 = Player("Corban", 1000);
    game.add_player(&player3);
    if (game.get_players().size() != 3) {
        std::cout << "Game does not have 3 players" << std::endl;
        return false;
    }

    // Free memory
    // for (Player* player : game.get_players()) {
    //     delete player;
    // }

    return true;
}

void Game::playHand() {

    // Deal cards out to the table 
    this->deal_hands();

    // Store the number of players in the hand 
    int numPlayers = this->get_players().size();

    // A vector of booleans corresponding to the players in the game - keep track of who is still in the hand
    vector<bool> inGame = vector<bool>(numPlayers, true);

    // Get the small blind and big blind players
    Player* smallBlindPlayer = this->get_players()[(button + 1) % numPlayers];
    Player* bigBlindPlayer = this->get_players()[(button + 2) % numPlayers];

    // Extract blinds from the players and add them to the pot
    this->addBlindsToPot(bigBlindPlayer, smallBlindPlayer);

    int largestBet = BIG_BLIND;
    Player *largestBetPlayer = bigBlindPlayer;


    // Perform the pre-flop betting round
    this->bettingRound(inGame, largestBet, numPlayers, largestBetPlayer, true);
    this->deal_flop();
    GUI::displayCommunityCards(community_cards);
    // Perform the flop betting round
    
    largestBetPlayer = nullptr;
    this->bettingRound(inGame, largestBet, numPlayers, largestBetPlayer, false);
    this->deal_turn();
    GUI::displayCommunityCards(community_cards);

    // Perform the turn betting round
    largestBetPlayer = nullptr;
    this->bettingRound(inGame, largestBet, numPlayers, largestBetPlayer, false);
    this->deal_river();
    GUI::displayCommunityCards(community_cards);

    // Perform the river betting round
    largestBetPlayer = nullptr;
    this->bettingRound(inGame, largestBet, numPlayers, largestBetPlayer, false);
    // Go to showdown
    // TODO: find winner (hand evaluator)
    // Award the pot to the winner
}

void Game::bettingRound(vector<bool>& inGame, int largestBet, int numPlayers, Player *largestBetPlayer, bool preflop) {
    int currentPlayer;

    //TESTING: print that this method has been called 
    cout << "Betting round has been called" << endl;
    
    if(preflop){   // UTG is first to act if pre-flop
        currentPlayer = button + 3 % numPlayers;
    } else {    // Otherwise, the player to the left of the button is first to act
        currentPlayer = button + 1 % numPlayers;
        while(!inGame[currentPlayer]){
            currentPlayer = (currentPlayer + 1) % numPlayers;
        }
    }

    // Loop for the betting round 
    while(true) {

        // End the betting round if there is only one player left in the game
        if (count(inGame.begin(), inGame.end(), true) == 1) {
            // TODO: Award the pot to the winner should be done in the playHand method as returning there will stop hand 
            Player *winner = this->get_players()[find(inGame.begin(), inGame.end(), true) - inGame.begin()];  
            this->awardPot(winner);
            return;
        }

        // If the player is still in the game
        if (inGame[currentPlayer]) {
            GUI::displayPlayerStack(this->get_players()[currentPlayer]);
            // Get the player's move
            string move = GUI::getUserMove();

            // Perform the move for the player
            int betSize = makeMoveForUser(move, this->get_players()[currentPlayer], currentPlayer, largestBet);

            // If the player has folded, update the inGame vector
            if (betSize == -1) {
                inGame[currentPlayer] = false;
                continue;
            }
            
            // Update the current bet for the player
            get_players()[currentPlayer]->bet(betSize);

            // Update the largest bet if the player has bet more than the current largest bet
            if (betSize > largestBet) {
                largestBet = betSize;
                largestBetPlayer = this->get_players()[currentPlayer];
            }
        }

        // Move to the next player
        currentPlayer = (currentPlayer + 1) % numPlayers;

        // If we have looped back to the player who made the largest bet, break
        // TODO: This does not work pre-flop if noone raises - the BB should be able to check
        if (this->get_players()[currentPlayer] == largestBetPlayer) {
            break;
        }
    }
}

/**
 * Private method used for converting a string move to a Move enum
 * @param move: string - The move to convert
 * @return Move - The converted move
 */
Move Game::getCurrentMove(std::string move) {

    // Strip the move of any whitespace
    move.erase(remove_if(move.begin(), move.end(), ::isspace), move.end());

    switch (move[0]) {
        case 'a': 
            return Move::CHECK;
        case 'c':
            return Move::CALL;
        case 'r':
            return Move::RAISE;
        case 'f':
            return Move::FOLD;
        default:
            return Move::FOLD;
    }
}


void Game::addBlindsToPot(Player *bigBlindPlayer, Player *smallBlindPlayer) {
    this->pot += bigBlindPlayer->deduct_blind(BIG_BLIND);
    this->pot += smallBlindPlayer->deduct_blind(SMALL_BLIND);
}