#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdint>
#include <chrono>

using namespace std;

// --- 1. Unified Dimensions ---
// Both RAG vectors and MNIST images share a 784-dimensional space.
const int INPUT_DIM = 784;         
const int HIDDEN_DIM = 784;        
const int CLUSTER_DIM = 1000; // Expanded capacity for Multimodal memory      

// --- 2. The LIF Spiking Neuron (v4) ---
struct LIF_Neuron {
    int8_t weight;              
    float membrane_potential; 
    float leak_rate;          
    uint64_t last_spike_time;   
};

LIF_Neuron Cortex[CLUSTER_DIM][HIDDEN_DIM];

// Metadata Storage for RAG
vector<string> article_titles;
vector<string> vocabulary;
vector<int> cluster_contents[CLUSTER_DIM]; 

void load_lines(const string& filepath, vector<string>& storage) {
    ifstream file(filepath);
    string line;
    while (getline(file, line)) if (!line.empty()) storage.push_back(line);
}

uint64_t get_time_ms() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

// --- 3. The Spiking Multimodal Engine (LIF + STDP) ---
void process_spikes(const vector<int8_t>& incoming_spikes, int doc_id = -1) {
    int winning_cluster = -1;
    int first_empty_cluster = -1;
    float highest_voltage = -9999.0f;
    uint64_t current_time = get_time_ms();

    // Phase 1: Integrate and Fire (Voltage Accumulation)
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Cortex[c][0].last_spike_time == 0) {
            if (first_empty_cluster == -1) first_empty_cluster = c;
            continue; 
        }

        float incoming_current = 0.0f;
        int active_synapses = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (incoming_spikes[i] != 0 || Cortex[c][i].weight != 0) {
                // Decay old voltage
                Cortex[c][i].membrane_potential *= (1.0f - Cortex[c][i].leak_rate);

                // Add or subtract current based on synaptic bond
                if (Cortex[c][i].weight == incoming_spikes[i]) incoming_current += 1.0f;
                else incoming_current -= 1.0f; // Orthogonal Mismatch Penalty
                
                active_synapses++;
            }
        }

        if (active_synapses > 0) {
            float normalized_voltage = incoming_current / active_synapses;
            Cortex[c][0].membrane_potential += normalized_voltage;

            if (Cortex[c][0].membrane_potential > highest_voltage) {
                highest_voltage = Cortex[c][0].membrane_potential;
                winning_cluster = c;
            }
        }
    }

    // Phase 2: Neurogenesis (Protecting old memories from Domain Shifts)
    float VIGILANCE_THRESHOLD = 0.05f; 
    if (highest_voltage < VIGILANCE_THRESHOLD && first_empty_cluster != -1) {
        winning_cluster = first_empty_cluster;
        Cortex[winning_cluster][0].membrane_potential = 1.0f; // Force Spike
    }

    // Phase 3: STDP Hebbian Update
    if (winning_cluster != -1) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (incoming_spikes[i] != 0) {
                Cortex[winning_cluster][i].weight = incoming_spikes[i];
                Cortex[winning_cluster][i].leak_rate = 0.1f;
            }
        }
        Cortex[winning_cluster][0].last_spike_time = current_time;
        Cortex[winning_cluster][0].membrane_potential = 0.0f; // Refractory reset
        
        if (doc_id != -1) cluster_contents[winning_cluster].push_back(doc_id);
    }
}

// --- 4. The Query Vectorizer ---
vector<int8_t> vectorize_query(const string& query) {
    vector<int8_t> query_vector(INPUT_DIM, 0);
    string word = "";
    for (char c : query) {
        if (isalpha(c)) word += tolower(c); 
        else if (!word.empty()) {
            for(int i = 0; i < vocabulary.size(); i++) 
                if(vocabulary[i] == word) { query_vector[i] = 1; break; }
            word = "";
        }
    }
    if (!word.empty()) {
        for(int i = 0; i < vocabulary.size(); i++) 
            if(vocabulary[i] == word) { query_vector[i] = 1; break; }
    }
    return query_vector;
}

