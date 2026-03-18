// virtual_radar.cpp
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    // We don't want cout to buffer. We want it to blast data instantly like a real wire.
    setvbuf(stdout, NULL, _IONBF, 0); 
    
    // Simulate an environmental pattern (e.g., a person walking back and forth)
    int pattern_cycle = 0;

    while (true) {
        // Generate an 8-dimensional sensor reading
        for (int i = 0; i < 8; i++) {
            // Add some noise, but keep an underlying pattern for the AI to learn
            int signal = (rand() % 100 < 20) ? 0 : ((pattern_cycle % 2 == 0) ? 1 : -1);
            cout << signal << (i == 7 ? "" : ",");
        }
        cout << "\n";
        
        pattern_cycle++;
        if (pattern_cycle > 100) pattern_cycle = 0; // Topic shifts every 100 frames

        // Hardware clock rate: 20 Hz (50ms delay)
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    return 0;
}