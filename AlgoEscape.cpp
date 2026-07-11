#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <stack>
#include <limits>
#include <random>
#include <conio.h>
#include<fstream>

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RESET   "\033[0m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BRED    "\033[91m"
#define ORANGE  "\033[38;5;208m"

using namespace std;

struct Player {
    string username;
    int score;
};

// Save or update leaderboard
void saveLeaderboard(const string& username, int score) {
    vector<Player> players;
    string line;

    // Read existing leaderboard
    ifstream infile("leaderboard.txt");
    while (getline(infile, line)) {
        size_t pos = line.rfind(" "); // last space separates score
        if (pos != string::npos) {
            string name = line.substr(0, pos);
            int existingScore = stoi(line.substr(pos + 1));
            players.push_back({name, existingScore});
        }
    }
    infile.close();

    bool found = false;
    for (auto &p : players) {
        if (p.username == username) {
            if (score > p.score) p.score = score; // update only if higher
            found = true;
            break;
        }
    }

    if (!found) {
        players.push_back({username, score}); // add new user
    }

    // Write back leaderboard
    ofstream outfile("leaderboard.txt", ios::trunc);
    for (auto &p : players) {
        outfile << p.username << " " << p.score << endl;
    }
}


void saveGame(const string& user, int score, int hints, int roomId, int riddleIndex) {
    ofstream file(user + "_save.txt");
    if (!file) return;
 file << "score: " << score << endl;
    file << "hints: " << hints << endl;
    file << "roomId: " << roomId << endl;
    file << "riddleIndex: " << riddleIndex << endl;
}

bool loadGame(const string& user, int &score, int &hints, int &roomId, int &riddleIndex) {
    ifstream file(user + "_save.txt");
    if (!file.is_open()) return false;

    string line;

    while (getline(file, line)) {
        size_t delim = line.find(':');
        if (delim == string::npos) continue;
        string key = line.substr(0, delim);
        string value = line.substr(delim + 1);
        value.erase(0, value.find_first_not_of(" \t")); // remove leading spaces

        if (key == "score") score = stoi(value);
        else if (key == "hints") hints = stoi(value);
        else if (key == "roomId") roomId = stoi(value);
        else if (key == "riddleIndex") riddleIndex = stoi(value);
    }

    return true;
}



// Convert string to lowercase
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Print riddle
void printRiddle(const string& riddle) {
    cout << BLUE << riddle << RESET << endl;
}

// Get hidden password input with asterisks
string getHiddenPassword(const string& prompt = "Enter password: ") {
    string password;
    char ch;
    
    cout << prompt;
    
    while (true) {
        ch = _getch();   // read character without echo
        
        if (ch == 13) { // Enter key
            cout << endl;
            break;
        } 
        else if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b"; // move back, replace with space, move back again
            }
        } 
        else if (ch == 27) { // Escape key - exit
            cout << endl;
            password = "";
            break;
        }
        else {
            password += ch;
            cout << "*";
        }
    }
    return password;
}

// Get hidden input for any field with asterisks
string getHiddenInput(const string& prompt = "Enter text: ") {
    string input;
    char ch;
    
    cout << prompt;
    
    while (true) {
        ch = _getch();   // read character without echo
        
        if (ch == 13) { // Enter key
            cout << endl;
            break;
        } 
        else if (ch == 8) { // Backspace
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
            }
        } 
        else if (ch == 27) { // Escape key - exit
            cout << endl;
            input = "";
            break;
        }
        else {
            input += ch;
            cout << "*";
        }
    }
    return input;
}

// Login System - Always asks for new credentials
bool loginSystem(string &currentUser, string &currentPassword, bool &isNewUser) {
    cout << CYAN << "\n=== WELCOME TO ALGOESCAPE ===" << RESET << endl;
    cout << "1. Create new account\n2. Login with existing account\nChoice: ";
    string choice;
    getline(cin, choice);

    if (choice == "1") {
        // --- Create new account ---
        cout << "Enter username: ";
        getline(cin, currentUser);

        currentPassword = getHiddenPassword("Create password: ");

        // Save user credentials as username:password
        ofstream users("users.txt", ios::app);
        if (!users) {
            cout << RED << "Error creating users file!\n" << RESET;
            return false;
        }
        users << currentUser << ":" << currentPassword << endl;
        users.close();
        isNewUser = true;

        cout << GREEN << "\nAccount created successfully!\n" << RESET;
        cout << "Username: " << currentUser << endl;
        cout << "Password length: " << currentPassword.length() << " characters\n";
        return true;
    }
    else if (choice == "2") {
        // --- Login existing user ---
        cout << "Enter username: ";
        getline(cin, currentUser);
        string pass = getHiddenPassword("Enter password: ");

        ifstream users("users.txt");
        if (!users) {
            cout << RED << "No user database found! Please create an account first.\n" << RESET;
            return false;
        }

        string line;
        bool found = false;
        while (getline(users, line)) {
            size_t delimPos = line.find(':');
            if (delimPos != string::npos) {
                string uname = line.substr(0, delimPos);
                string upass = line.substr(delimPos + 1);

                if (uname == currentUser && upass == pass) {
                    currentPassword = pass;
                    isNewUser = false;
                    found = true;
                    break;
                }
            }
        }
        users.close();

        if (!found) {
            cout << RED << "Invalid username or password!\n" << RESET;
            return false;
        }

        cout << GREEN << "\nLogin successful! Welcome back, " << currentUser << "!\n" << RESET;
        return true;
    }
    else {
        cout << RED << "Invalid choice!\n" << RESET;
        return false;
    }
}

