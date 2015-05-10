// Dave Ross
// CIS 266 Final Exam (take home)
// Hunt the Wumpus
// 12-14-99

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

using namespace std;

class Room
{
	private:
		int number;
		int numConnectedRooms;
		Room *connectedRooms[10];
		
		string shortName, longName, stringConnectedRooms;
		string tempString;

	public:

		// Returns the room's number as an int
		inline int getRoomNumber() const { return number; }

		// Returns the # of rooms connected to this one
		inline int getNumConnectedRooms() const { return numConnectedRooms; }

		// Returns the short name/description of the room
		inline string getShortName() const { return shortName; }

		// Returms the long name/description of the room
		inline string getLongName() const { return longName; }

		// Constructor
		Room(int num = -1, string strConRooms = "", string sName = "", string lName = "")
		{
			number = num;
			stringConnectedRooms = strConRooms;
			shortName = sName;
			longName = lName;

			numConnectedRooms = 0;
		}

		// Returns a string containing all connected rooms separated by ';'
		inline string getStringConnectedRooms() const {	return stringConnectedRooms; }

		// Returns the short and long names/descriptions of the room
		inline string const describeFull() { return (shortName + " (" + longName + ')'); }

		// Connect this room to another
		void connect(Room *connectedRoom)
		{
			if(numConnectedRooms < 10) connectedRooms[numConnectedRooms++] = connectedRoom;
			connectedRooms[numConnectedRooms-1]->getRoomNumber();
		}

		// Is this room connected to another?
		bool isConnectedTo(Room *connectedRoom)
		{
			int i;
			for(i=0; i < numConnectedRooms; ++i) if(connectedRooms[i] == connectedRoom) return true;
			return false;
		}
};

class Entity
{
	protected:
		Room *location;

		bool alive;

	public:

		// Constructor
		Entity()
		{
			alive = true;
		}

		// Is this entity/creature alive?
		inline bool isAlive() const { return alive; }

		// Where is this entity?
		int getLocation() const { return location->getRoomNumber(); }

		// This function is used for placing the creature at the beginning.
		// It does no validation.  Use with caution.
		inline void teleport(Room *destination) { location = destination; }

		// Kill this entity.  Be sure to check isAlive() to make sure the
		// entity has not been kill()-ed
		inline void kill() { alive = false; } 

};

class Mover:public Entity
{
	public:

		// A wrapper around teleport() to check if the destination
		// is connected to the current room.
		bool walk(Room *destination)
		{
			int foo;
			bool valid = false;

			string temp = destination->getStringConnectedRooms();

			for(foo = 0; foo < location->getNumConnectedRooms(); foo++)
			{
				if(location->isConnectedTo(destination)) valid = true;	
			}
			if(valid == true) this->teleport(destination);

			return valid;
		}
};

class Cave
{
	private:
		Room *rooms[20];
		int numRooms;
		string tempString;

		// Coonect the rooms together
		void refreshLinks()
		{
			int i = 0;
			char *temp = new char[80];

			while(i < numRooms)
			{
				char *token;

				char *conRooms = new char[80];
				rooms[i]->getStringConnectedRooms().copy(conRooms,80);

				token = strtok(conRooms,";");
				while(token != NULL)
				{
					int foo = strtol(token,&temp,10);
					rooms[i]->connect(rooms[foo]);
					token = strtok(NULL, ";");

//					cout << endl << "Connecting room " << i << " to room " << foo;
				}
				++i;
			}
		}

	public:

		// Constructor
		Cave()
		{
			numRooms = 0;
			tempString = "";
		}

		// Returns the # of rooms in the cave
		int getSize() const { return numRooms; }

		// Returns a pointer to the specified room
		Room *getRoomPointer(int roomNum) { return rooms[roomNum]; }

		//Builds the cave, then calls refreshLinks()
		int build(fstream *configFile)
		{
			char inputString[80];
			int inputRoomNum;
			string inputDestRooms, inputShortName, inputLongName;
			char *temp = new char[80];

			while( !configFile->eof() && numRooms < 20)
			{

				configFile->getline(inputString,80,':');
				inputRoomNum = strtol(inputString,&temp,10);

				configFile->getline(inputString,80,':');
				inputDestRooms = inputString;

				configFile->getline(inputString,80,':');
				inputShortName = inputString;

				configFile->getline(inputString,80,'\n');
				inputLongName = inputString;

				rooms[numRooms] = new Room(inputRoomNum, inputDestRooms, inputShortName, inputLongName);

				++numRooms;
			}

			refreshLinks(); // Link the rooms

			return numRooms;
		}

		// Returns a string containing the rooms connected to this one
		string connectedRooms(int roomNum) const { return rooms[roomNum]->getStringConnectedRooms(); }
};

