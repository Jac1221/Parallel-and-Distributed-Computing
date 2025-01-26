#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>
#include <algorithm>

/*
	Logic Explanation:
	elev starts at ground floor 1 going up to 9th floor, has a passenger limit of <depending on const variable>.
	
	
	elev will first sort the queue into ascending order when entered by the user.
	example: 2 passengers entered. 1st passenger wants to go floor 8 but 2nd passenger presses floor 2
	
	once the elev reaches 9th floor, the elev will now go down. with the sorting now being descending order
	
	the elev will ignore passed floors (example: you entered 2nd floor going up when the elev is at 4th floor)
	elev will also ignore duplicate floors (why enter floor to only leave at the same one?)
	the elev's threads will wait until the elev reaches a floor (time remaining is current floor between next floor)
	
*/

using namespace std;

mutex mtx;
int currFloor = 1;
queue<int> requestFloor;
bool running = true;
bool goingUp = true;
int passengers = 0;
const int passengerLoadLimit = 3; // load limit

// Elevator functions
void elevator();
void requestFloors();

// Sorting functions
void sortQueue(queue<int> &requestFloor, int currFloor, bool goingUp);

int main() {
    cout << "Max elevator passenger is "<<passengerLoadLimit<<endl;
    cout << "Starting at floor 1 / ground floor\n";
    cout << "Enter -1 to quit\n";

    thread elev_t(elevator);
    thread request_t(requestFloors);
    elev_t.join();
    request_t.join();

    cout << "\nElevator simulation done.\n";
    return 0;
}

//function to request floors [START]
void requestFloors() {
    int i = 1;
    int floor = 0;
    do {
        cout << "Enter Floor (1-9): ";
        cin >> floor;

        if (floor == -1) {
            cout << "\nThe elevator is now shutting down\n";
            running = false;
            break;
        }

        if (floor < 1 || floor > 9) {
            cout << "\nInvalid floor request. Must be 1-9\n";
            continue;
        }

        mtx.lock();
        if ((goingUp && floor > currFloor) || (!goingUp && floor < currFloor)) { //conditional statement determining if floor is going up or down
            if (passengers < passengerLoadLimit) { //lets say elevator has a load limit 
			    requestFloor.push(floor);
			    passengers++; //means someone has entered the elevator
			    sortQueue(requestFloor, currFloor, goingUp);
			    cout << "A passenger entered. Elevator load: " << passengers << endl;
			} else {
			    cout << "\nThe elevator is now full. The elevator is now moving\n";
			}
        } else {
            cout << "Request ignored: Floor " <<floor<< " has already been passed.\n";
        }
        mtx.unlock();

    } while (running);
}

//function to simulate elevator
void elevator() {
    while (true) {
        mtx.lock();
        if (!running && requestFloor.empty()) {
            mtx.unlock();
            break;
        }

        if (!requestFloor.empty()) {
            int nextFloor = requestFloor.front();
            requestFloor.pop();

            if ((goingUp && nextFloor <= currFloor) || (!goingUp && nextFloor >= currFloor)) {
                mtx.unlock();
                continue;
            }

            mtx.unlock();

            if (currFloor != nextFloor) {
                cout<<"Elevator leaving floor "<<currFloor<<endl;
                this_thread::sleep_for(chrono::seconds(abs(currFloor - nextFloor))); //wait between floors
                
                currFloor = nextFloor;
                
                cout<<"Next Floor: "<<currFloor<<endl;
            }

            cout<<"Elevator now arrived at floor " << currFloor << endl;

            mtx.lock();
            passengers--;
            cout<<"\nA paasenger has left. Elevator load: "<<passengers<<endl;
            
            //determines whether floor reaches floor 1 or 9 to change direction going up or down
            if (currFloor == 9) {
                goingUp = false;
                cout << "\nThe elevator is now going down\n";
            } else if (currFloor == 1) {
                goingUp = true;
                cout << "\nThe elevator is now going up\n";
            }
            mtx.unlock();

        } else {
            mtx.unlock();
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
}

void sortQueue(queue<int> &requestFloor, int currFloor, bool goingUp) {
    vector<int> floors; // create temporary vector for sorting
    while (!requestFloor.empty()) {
        floors.push_back(requestFloor.front());
        requestFloor.pop();
    }

    if (goingUp) {
        sort(floors.begin(), floors.end()); // sort ascending order
    } else {
        sort(floors.rbegin(), floors.rend()); // sort descending order
    }

    for (int floor : floors) {
        requestFloor.push(floor); //push temporary vector elements into requestFloor queue after sorting
    }
}