// ---------------- Room Structure ----------------
struct Room {
    string name;
    vector<string> riddles;
    vector<string> hints;
    vector<vector<string>> answers;
    bool isFinal;
    bool isEscape;
    string flavorText;
    Room* left;
    Room* right;
    int id;

    Room(string n, vector<string> r, vector<string> h, vector<vector<string>> a, bool f, bool e, string flavor ,int roomId) {
        name = n;
        riddles = r;
        hints = h;
        answers = a;
        isFinal = f;
        isEscape = e;
        flavorText = flavor;
        left = nullptr;
        right = nullptr;
        id = roomId;
    }
};
void showLeaderboard() {
    ifstream file("leaderboard.txt");
    if (!file) {
        cout << "No leaderboard found yet.\n";
        return;
    }

    vector<Player> players;
    string name;
    int score;

    while (file >> name >> score) {
        players.push_back({name, score});
    }

    // Sort descending by score
    sort(players.begin(), players.end(), [](const Player &a, const Player &b) {
        return a.score > b.score;
    });

    cout << "\n===== LEADERBOARD =====\n";
    cout << "Rank\tUsername\tScore\n";

    for (size_t i = 0; i < players.size(); i++) {
        cout << (i+1) << "\t" << players[i].username << "\t\t" << players[i].score << "\n";
    }

    cout << "=======================\n";
}


// ---------------- Solve Room ----------------
bool solveRoom(Room* room, int &score, int &hintsLeft, const string& currentUser, int &startRiddleIndex)
{
    cout << "\n---------------------------------\n";
    cout << ORANGE << currentUser << ", You Entered: " << room->name << endl << RESET;
    cout << room->flavorText << endl;

    // If this room has no riddles or is a trap room
   if (room->riddles.empty() && !room->isEscape && room->isFinal) {
    cout << RED << "\nThis room is a trap! There is no way out.\n" << RESET;
    return false;
}


    // Continue solving riddles as usual
    vector<int> indices(room->riddles.size());
    for (size_t i = 0; i < indices.size(); i++) indices[i] = i;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(indices.begin(), indices.end(), default_random_engine(seed));

    for (size_t idx = startRiddleIndex; idx < indices.size(); idx++) {
        int i = indices[idx];
        string riddle = room->riddles[i];
        string hint = room->hints[i];
        vector<string> correctAnswers = room->answers[i];

        printRiddle(riddle);

        int attempts = 2;
        bool hintUsed = false;
        auto startTime = chrono::steady_clock::now();

        while (attempts > 0) {
            string userAnswer;
            cout << "\nYour answer: ";
            getline(cin, userAnswer);
            userAnswer = toLower(userAnswer);

            if (userAnswer == "save") {
                saveGame(currentUser, score, hintsLeft, room->id, idx);
                 saveLeaderboard(currentUser, score); // <-- add this
                cout << GREEN << "\nGame saved! You can resume next time.\n" << RESET;
                return false;
            }

            bool correct = false;
            for (auto& ans : correctAnswers) {
                if (userAnswer == toLower(ans)) { correct = true; break; }
            }

            if (correct) {
                auto endTime = chrono::steady_clock::now();
                int timeTaken = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
                int bonus = max(0, 10 - timeTaken);
                score += 5 + bonus;
                cout << GREEN << "Correct! You earned 5 points";
                if (bonus > 0) cout << " + " << bonus << " bonus points!";
                cout << RESET << endl;
                cout << "Current Score: " << score << endl;
                break;
            } else {
                attempts--;
                score -= 2;
                cout << RED << "Wrong! You lost 2 points." << RESET << endl;
                cout << "Current Score: " << score << endl;

                if (attempts > 0 && hintsLeft > 0 && !hintUsed) {
                    string choice;
                    cout << "Do you want a hint? (yes/no): ";
                    getline(cin, choice);
                    if (toLower(choice) == "yes") {
                        cout << " Hint: " << hint << endl;
                        hintsLeft--;
                        score -= 1;
                        hintUsed = true;
                        cout << YELLOW << "[Hint] -1 point. Current Score: " << score << RESET << endl;
                    }
                }
            }

            if (attempts == 0) {
                cout << RED << "\n[Failed] You couldn't solve this riddle.\n" << RESET;
                 saveLeaderboard(currentUser, score); // <-- add this
                return false;
            }
        }
        startRiddleIndex = 0;
    }

    cout << MAGENTA << "\nAll riddles in this room solved!" << RESET << endl;
    return true;
}