class Wumpus:public Mover
{
	public:
		// Constructor
		Wumpus() {}

		// Moves the wumpus to a random room, or keeps it in the same room
		void goForAWalk(Cave *cave)
		{
			// If the walk() function fails, we'll assume the wumpus is being
			// lazy and doesn't want to go anywhere.  Really.  Honest.
			this->walk(cave->getRoomPointer((rand() % cave->getSize())));
		}

};

class Player:public Mover
{
	private:
		
		int numArrows;

	public:

		// Constructor
		Player()
		{
			numArrows = 5;
		}

		// Prototype for the shot() function.  See below.
		void Player::shoot(int roomNum, Wumpus *wumpus);
};

class Bat:public Mover
{
	public:

		// teleport() the player to a random room, then move the bat to a
		// random room
		void snatch(Player *player, Cave *cave)
		{
			player->teleport(cave->getRoomPointer(rand() % cave->getSize()));
			// If the bat stays in the same room, it'll cause an infinite loop
			this->teleport(cave->getRoomPointer(rand() % cave->getSize()));
			return;
		}
};

// Allows the player to shoot the wumpus
void Player::shoot(int roomNum, Wumpus *wumpus)
{
	if(numArrows-- == 0)
	{
		cout << endl;
		cout << endl << "You are out of arrows.";
		cout << endl << "You wander the cave aimlessly and die from malnutrition.";
		cout << endl << "Or maybe you were eaten by a wumpus.  Which is good for the wumpus.";
		cout << endl << "Now he wont die of malnutrition.  Or something like that.";
		cout << endl << endl;
		cout << endl << "If you were trying to lose, consider this a success.  Then seek help.";

		this->kill();
		return;
	}
	
	if(roomNum == wumpus->getLocation())
	{
		wumpus->kill();
	}
	else cout << endl << endl << "Missed." << endl;

	return;
}

void tellStory()
{
	cout << endl;
	cout << "You are standing in an open field west of a white house, with a boarded front";
	cout << endl;
	cout << "door.  A strange old man appears in the doorway.  He is wearing faded denim";
	cout << endl;
	cout << "pants and a t-shirt upon which is written the number \"2600\", obviously of";
	cout << endl;
	cout << "some  numerological importance.  Curiously, the old man looks you over.";
	cout << endl;
	cout << "\"Are...are you a wizard?,\" you inquire meekly.";
    cout << endl << endl;
	cout << "\"No, foolish child.  I am a Woz-ard.  From the land of Cupertino.  And you";
	cout << endl;
	cout << "are standing on my lawn.  Normally, I'd have turned you into a newt by now,";
	cout << endl;
	cout << "but....\"  He disappears and re-emerges carrying a box that is glowing a";
	cout << endl;
	cout << "bright shade of blue.  The box opens, revealing a golden apple.  \"This is a";
	cout << endl;
	cout << "teleportation spell I have been working on.  I call it the Apple I.  Eat";
	cout << endl;
	cout << "this apple, accept my challenge, and I will let you live.\"";
	cout << endl << endl;
	cout << "\"There is a cave in these parts.  I had used this as my workshop, but it";
	cout << endl;
	cout << "recently became the home of a most vile, smelly creature called a wumpus.";
	cout << endl;
	cout << "I need you to rid my workshop of this menace.\"  You gulp, then reach out and";
	cout << endl;
	cout << "take the apple in your hand.  You bring it to your lips and bite it.  The";
	cout << endl;
	cout << "world shimmers and you find yourself in a cave, armed with a bow and 5";
	cout << endl;
	cout << "arrows.";
	cout << endl << endl;
	cout << "You wonder what have you gotten yourself into...";

	return;
}