// --- 5. Phase 3: Semantic Retrieval ---
void perform_search(const string& query) {
    vector<int8_t> query_vector = vectorize_query(query);
    int best_cluster_id = -1;
    float highest_voltage = -9999.0f;

    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Cortex[c][0].last_spike_time == 0) continue; 
        float incoming_current = 0.0f;
        int active_synapses = 0;
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (query_vector[i] != 0) { 
                if (Cortex[c][i].weight == query_vector[i]) incoming_current += 1.0f;
                else incoming_current -= 1.0f;
                active_synapses++;
            }
        }
        if (active_synapses > 0) {
            float normalized_voltage = incoming_current / active_synapses;
            if (normalized_voltage > highest_voltage) {
                highest_voltage = normalized_voltage;
                best_cluster_id = c;
            }
        }
    }

    cout << "\n==========================================" << endl;
    if (best_cluster_id != -1 && highest_voltage > -1.0f && !cluster_contents[best_cluster_id].empty()) {
        cout << "🧠 RAG RETRIEVAL SUCCESS | Spiked Cluster: " << best_cluster_id << endl;
        for (int doc_id : cluster_contents[best_cluster_id]) cout << "  -> " << article_titles[doc_id] << endl;
    } else {
        cout << "❌ MEMORY CORRUPTED OR NOT FOUND." << endl;
    }
    cout << "==========================================\n" << endl;
}

// --- System Boot & Multimodal Benchmark ---
int main() {
    cout << "=========================================================" << endl;
    cout << "  FP-SAN v4 | Asynchronous Spiking Neural OS (Apex) " << endl;
    cout << "=========================================================\n" << endl;
    
    // PHASE 1: SEMANTIC MEMORY (Text)
    cout << "[Phase 1] Booting Semantic Lexicon..." << endl;
    load_lines("rag_metadata.txt", article_titles);
    load_lines("rag_vocab.txt", vocabulary);
    
    ifstream rag_file("rag_vectors.csv");
    string line;
    int doc_id = 0;
    cout << "[Phase 1] Crystallizing Wikipedia Knowledge Base..." << endl;
    while (getline(rag_file, line) && doc_id < article_titles.size()) {
        stringstream ss(line);
        string value;
        vector<int8_t> article_vector(INPUT_DIM, 0);
        int i = 0;
        while (getline(ss, value, ',') && i < INPUT_DIM) article_vector[i++] = (stof(value) > 0.0f) ? 1 : 0;
        process_spikes(article_vector, doc_id);
        doc_id++;
    }

    // PHASE 2: CATASTROPHIC DOMAIN SHIFT (Visual Pixels)
    cout << "\n[CRITICAL] Domain Shift Detected. Injecting 10,000 Visual MNIST Frames..." << endl;
    ifstream mnist_file("mnist_stream.csv");
    if (mnist_file.is_open()) {
        int digit_count = 0;
        while (getline(mnist_file, line)) {
            stringstream ss(line);
            string value;
            vector<int8_t> visual_spikes(INPUT_DIM, 0);
            int i = 0;
            while (getline(ss, value, ',') && i < INPUT_DIM) visual_spikes[i++] = (stof(value) > 0.5f) ? 1 : 0;
            process_spikes(visual_spikes); // Note: No doc_id attached to pixels
            digit_count++;
        }
        cout << "[Phase 2] Visual Cortex fully mapped. Processed " << digit_count << " sensory frames." << endl;
    } else {
        cout << "[Error] mnist_stream.csv missing. Skipping Phase 2." << endl;
    }

    // PHASE 3: THE FORGETTING BENCHMARK
    cout << "\n[Phase 3] Initiating Catastrophic Forgetting Test." << endl;
    cout << "The AI has processed 10,000 pixel images. Did it overwrite its text memory?" << endl;
    
    string user_query;
    while (true) {
        cout << "Enter RAG Search Query (or type 'exit'): ";
        getline(cin, user_query);
        if (user_query == "exit" || user_query == "quit") break;
        if (user_query.empty()) continue;
        perform_search(user_query);
    }

    return 0;
}