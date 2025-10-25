#include <bits/stdc++.h>
using namespace std;
#pragma execution_character_set("utf-8")

// ===================== CẤU TRÚC DỮ LIỆU =====================
struct Item {
    int weight, value;
};
struct Goat {
    vector<double> pos; // vị trí thực
    vector<int> bin;    // biểu diễn nhị phân (0/1)
    double fitness;
};

// ===================== THAM SỐ GOA =====================
int N = 30;           // số dê
int Tmax = 300;       // vòng lặp tối đa
double alpha = 0.05;  // hệ số khám phá
double b_factor = 0.5;// hệ số khai thác
double J = 0.1;       // xác suất nhảy
double weak_ratio = 0.2; // tỉ lệ dê yếu bị thay

// ===================== DỮ LIỆU KNAPSACK =====================
int n_items, W;
vector<Item> items;

// ===================== HÀM FITNESS =====================
double evaluate(const vector<int> &x) {
    int sumW = 0, sumV = 0;
    for (int i = 0; i < n_items; i++) {
        if (x[i]) {
            sumW += items[i].weight;
            sumV += items[i].value;
        }
    }
    if (sumW > W) {
        double penalty = 1000.0;
        return max(0.0, sumV - penalty * (sumW - W));
    }
    return sumV;
}

// ===================== RỜI RẠC HÓA =====================
vector<int> toBinary(const vector<double> &pos) {
    vector<int> b(n_items);
    for (int i = 0; i < n_items; i++) {
        double p = 1.0 / (1.0 + exp(-pos[i]));
        b[i] = (p > 0.5) ? 1 : 0;
    }
    return b;
}

// ===================== KHỞI TẠO QUẦN THỂ =====================
vector<Goat> initPopulation() {
    vector<Goat> g(N);
    for (int i = 0; i < N; i++) {
        g[i].pos.resize(n_items);
        g[i].bin.assign(n_items, 0);

        int totalW = 0;
        // Sinh nghiệm hợp lệ
        for (int j = 0; j < n_items; j++) {
            int w = items[j].weight;
            if (totalW + w <= W && ((rand() % 100) < 10)) { // 10% cơ hội chọn
                g[i].bin[j] = 1;
                totalW += w;
            }
            g[i].pos[j] = (rand() / (double)RAND_MAX) * 4 - 2;
        }

        g[i].fitness = evaluate(g[i].bin);
    }
    return g;
}


// ===================== THUẬT TOÁN GOA =====================
Goat GOA_Knapsack() {
    srand(time(0));
    vector<Goat> goats = initPopulation();

    Goat best = *max_element(goats.begin(), goats.end(),
        [](const Goat &a, const Goat &b){ return a.fitness < b.fitness; });

    for (int t = 0; t < Tmax; t++) {

        for (int i = 0; i < N; i++) {
            // --- Exploration ---
            for (int j = 0; j < n_items; j++) {
                double r = ((rand() / (double)RAND_MAX) - 0.5);
                goats[i].pos[j] += alpha * r;
            }

            // --- Exploitation ---
            for (int j = 0; j < n_items; j++) {
                goats[i].pos[j] += b_factor * (best.pos[j] - goats[i].pos[j]);
            }

            // --- Jump Strategy ---
            if ((rand() / (double)RAND_MAX) < J) {
                int r = rand() % N;
                for (int j = 0; j < n_items; j++)
                    goats[i].pos[j] += J * (goats[r].pos[j] - goats[i].pos[j]);
            }

            // --- Cập nhật binary và fitness ---
            goats[i].bin = toBinary(goats[i].pos);
            goats[i].fitness = evaluate(goats[i].bin);
        }

        // --- Regeneration ---
        sort(goats.begin(), goats.end(),
            [](const Goat &a, const Goat &b){ return a.fitness > b.fitness; });

        int num_weak = max(1, (int)round(N * weak_ratio));
        for (int i = N - num_weak; i < N; i++) {
            for (int j = 0; j < n_items; j++)
                goats[i].pos[j] = (rand() / (double)RAND_MAX) * 4 - 2;
            goats[i].bin = toBinary(goats[i].pos);
            goats[i].fitness = evaluate(goats[i].bin);
        }

        Goat cur_best = *max_element(goats.begin(), goats.end(),
            [](const Goat &a, const Goat &b){ return a.fitness < b.fitness; });
        if (cur_best.fitness > best.fitness)
            best = cur_best;
    }

    return best;
}

// ===================== MAIN =====================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(0));

    // === ĐỌC DỮ LIỆU TỪ FILE ===
    ifstream fin("input_knapsack.txt");
    if (!fin.is_open()) {
        cerr << "Không thể mở file input_knapsack.txt\n";
        return 1;
    }

    fin >> n_items >> W;
    items.resize(n_items);
    for (int i = 0; i < n_items; i++)
        fin >> items[i].weight >> items[i].value;
    fin.close();

    cout << "Đọc dữ liệu thành công! (" << n_items << " vật phẩm, W = " << W << ")\n";

    // === Chạy GOA ===
    Goat result = GOA_Knapsack();

    // === Tính trọng lượng thực tế ===
    int total_weight = 0;
    for (int i = 0; i < n_items; i++)
        if (result.bin[i]) total_weight += items[i].weight;

    // === In kết quả ===
    cout << "Giá trị tối ưu: " << result.fitness << "\n";
    cout << "Tổng trọng lượng: " << total_weight << " / " << W << "\n";
    /*cout << "Vật phẩm được chọn: ";
    for (int i = 0; i < n_items; i++)
        if (result.bin[i]) cout << i + 1 << " ";
    cout << "\n";
    */
    return 0;
}
