#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdint>

using namespace std;

const int INPUT_DIM = 784;         
const int HIDDEN_DIM = 784;        
const int CLUSTER_DIM = 100;       

struct Neuron {
    int8_t weight;              
    float stiffness;            
    uint64_t last_fired_time;   
};

Neuron Brain[CLUSTER_DIM][HIDDEN_DIM]; 

// --- RAG Metadata Storage ---
vector<string> article_titles;
vector<string> vocabulary;
vector<int> cluster_contents[CLUSTER_DIM]; // Tracks which article went to which cluster

// Helper to load text files
void load_lines_from_file(const string& filepath, vector<string>& storage) {
    ifstream file(filepath);
    string line;
    while (getline(file, line)) {
        if (!line.empty()) storage.push_back(line);
    }
}

// --- The Unsupervised Clustering Engine ---
void process_document(const vector<int8_t>& input_data, int doc_id) {
    int best_cluster_id = -1;
    int first_empty_cluster_id = -1;
    float highest_similarity = -9999.0f;

    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Brain[c][0].last_fired_time == 0) {
            if (first_empty_cluster_id == -1) first_empty_cluster_id = c;
            continue; 
        }

        int similarity_score = 0;
        int active_sensors = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0 || Brain[c][i].weight != 0) { 
                if (Brain[c][i].weight == input_data[i]) similarity_score += 1;
                else similarity_score -= 1; // Mismatch Penalty
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

    // Vigilance Threshold (Neurogenesis)
    if (highest_similarity < 0.05f && first_empty_cluster_id != -1) {
        best_cluster_id = first_empty_cluster_id;
    }

    // Hebbian Update & Document Storage
    if (best_cluster_id != -1) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0) {
                Brain[best_cluster_id][i].weight = input_data[i];
                Brain[best_cluster_id][i].stiffness = 1.0f; // Hardcode memory for search
            }
        }
        Brain[best_cluster_id][0].last_fired_time = 1; // Mark as active
        cluster_contents[best_cluster_id].push_back(doc_id); // Save document to cluster
    }
}

// --- The C++ Text Tokenizer (Query Vectorization) ---
vector<int8_t> vectorize_query(const string& query) {
    vector<int8_t> query_vector(INPUT_DIM, 0);
    string word = "";
    
    for (char c : query) {
        if (isalpha(c)) { 
            word += tolower(c); 
        } else if (!word.empty()) {
            // Find word in our Wikipedia vocabulary
            for(int i = 0; i < vocabulary.size(); i++) {
                if(vocabulary[i] == word) { query_vector[i] = 1; break; }
            }
            word = "";
        }
    }
    // Check last word
    if (!word.empty()) {
        for(int i = 0; i < vocabulary.size(); i++) {
            if(vocabulary[i] == word) { query_vector[i] = 1; break; }
        }
    }
    return query_vector;
}

// --- The Retrieval Engine (RAG Search) ---
void perform_search(const string& query) {
    vector<int8_t> query_vector = vectorize_query(query);
    
    int best_cluster_id = -1;
    float highest_similarity = -9999.0f;

    // Scan the brain for the strongest Atomic Bond to the user's query
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Brain[c][0].last_fired_time == 0) continue; // Skip empty clusters

        int similarity_score = 0;
        int active_sensors = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            // Only score based on words the user actually typed
            if (query_vector[i] != 0) { 
                if (Brain[c][i].weight == query_vector[i]) similarity_score += 1;
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

    cout << "\n==========================================" << endl;
    cout << "🔎 SEARCH RESULTS" << endl;
    cout << "==========================================" << endl;
    
    if (best_cluster_id != -1 && highest_similarity > -1.0f) {
        cout << "[System] Query resonated with Brain Cluster " << best_cluster_id << endl;
        cout << "[System] Retrieving conceptually linked documents:\n" << endl;
        
        for (int doc_id : cluster_contents[best_cluster_id]) {
            cout << "  -> " << article_titles[doc_id] << endl;
        }
    } else {
        cout << "[System] No relevant concepts found in memory. I don't know this yet." << endl;
    }
    cout << "==========================================\n" << endl;
}

// --- System Boot ---
int main() {
    cout << "[System] FP-SAN v2.0 RAG Engine Booting..." << endl;
    
    // 1. Load the biological structure
    load_lines_from_file("rag_metadata.txt", article_titles);
    load_lines_from_file("rag_vocab.txt", vocabulary);
    
    cout << "[System] Loaded " << article_titles.size() << " documents and " 
         << vocabulary.size() << " word vocabulary." << endl;

    // 2. Read Wikipedia and Crystallize the Brain
    ifstream file("rag_vectors.csv");
    string line;
    int doc_id = 0;
    
    cout << "[Cortex] Ingesting Wikipedia and forming Neural Clusters..." << endl;
    while (getline(file, line) && doc_id < article_titles.size()) {
        stringstream ss(line);
        string value;
        vector<int8_t> article_vector(INPUT_DIM, 0);
        int i = 0;
        while (getline(ss, value, ',') && i < INPUT_DIM) {
            article_vector[i] = (stof(value) > 0.0f) ? 1 : 0;
            i++;
        }
        process_document(article_vector, doc_id);
        doc_id++;
    }
    
    cout << "[System] Brain Crystallization Complete. Ready for queries.\n" << endl;

    // 3. The Interactive Terminal
    string user_query;
    while (true) {
        cout << "Enter Search Query (or type 'exit'): ";
        getline(cin, user_query);
        
        if (user_query == "exit" || user_query == "quit") break;
        if (user_query.empty()) continue;
        
        perform_search(user_query);
    }

    return 0;
}