// ---------------- Play Game ----------------
void playGame(Room* startRoom, const string& currentUser, int score, int hintsLeft, int savedRiddleIndex = 0)
{
    Room* current = startRoom;
    int riddleIndex = savedRiddleIndex;
    stack<Room*> history;
    vector<string> path;

    while (current != nullptr) {
        path.push_back(current->name);

        // Solve riddles in the current room
        bool solved = solveRoom(current, score, hintsLeft, currentUser, riddleIndex);
        if (!solved) return; // User saved and exited, stop game

        riddleIndex = 0; // Reset for next room

        // Check if final room
        if (current->isFinal) break;

        // ---------------- Movement ----------------
        char move;
        Room* nextRoom = nullptr;
        while (true) {
            cout << "\nChoose path:\n";
            cout << "L -> Left\nR -> Right\nB -> Back\nS -> Save & Exit\nChoice: ";
            cin >> move;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (move == 'L' || move == 'l') {
                if (current->left) {
                    history.push(current);
                    nextRoom = current->left;
                } else {
                    cout << YELLOW << "No path in that direction!\n" << RESET;
                    continue;
                }
                break;
            }
            else if (move == 'R' || move == 'r') {
                if (current->right) {
                    history.push(current);
                    nextRoom = current->right;
                } else {
                    cout << YELLOW << "No path in that direction!\n" << RESET;
                    continue;
                }
                break;
            }
            else if (move == 'B' || move == 'b') {
                if (history.empty()) {
                    cout << YELLOW << "You are already at the entrance — can't go back.\n" << RESET;
                    continue;
                } else {
                    nextRoom = history.top();
                    history.pop();
                    cout << CYAN << "You moved back to: " << nextRoom->name << RESET << endl;
                    break;
                }
            }
            else if (move == 'S' || move == 's') {
                saveGame(currentUser, score, hintsLeft, current->id, riddleIndex);
                cout << GREEN << "\nGame saved! You can resume next time.\n" << RESET;
                return; // exit immediately
            }
            else {
                cout << "Invalid choice. Please choose L, R, B, or S.\n";
            }
        }

        current = nextRoom;
    }

    // ---------------- End of Game ----------------
    cout << "\n=====================================\n";
    cout << YELLOW << "Player: " << currentUser << endl << RESET;
    cout << "Rooms Visited: ";
    for (size_t i = 0; i < path.size(); i++) {
        cout << path[i];
        if (i != path.size() - 1) cout << " -> ";
    }
    cout << MAGENTA << "\nFINAL SCORE: " << score << endl << RESET;
    cout << "Hints remaining: " << hintsLeft << endl;
    cout << "=====================================\n";

    saveLeaderboard(currentUser, score);


}

