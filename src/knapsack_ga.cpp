#include <bits/stdc++.h>
using namespace std;

struct Item {
    size_t index;
    long long weight, value;
    double ratio() const {
        return (double)value / weight;
    }
};

using Individual = vector<bool>;
Individual best_ind;
long long best_val = 0;

const size_t POP_SIZE = 25;
const size_t GENERATIONS = 1000;
const size_t MAX_STAGNATION = 200;
const double MUTATION_RATE = 0.15;

size_t n;
long long W;
vector<Item> items;
mt19937 rng(random_device{}());

// ===================== HÀM PHỤ TRỢ =====================
pair<long long, long long> compute_value_weight(const Individual &ind) {
    long long value = 0, weight = 0;
    for (size_t i = 0; i < ind.size(); ++i) {
        if (ind[i]) {
            weight += items[i].weight;
            value += items[i].value;
        }
    }
    return {value, weight};
}

long long evaluate(const Individual &ind, long long &weight_out) {
    auto [value, weight] = compute_value_weight(ind);
    weight_out = weight;
    return (weight <= W) ? value : 0;
}

Individual greedy_heuristic() {
    vector<Item> sorted = items;
    sort(sorted.begin(), sorted.end(), [](const Item &a, const Item &b) {
        return a.ratio() > b.ratio();
    });

    Individual ind(n, false);
    long long total_weight = 0;
    for (const auto &it : sorted) {
        if (total_weight + it.weight <= W) {
            ind[it.index] = true;
            total_weight += it.weight;
        }
    }
    return ind;
}

void update_best_solution(const Individual &ind) {
    long long w;
    long long val = evaluate(ind, w);
    if (val > best_val) {
        best_val = val;
        best_ind = ind;
    }
}

Individual crossover(const Individual &a, const Individual &b) {
    uniform_int_distribution<size_t> dist(0, n - 1);
    size_t point = dist(rng);
    Individual child(n);
    for (size_t i = 0; i < n; ++i)
        child[i] = (i < point) ? a[i] : b[i];
    return child;
}

// ===================== LOCAL SEARCH =====================
void local_search_1(Individual &ind) {
    auto [current_value, current_weight] = compute_value_weight(ind);

    if (current_weight > W) {
        vector<Item> selected;
        for (size_t i = 0; i < n; ++i)
            if (ind[i]) selected.push_back(items[i]);

        sort(selected.begin(), selected.end(), [](auto &a, auto &b) { return a.ratio() < b.ratio(); });
        for (auto &it : selected) {
            if (current_weight <= W) break;
            ind[it.index] = false;
            current_weight -= it.weight;
            current_value -= it.value;
        }
    }

    vector<Item> not_selected;
    for (size_t i = 0; i < n; ++i)
        if (!ind[i]) not_selected.push_back(items[i]);

    sort(not_selected.begin(), not_selected.end(), [](auto &a, auto &b) { return a.ratio() > b.ratio(); });
    for (auto &it : not_selected) {
        if (current_weight + it.weight <= W) {
            ind[it.index] = true;
            current_weight += it.weight;
            current_value += it.value;
        }
    }
    update_best_solution(ind);
}

// ===================== KHỞI TẠO POPULATION =====================
vector<Individual> initialize_population() {
    vector<Individual> population;
    //population.push_back(greedy_heuristic());

    uniform_int_distribution<int> coin(0, 1);
    while (population.size() < POP_SIZE) {
        Individual ind(n, false);
        vector<size_t> indices(n);
        iota(indices.begin(), indices.end(), 0);
        shuffle(indices.begin(), indices.end(), rng);

        long long total_weight = 0;
        for (size_t i : indices) {
            if (coin(rng) && total_weight + items[i].weight <= W) {
                ind[i] = true;
                total_weight += items[i].weight;
            }
        }
        population.push_back(ind);
    }

    for (auto const &ind : population)
        update_best_solution(ind);
    return population;
}

// ===================== HÀM MAIN =====================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ifstream fin("../data/input_knapsack.txt");
    if (!fin.is_open()) {
        cerr << " Khong the mo file input_knapsack.txt!\n";
        return 1;
    }

    fin >> n >> W;
    if (!fin || n <= 0 || W <= 0) {
        cerr << " Du lieu dau vao khong hop le!\n";
        return 1;
    }

    ofstream fout("../data/output_result_ga.txt");
    if (!fout.is_open()) {
        cerr << " Khong the tao file output_result_ga.txt!\n";
        return 1;
    }

    items.resize(n);
    for (size_t i = 0; i < n; ++i) {
        fin >> items[i].weight >> items[i].value;
        items[i].index = i;
    }
    fin.close();

    cout << "Doc du lieu thanh cong! (" << n << " vat pham, W = " << W << ")\n";

    vector<Individual> population = initialize_population();
    auto start = chrono::steady_clock::now();
    size_t actual_gen = 0;
    int stagnation_count = 0; 
    long long last_best_val = best_val;

    for (size_t gen = 0; gen < GENERATIONS; ++gen) {
        actual_gen = gen + 1;

        vector<long long> fitness;
        for (const auto &ind : population) {
            long long w;
            fitness.push_back(evaluate(ind, w));
        }

        vector<Individual> new_pop;
        while (new_pop.size() < POP_SIZE) {
            uniform_int_distribution<size_t> dist(0, POP_SIZE - 1);
            const auto &p1 = population[dist(rng)];
            const auto &p2 = population[dist(rng)];
            auto child = crossover(p1, p2);
            //local_search_1(child);
            new_pop.push_back(child);
        }
        population = move(new_pop);

        if (best_val > last_best_val) {
            // Có cải thiện -> Reset bộ đếm
            last_best_val = best_val;
            stagnation_count = 0;
        } else {
            // Không cải thiện -> Tăng bộ đếm
            stagnation_count++;
        }

        fout << "Generation " << actual_gen << ": Best Value = " << best_val << endl;

        // Kiểm tra điều kiện dừng
        if (stagnation_count >= 200) { // Dừng nếu 200 thế hệ không đổi
            break;
        }
    }

    // ===================== KẾT QUẢ TỐI ƯU =====================
long long total_weight = 0;
for (size_t i = 0; i < best_ind.size(); ++i)
    if (best_ind[i]) total_weight += items[i].weight;

fout << "----------------------------------------" << endl;
    fout << "TONG KET:" << endl;
    fout << "Tong so the he (Generations): " << actual_gen << endl;
    fout << "Gia tri tot nhat (Best Value): " << best_val << endl;
    fout << "Tong trong luong (Total Weight): " << total_weight << " / " << W << endl;
    
    fout << "Danh sach vat pham duoc chon (Index): ";
    for (size_t i = 0; i < best_ind.size(); ++i) {
        if (best_ind[i]) {
            fout << (i + 1) << " "; // In chỉ số bắt đầu từ 1
        }
    }
    fout << endl;
    
    // Đóng file output
    fout.close();

    // In thông báo ra màn hình console để biết đã xong
    cout << "----------------------------------------\n";
    cout << "Da chay xong! Kiem tra file 'output_result_ga.txt' de xem ket qua.\n";
    cout << "Ket qua toi uu: " << best_val << "\n";
}
