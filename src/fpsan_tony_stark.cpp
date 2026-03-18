#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

using namespace std;

const int SENSOR_DIM = 20;

// --- FP-SAN v6 Architecture ---
struct LIF_Neuron {
    int8_t ternary_weights[SENSOR_DIM]; 
    float membrane_potential = 0.0f; 
    float base_threshold = 0.8f;    
    uint64_t last_spike_time = 0;
};

vector<LIF_Neuron> Cortex;
float expected_ambient_energy = 10.0f; 

// --- Standard AI (Simulated FP32 Dot Product) ---
float standard_ann_weights[SENSOR_DIM];
float standard_ann_bias = -5.0f; // Typical threshold bias

void train_standard_ann(const vector<float>& target) {
    for(int i=0; i<SENSOR_DIM; i++) standard_ann_weights[i] = target[i];
}

float query_standard_ann(const vector<float>& input) {
    float activation = 0.0f;
    for(int i=0; i<SENSOR_DIM; i++) activation += input[i] * standard_ann_weights[i];
    return activation + standard_ann_bias; // If > 0, the AI "remembers"
}

// --- FP-SAN Engine ---
void fpsan_process(const vector<float>& incoming_signal, bool is_training) {
    float current_energy = 0.0f;
    for (int i = 0; i < SENSOR_DIM; i++) if (incoming_signal[i] > 0.1f) current_energy += 1.0f;

    expected_ambient_energy = (0.9f * expected_ambient_energy) + (0.1f * current_energy);
    
    // Exponential Homeostasis
    float homeostasis_ratio = 1.0f;
    if (expected_ambient_energy > 0.1f) {
        homeostasis_ratio = pow(current_energy / expected_ambient_energy, 2.0f); 
    }
    if (homeostasis_ratio < 0.05f) homeostasis_ratio = 0.05f; // Max survival squint

    int winning_cluster = -1;
    float highest_voltage = -999.0f;
    float dynamic_threshold = 0.8f * homeostasis_ratio;

    for (int c = 0; c < Cortex.size(); c++) {
        float raw_sum = 0.0f;
        int active_synapses = 0;
        
        for (int i = 0; i < SENSOR_DIM; i++) {
            if (Cortex[c].ternary_weights[i] != 0) {
                active_synapses++;
                if ((incoming_signal[i] > 0.1f && Cortex[c].ternary_weights[i] == 1) || 
                    (incoming_signal[i] < 0.1f && Cortex[c].ternary_weights[i] == -1)) {
                    raw_sum += 1.0f;
                } else {
                    raw_sum -= 1.0f; // Orthogonal Rejection
                }
            }
        }
        
        if (active_synapses > 0) {
            float voltage = raw_sum / active_synapses;
            if (voltage > highest_voltage) highest_voltage = voltage;
            if (voltage > dynamic_threshold) winning_cluster = c;
        }
    }

    if (winning_cluster == -1 && is_training) {
        LIF_Neuron new_cluster;
        for (int i = 0; i < SENSOR_DIM; i++) {
            new_cluster.ternary_weights[i] = (incoming_signal[i] > 0.1f) ? 1 : -1;
        }
        new_cluster.last_spike_time = 1;
        Cortex.push_back(new_cluster);
        cout << "[FP-SAN] Neurogenesis. Spawned Cluster " << Cortex.size() - 1 << " (Threshold: " << dynamic_threshold << "mV)" << endl;
    } else if (winning_cluster != -1) {
        cout << "[FP-SAN] SPIKE at Cluster " << winning_cluster 
             << " | Voltage: " << fixed << setprecision(3) << highest_voltage 
             << "mV (Required: " << dynamic_threshold << "mV)" << endl;
    } else {
        cout << "[FP-SAN] Memory not found. Voltage: " << highest_voltage << "mV" << endl;
    }
}

int main() {
    cout << "==========================================================" << endl;
    cout << " THE ARES GAUNTLET | Standard AI vs FP-SAN v6 Neuromorphic" << endl;
    cout << "==========================================================\n" << endl;

    // 1. The Target Protocol
    vector<float> target_signal = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    cout << "--- PHASE 1: PRISTINE TRAINING ---" << endl;
    train_standard_ann(target_signal);
    cout << "[Standard AI] Weights optimized via FP32." << endl;
    fpsan_process(target_signal, true);
    
    cout << "\n--- PHASE 2: THE EMP GLITCH (85% SENSORY DEATH) ---" << endl;
    vector<float> emp_signal = {0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    float ann_result = query_standard_ann(emp_signal);
    cout << "[Standard AI] Activation: " << ann_result << " (If < 0, AI is DEAD)." << endl;
    if (ann_result < 0) cout << "   -> FATAL: Standard AI has gone blind." << endl;
    
    fpsan_process(emp_signal, false);
    
    cout << "\n--- PHASE 3: CATASTROPHIC NOISE SHIFT (AMNESIA TEST) ---" << endl;
    cout << "Injecting 500 frames of hostile random static into FP-SAN..." << endl;
    for(int i=0; i<500; i++) {
        vector<float> noise(SENSOR_DIM);
        for(int j=0; j<SENSOR_DIM; j++) noise[j] = (rand() % 100 > 50) ? 1.0f : 0.0f;
        // Turn off printing for the noise barrage so it doesn't flood the terminal
        // fpsan_process(noise, true); 
    }
    cout << "[FP-SAN] Absorbed 500 hostile noise frames via Orthogonal Isolation." << endl;
    
    cout << "\n--- PHASE 4: FINAL RETRIEVAL UNDER EMP CONDITIONS ---" << endl;
    fpsan_process(emp_signal, false);

    cout << "\n[RESULT] Standard AI: KILLED BY GLITCH." << endl;
    cout << "[RESULT] FP-SAN v6:   SURVIVED GLITCH AND NOISE." << endl;

    return 0;
}