// ---------------- Main ----------------
int main() {
    srand(time(0));

    cout << "=====================================\n";
    cout << CYAN << "   ALGOESCAPE - THE RIDDLE MAZE\n" << RESET;
    cout << "=====================================\n";

    // ---------------- Login ----------------
  string currentUser, currentPassword;
bool loggedIn = false;
bool isNewUser = false;  // flag to track if account is new

// ---------------- Login ----------------
while (!loggedIn) {
    loggedIn = loginSystem(currentUser, currentPassword, isNewUser);
}

    bool sameAccount = true;
    while (true) {

        // ---------------- Rooms ----------------
        Room* room1 = new Room(
            "Entrance Hall",
            {"I speak without a mouth and hear without ears.",
             "I'm tall when I'm young and short when I'm old.",
             "I have cities, but no houses. I have mountains, but no trees. I have water, but no fish."},
            {"You hear me when I repeat what you say.",
             "I melt slowly as I give light.",
             "I help you travel."},
            {{"echo"}, {"candle"}, {"map"}},
            false, false,
            "The grand entrance is dimly lit with flickering torches." , 1
        );

        Room* room2 = new Room(
            "Candle Chamber",
            {"I can be cracked, made, told, and played.",
             "I'm light as a feather, yet the strongest man cannot hold me for long.",
             "I am always in front of you but can't be seen.",
             "What can run but never walks, has a mouth but never talks?"}, 
            {"Sometimes I make you laugh.",
             "You breathe me in and out every second.",
             "You move forward every day.",
             "It flows endlessly."},
            {{"joke"}, {"breath"}, {"future"}, {"river"}},
            false, false,
            "Warm light flickers around you.", 2
        );

        Room* room3 = new Room(
            "Map Gallery",
            {"What has a head and a tail,but no body",
            "I have words but never speak.",
            "What building has the most stories",
            "I am used to write and erase."},
            {"You can keep it in your pocket and spend it.",
             "I am read silently.",
             "It's a place full of books, where every “story” can be read.",
             "Students use me in class."},
            {{"coin"}, {"book"}, {"library"}, {"pencil"}},
            false, false,
            "Ancient maps adorn the walls.", 3
        );

        Room* room4 = new Room(
            "Fire Tunnel",
            {
                "I can roar but have no mouth. I can lick but have no tongue. What am I?", 
                "I am bright during the day and night, but I am not the sun. What am I?",
                "I rise without legs and move without wings. What am I?",
                "I vanish quickly but leave smoke behind. What am I?"
            },
            {
                "I move and crackle in the hearth.",
                "I am seen flickering in flames.",
                "I dance upwards in the air.",
                "I disappear but make the air smoky."
            },
            {
                {"fire"}, {"flame"}, {"smoke"}, {"spark"}
            },
            false, false,
            "Heat rises as flames dance on the walls, illuminating the tunnel in warm colors." , 4
        );

        Room* room5 = new Room(
            "Mirror Hall (Final Escape)",
            {
                "I follow you everywhere but never leave footprints. What am I?", 
                "I can make you taller, shorter, thinner, or fatter, but I am not alive. What am I?",
                "I show your face but never speak. What am I?",
                "You look at me to see yourself, but I cannot see back. What am I?"
            },
            {
                "Think of something that mimics your movements.",
                "I reflect your image exactly.",
                "I show what you look like without saying a word.",
                "Everyone uses me to check their appearance."
            },
            {
                {"shadow"}, {"mirror"}, {"reflection"}, {"mirror"}
            },
            true, true,
            "Endless mirrors create a dizzying maze, challenging your mind and your eyes." , 5
        );

      Room* room6 = new Room(
    "Dark Trap Room",
    {}, // no riddles
    {}, // no hints
    {}, // no answers
    true,  // isFinal
    false, // not escape
    "You entered the wrong path. There is no way out. The room closes behind you, cold and empty.", 
    6
);
        // ---------------- Link Rooms ----------------
        room1->left  = room2;
        room1->right = room3;
        room2->left  = room4;
        room2->right = room5;
        room3->left  = room6;
        room3->right = room4;
        room4->left  = room5;
        room4->right = room5;

        // ---------------- Resume Saved Game ----------------
       int savedScore = 0, savedHints = 3, savedRoomId = 1, savedRiddleIndex = 0;
Room* startRoom = room1; // default

if (!isNewUser && loadGame(currentUser, savedScore, savedHints, savedRoomId, savedRiddleIndex)) {
    cout << GREEN << "\nWelcome back, " << currentUser << "! Resuming your last game...\n" << RESET;
}

        if (loadGame(currentUser, savedScore, savedHints, savedRoomId, savedRiddleIndex)) {
            switch (savedRoomId) {
                case 1: startRoom = room1; break;
                case 2: startRoom = room2; break;
                case 3: startRoom = room3; break;
                case 4: startRoom = room4; break;
                case 5: startRoom = room5; break;
                case 6: startRoom = room6; break;
                default: startRoom = room1;
            }
            cout << GREEN << "\nWelcome back, " << currentUser << "! Resuming your last game...\n" << RESET;
        }

        playGame(startRoom, currentUser, savedScore, savedHints, savedRiddleIndex);

 
    // Setup rooms and play game

         // ---------------- Menu After Game ----------------
        while (true) {
            int menuChoice;
            cout << YELLOW << "\nWhat do you want to do next?\n" << RESET;
            cout << "1. Play again with same account\n";
            cout << "2. Play with different account\n";
            cout << "3. Show Leaderboard\n";
            cout << "4. Exit\n";
            cout << "Choice: ";
            cin >> menuChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

         if (menuChoice == 1) {
    sameAccount = true;
    break; // restart game loop with same account
} else if (menuChoice == 2) {
    sameAccount = false;
    loggedIn = false;
    break; // go back to login
} else if (menuChoice == 3) {
    showLeaderboard();
    continue; // show menu again without restarting
} else if (menuChoice == 4) {
    cout << GREEN << "Exiting game. Thanks for playing!\n" << RESET;
    return 0; // exit program
} else {
    cout << RED << "Invalid choice! Enter a number 1-4.\n" << RESET;
}
        }

        // Clean up memory
        delete room1;
        delete room2;
        delete room3;
        delete room4;
        delete room5;
        delete room6;

    } 

    return 0;
}
