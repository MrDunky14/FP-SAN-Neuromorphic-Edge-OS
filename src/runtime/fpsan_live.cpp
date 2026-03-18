// fpsan_live.cpp
#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <fstream>
#include <sstream>

using namespace std;

const int INPUT_DIM = 8;         
const int HIDDEN_DIM = 8;        
const int CLUSTER_DIM = 100;       

struct Neuron {
    int8_t weight;              
    float stiffness;            
    uint64_t birth_timestamp;   
    uint64_t last_fired_time;   
};

Neuron Brain_Bank_A[CLUSTER_DIM][HIDDEN_DIM]; 
Neuron Brain_Bank_B[CLUSTER_DIM][HIDDEN_DIM];
atomic<Neuron(*)[HIDDEN_DIM]> Active_Brain{Brain_Bank_A}; 

// I/O Pipeline Waiting Room
mutex sensor_mutex;
vector<int8_t> shared_sensor_buffer(INPUT_DIM, 0);
bool new_data_available = false;
bool os_running = true;

uint64_t get_current_time_ms() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

// --- Serialization Module ---
void save_brain_state() {
    ofstream out("aegis.brain", ios::binary);
    out.write(reinterpret_cast<char*>(Active_Brain.load()), sizeof(Neuron) * CLUSTER_DIM * HIDDEN_DIM);
    out.close();
    cout << "\n[Storage] Brain state safely serialized to disk." << endl;
}

bool load_brain_state() {
    ifstream in("aegis.brain", ios::binary);
    if (!in) return false; 
    in.read(reinterpret_cast<char*>(Active_Brain.load()), sizeof(Neuron) * CLUSTER_DIM * HIDDEN_DIM);
    in.close();
    return true;
}

// --- The Math (Unsupervised Clustering) ---
void process_sensory_input(const vector<int8_t>& input_data, Neuron (*target_bank)[HIDDEN_DIM]) {
    int best_cluster_id = -1;
    int first_empty_cluster_id = -1;
    float highest_similarity = -9999.0f;
    uint64_t current_time = get_current_time_ms();

    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (target_bank[c][0].last_fired_time == 0) {
            if (first_empty_cluster_id == -1) first_empty_cluster_id = c;
            continue; 
        }

        int similarity_score = 0;
        int active_sensors = 0;
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0 || target_bank[c][i].weight != 0) { 
                if (target_bank[c][i].weight == input_data[i]) similarity_score += 1;
                else similarity_score -= 1;
                active_sensors++;
            }
        }
        if (active_sensors > 0) {
            float normalized_score = (float)similarity_score / active_sensors;
            if (normalized_score > highest_similarity) {
                highest_similarity = normalized_score;
                best_cluster_id = c;
            }
        }
    }

    if (highest_similarity < 0.10f && first_empty_cluster_id != -1) {
        best_cluster_id = first_empty_cluster_id;
    }

    if (best_cluster_id != -1) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0) {
                target_bank[best_cluster_id][i].weight = input_data[i];
                target_bank[best_cluster_id][i].stiffness = 1.0f; 
            }
        }
        target_bank[best_cluster_id][0].last_fired_time = current_time;
        cout << "[Cortex] Data locked to Cluster " << best_cluster_id << endl;
    }
}

// --- Thread 1: Live Hardware Listener ---
void hardware_listener_loop() {
    string line;
    // Continuously read the standard input (simulating a UART RX pin)
    while (getline(cin, line) && os_running) {
        stringstream ss(line);
        string value;
        vector<int8_t> sensor_reading(INPUT_DIM, 0);
        int i = 0;

        while (getline(ss, value, ',') && i < INPUT_DIM) {
            sensor_reading[i] = stoi(value);
            i++;
        }

        // Drop data into the Waiting Room and lock the door
        {
            lock_guard<mutex> lock(sensor_mutex);
            shared_sensor_buffer = sensor_reading;
            new_data_available = true;
        }
    }
}

// --- Thread 2: The Cortex ---
void background_learning_loop() {
    while (os_running) {
        vector<int8_t> local_data;
        {
            lock_guard<mutex> lock(sensor_mutex);
            if (!new_data_available) {
                this_thread::yield(); 
                continue;
            }
            local_data = shared_sensor_buffer;
            new_data_available = false;
        }

        Neuron(*Shadow_Brain)[HIDDEN_DIM] = (Active_Brain.load() == Brain_Bank_A) ? Brain_Bank_B : Brain_Bank_A;
        process_sensory_input(local_data, Shadow_Brain); 
        Active_Brain.store(Shadow_Brain); // Sub-nanosecond update
    }
}

// --- Thread 3: The Brainstem (Autosave) ---
void persistence_loop() {
    while (os_running) {
        // Sleep for 10 seconds, then save the brain
        this_thread::sleep_for(chrono::seconds(10));
        save_brain_state();
    }
}

int main() {
    cout << "[System] FP-SAN v2.0 Production OS Booting..." << endl;
    
    if (load_brain_state()) {
        cout << "[System] Discovered 'aegis.brain'. Memories restored." << endl;
    } else {
        cout << "[System] Blank brain initialized." << endl;
    }

    cout << "[System] Establishing Hardware I/O Pipeline..." << endl;

    thread listener(hardware_listener_loop);
    thread cortex(background_learning_loop);
    thread storage(persistence_loop);

    // Keep the main thread alive. In a real embedded system, this runs forever.
    listener.join();
    cortex.join();
    storage.join();

    return 0;
}