void main()
{

	string inputFile;
	char fileBanner[80];
	string shootmove;

	// Seed the random number generator so it's actually pseudo-random
	srand(time(NULL));

	// Introduction
//	cout << endl << "Welcome to Wumpus2000.";
	tellStory();

	// Load a configuration file
	cout << endl << "What maze datafile would you like to use?: ";
	cin.clear(); cin  >> inputFile;
	cout << endl << "Opening " << inputFile << "...";
	fstream *configFile = new fstream(inputFile.data());
	if(configFile->fail())
	{
		cout << endl << "The specified config file " << inputFile << " does not exist.";
		exit(1);
	}

	// Build the cave
	Cave cave;
	configFile->getline(fileBanner,80);
	cout << endl << fileBanner;
	cout << endl << cave.build(configFile) << " rooms loaded.";
	cout << endl;
	configFile->close();

	// Create the wumpus & put it somewhere
	Entity pit;
	pit.teleport(cave.getRoomPointer((rand() % cave.getSize())));

	// Create the wumpus & put it somewhere
	Wumpus wumpus;
	wumpus.teleport(cave.getRoomPointer((rand() % cave.getSize())));

	// Create the player & put it somewhere
	Player player;
	player.teleport(cave.getRoomPointer((rand() % cave.getSize())));

	// Create the player & put it somewhere
	Bat bat;
	bat.teleport(cave.getRoomPointer((rand() % cave.getSize())));
	
	int destination = 0;

	while(true)
	{
		// Move the wumpus
		wumpus.goForAWalk(&cave);
		
		//		cout << endl << "The wumpus is in room " << wumpus.getLocation();
//		cout << endl << "The pit is in room " << pit.getLocation();
//		cout << endl << "The bat is in room " << bat.getLocation();
		cout << endl << "You are in room " << player.getLocation() << ": " << cave.getRoomPointer(player.getLocation())->describeFull();	
		cout << endl << "Tunnels lead to rooms: " << cave.connectedRooms(player.getLocation());

		// Are we in a room next to the pit?
		if(cave.getRoomPointer(player.getLocation())->isConnectedTo(cave.getRoomPointer(pit.getLocation()))) cout << endl << "You feel a draft.";

		// Are we in a room next to the wumpus?
		if(cave.getRoomPointer(player.getLocation())->isConnectedTo(cave.getRoomPointer(wumpus.getLocation()))) cout << endl << "You smell a wumpus.";

		// Are we in a room next to the bat?
		if(cave.getRoomPointer(player.getLocation())->isConnectedTo(cave.getRoomPointer(bat.getLocation()))) cout << endl << "You hear bats.";


		// Did we enter the same room as the pit?
		if(pit.getLocation() == player.getLocation())
		{
			cout << endl;
			cout << endl << "Uh-oh.  You have fallen into a bottomless pit."
				 << endl << "If you were trying to lose, consider this a success.  Then seek help.";
			cout << endl;
			player.kill();
			break;
		}

		// Did we enter the same room as the wumpus?
		if(wumpus.getLocation() == player.getLocation())
		{
			cout << endl;
			cout << endl << "Uh-oh.  You woke the Wumpus!"
				 << endl << "You have been eaten by the Wumpus."
				 << endl << "If you were trying to lose, consider this a success.  Then seek help.";
			cout << endl;
			player.kill();
			break;
		}


		// Prompt for action
		cout << endl << "(S)hoot or (M)ove?: ";
		cin  >> shootmove;
		shootmove = toupper(shootmove.data()[0]);

		if(shootmove == "M")
		{
			// Where to, Mack?
			cout << endl << "Where to?: ";
			cin  >> destination;
			if(destination == player.getLocation())
				cout << endl << "Wherever you go, there you are." << endl;
			else if(!player.walk(cave.getRoomPointer(destination)))
				cout << endl << "You cannot go there." << endl;

			// Only move the wumpus if a valid command has been entered
			wumpus.goForAWalk(&cave);
		}
		else if(shootmove == "S")
		{
			cout << endl << "Shoot where?: ";
			cin  >> destination;
			if(!cave.getRoomPointer(player.getLocation())->isConnectedTo(cave.getRoomPointer(destination)))
			{
				cout << endl;
				cout << endl << "You cannot shoot there from here.";
				cout << endl;
			}
			else
			{
				player.shoot(destination,&wumpus);
			}


		}

		else cout << endl << endl << "Invalid Command." << endl;

		// Did we win?
		if(!wumpus.isAlive() || !player.isAlive()) break;

		
		// Did we enter the same room as the bat?
		if(player.getLocation() == bat.getLocation())
		{
			bat.snatch(&player,&cave);
			cout << endl;
			cout << endl << "You have been snatched up by a Super Bat!";
			cout << endl << "The bat teleports you to...room "
				 << player.getLocation() << '.';
			cout << endl;
		}
	}

	// It's dead, Jim
	if(!wumpus.isAlive())
	{
		cout << endl;
		cout << endl << "Congratulations!  You have killed the wumpus.";
		cout << endl;
		cout << endl << "     \"And hast thou slain the Jabberwock?";
		cout << endl << "      Come to my arms, my beamish boy!";
		cout << endl << "      O frabjous day! Callooh! Callay!\"";
		cout << endl << "      He chortled in his joy.";
		cout << endl;
		cout << endl << "\"It seems very pretty\", she said when she had finished it,";
		cout << endl << "\"but it's rather hard to understand!\" (You see, she didn't";
		cout << endl << "like to confess, even to herself, that she couldn't make it";
		cout << endl << "out at all.) \"Somehow it seems to fill my head with ideas-";
		cout << endl << "-only I don't exactly know what they are! However, somebody ";
		cout << endl << "killed something: that's clear, at any rate--\"";
		cout << endl;
		cout << endl << "                     Through the Looking Glass, Lewis Carroll";

	}

	// Return to the OS level
	return;
}


