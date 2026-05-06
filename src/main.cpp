#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Task {
    int id{};
    string name;
    string course;
    string deadline; // format: YYYY-MM-DD HH:MM
    int priority{};
    string note;
    bool done{false};
};

struct TaskKey {
    string deadline;
    int priority{};
    int id{};

    bool operator<(const TaskKey& other) const {
        if (deadline != other.deadline) return deadline < other.deadline;
        if (priority != other.priority) return priority > other.priority;
        return id < other.id;
    }

    bool operator==(const TaskKey& other) const {
        return deadline == other.deadline &&
               priority == other.priority &&
               id == other.id;
    }
};

enum class Color {
    Red,
    Black
};

struct Node {
    TaskKey key;
    Task task;
    Color color{Color::Red};
    Node* parent{nullptr};
    Node* left{nullptr};
    Node* right{nullptr};

    Node(const TaskKey& k, const Task& t) : key(k), task(t) {}
};

class RedBlackTree {
private:
    Node* root{nullptr};

    static Color colorOf(Node* node) {
        return node == nullptr ? Color::Black : node->color;
    }

    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;

        if (y->left != nullptr) {
            y->left->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;

        if (y->right != nullptr) {
            y->right->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }

        y->right = x;
        x->parent = y;
    }

    void fixInsert(Node* z) {
        while (z->parent != nullptr && z->parent->color == Color::Red) {
            Node* parent = z->parent;
            Node* grandparent = parent->parent;

            if (parent == grandparent->left) {
                Node* uncle = grandparent->right;

                if (colorOf(uncle) == Color::Red) {
                    parent->color = Color::Black;
                    uncle->color = Color::Black;
                    grandparent->color = Color::Red;
                    z = grandparent;
                } else {
                    if (z == parent->right) {
                        z = parent;
                        rotateLeft(z);
                        parent = z->parent;
                        grandparent = parent->parent;
                    }

                    parent->color = Color::Black;
                    grandparent->color = Color::Red;
                    rotateRight(grandparent);
                }
            } else {
                Node* uncle = grandparent->left;

                if (colorOf(uncle) == Color::Red) {
                    parent->color = Color::Black;
                    uncle->color = Color::Black;
                    grandparent->color = Color::Red;
                    z = grandparent;
                } else {
                    if (z == parent->left) {
                        z = parent;
                        rotateRight(z);
                        parent = z->parent;
                        grandparent = parent->parent;
                    }

                    parent->color = Color::Black;
                    grandparent->color = Color::Red;
                    rotateLeft(grandparent);
                }
            }
        }

        root->color = Color::Black;
    }

    void inorder(Node* node, vector<Task>& result) const {
        if (node == nullptr) return;
        inorder(node->left, result);
        result.push_back(node->task);
        inorder(node->right, result);
    }

    Node* minimum(Node* node) const {
        if (node == nullptr) return nullptr;
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    Node* findById(Node* node, int id) const {
        if (node == nullptr) return nullptr;

        if (node->task.id == id) return node;

        Node* leftResult = findById(node->left, id);
        if (leftResult != nullptr) return leftResult;

        return findById(node->right, id);
    }

    void destroy(Node* node) {
        if (node == nullptr) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    RedBlackTree() = default;

    ~RedBlackTree() {
        clear();
    }

    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;

    void clear() {
        destroy(root);
        root = nullptr;
    }

    void insert(const Task& task) {
        TaskKey key{task.deadline, task.priority, task.id};
        Node* z = new Node(key, task);

        Node* y = nullptr;
        Node* x = root;

        while (x != nullptr) {
            y = x;
            if (z->key < x->key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        z->parent = y;

        if (y == nullptr) {
            root = z;
        } else if (z->key < y->key) {
            y->left = z;
        } else {
            y->right = z;
        }

        fixInsert(z);
    }

    vector<Task> getAllTasks() const {
        vector<Task> result;
        inorder(root, result);
        return result;
    }

    bool empty() const {
        return root == nullptr;
    }

    bool getNearest(Task& output) const {
        Node* node = minimum(root);
        if (node == nullptr) return false;
        output = node->task;
        return true;
    }

    bool findTaskById(int id, Task& output) const {
        Node* node = findById(root, id);
        if (node == nullptr) return false;
        output = node->task;
        return true;
    }

    bool removeById(int id) {
        vector<Task> tasks = getAllTasks();

        auto oldSize = tasks.size();

        tasks.erase(
            remove_if(tasks.begin(), tasks.end(),
                      [id](const Task& task) { return task.id == id; }),
            tasks.end()
        );

        if (tasks.size() == oldSize) {
            return false;
        }

        clear();

        for (const auto& task : tasks) {
            insert(task);
        }

        return true;
    }
};

class TaskManager {
private:
    RedBlackTree tree;
    int nextId{1};

    static string readLine(const string& prompt) {
        cout << prompt;
        string input;
        getline(cin, input);
        return input;
    }

    static int readInt(const string& prompt) {
        while (true) {
            cout << prompt;
            string input;
            getline(cin, input);

            try {
                return stoi(input);
            } catch (...) {
                cout << "請輸入整數。\n";
            }
        }
    }

    static void printTask(const Task& task) {
        cout << "ID: " << task.id << '\n';
        cout << "任務名稱: " << task.name << '\n';
        cout << "課程名稱: " << task.course << '\n';
        cout << "Deadline: " << task.deadline << '\n';
        cout << "優先級: " << task.priority << '\n';
        cout << "備註: " << task.note << '\n';
        cout << "狀態: " << (task.done ? "已完成" : "未完成") << '\n';
    }

public:
    void addTask() {
        Task task;
        task.id = nextId++;

        task.name = readLine("任務名稱: ");
        task.course = readLine("課程名稱: ");
        task.deadline = readLine("Deadline (YYYY-MM-DD HH:MM): ");
        task.priority = readInt("優先級，數字越大越優先: ");
        task.note = readLine("備註: ");
        task.done = false;

        tree.insert(task);

        cout << "新增成功。\n";
    }

    void listTasks() const {
        vector<Task> tasks = tree.getAllTasks();

        if (tasks.empty()) {
            cout << "目前沒有任務。\n";
            return;
        }

        cout << "\n=== 所有任務，依 deadline 排序 ===\n";

        for (const auto& task : tasks) {
            cout << "-------------------------\n";
            printTask(task);
        }
    }

    void showNearest() const {
        Task task;

        if (!tree.getNearest(task)) {
            cout << "目前沒有任務。\n";
            return;
        }

        cout << "\n=== 最近到期任務 ===\n";
        printTask(task);
    }

    void removeTask() {
        int id = readInt("請輸入要刪除的任務 ID: ");

        if (tree.removeById(id)) {
            cout << "刪除成功。\n";
        } else {
            cout << "找不到這個 ID。\n";
        }
    }

    void editTask() {
        int id = readInt("請輸入要修改的任務 ID: ");

        Task oldTask;
        if (!tree.findTaskById(id, oldTask)) {
            cout << "找不到這個 ID。\n";
            return;
        }

        tree.removeById(id);

        Task task = oldTask;

        cout << "直接按 Enter 代表保留原本內容。\n";

        string name = readLine("新任務名稱: ");
        if (!name.empty()) task.name = name;

        string course = readLine("新課程名稱: ");
        if (!course.empty()) task.course = course;

        string deadline = readLine("新 Deadline (YYYY-MM-DD HH:MM): ");
        if (!deadline.empty()) task.deadline = deadline;

        string priority = readLine("新優先級: ");
        if (!priority.empty()) task.priority = stoi(priority);

        string note = readLine("新備註: ");
        if (!note.empty()) task.note = note;

        tree.insert(task);

        cout << "修改成功。\n";
    }

    void markDone() {
        int id = readInt("請輸入要標記完成的任務 ID: ");

        Task task;
        if (!tree.findTaskById(id, task)) {
            cout << "找不到這個 ID。\n";
            return;
        }

        tree.removeById(id);
        task.done = true;
        tree.insert(task);

        cout << "已標記完成。\n";
    }

    void benchmark() {
        constexpr int taskCount = 10000;

        vector<Task> tasks;
        tasks.reserve(taskCount);

        mt19937 rng(42);
        uniform_int_distribution<int> dayDist(1, 28);
        uniform_int_distribution<int> hourDist(0, 23);
        uniform_int_distribution<int> minuteDist(0, 59);
        uniform_int_distribution<int> priorityDist(1, 5);

        for (int i = 0; i < taskCount; ++i) {
            ostringstream deadline;
            deadline << "2026-05-"
                     << setw(2) << setfill('0') << dayDist(rng)
                     << " "
                     << setw(2) << setfill('0') << hourDist(rng)
                     << ":"
                     << setw(2) << setfill('0') << minuteDist(rng);

            Task task;
            task.id = i + 1;
            task.name = "Task " + to_string(i + 1);
            task.course = "Course";
            task.deadline = deadline.str();
            task.priority = priorityDist(rng);
            task.note = "benchmark";
            task.done = false;

            tasks.push_back(task);
        }

        auto startTree = chrono::high_resolution_clock::now();

        RedBlackTree benchmarkTree;
        for (const auto& task : tasks) {
            benchmarkTree.insert(task);
        }

        auto endTree = chrono::high_resolution_clock::now();

        auto treeInsertTime =
            chrono::duration_cast<chrono::microseconds>(endTree - startTree).count();

        auto startVector = chrono::high_resolution_clock::now();

        vector<Task> normalVector;
        for (const auto& task : tasks) {
            normalVector.push_back(task);
        }

        auto endVector = chrono::high_resolution_clock::now();

        auto vectorInsertTime =
            chrono::duration_cast<chrono::microseconds>(endVector - startVector).count();

        auto startSortedVector = chrono::high_resolution_clock::now();

        vector<Task> sortedVector;
        for (const auto& task : tasks) {
            auto it = lower_bound(
                sortedVector.begin(),
                sortedVector.end(),
                task,
                [](const Task& a, const Task& b) {
                    if (a.deadline != b.deadline) return a.deadline < b.deadline;
                    if (a.priority != b.priority) return a.priority > b.priority;
                    return a.id < b.id;
                }
            );

            sortedVector.insert(it, task);
        }

        auto endSortedVector = chrono::high_resolution_clock::now();

        auto sortedVectorInsertTime =
            chrono::duration_cast<chrono::microseconds>(
                endSortedVector - startSortedVector
            ).count();

        cout << "\n=== Benchmark Result ===\n";
        cout << "測試資料數量: " << taskCount << '\n';
        cout << "普通陣列新增時間: " << vectorInsertTime << " microseconds\n";
        cout << "排序陣列新增時間: " << sortedVectorInsertTime << " microseconds\n";
        cout << "Red-Black Tree 新增時間: " << treeInsertTime << " microseconds\n";

        Task nearest;
        auto startNearestTree = chrono::high_resolution_clock::now();
        benchmarkTree.getNearest(nearest);
        auto endNearestTree = chrono::high_resolution_clock::now();

        auto treeNearestTime =
            chrono::duration_cast<chrono::nanoseconds>(
                endNearestTree - startNearestTree
            ).count();

        auto startNearestVector = chrono::high_resolution_clock::now();

        auto nearestIt = min_element(
            normalVector.begin(),
            normalVector.end(),
            [](const Task& a, const Task& b) {
                if (a.deadline != b.deadline) return a.deadline < b.deadline;
                if (a.priority != b.priority) return a.priority > b.priority;
                return a.id < b.id;
            }
        );

        auto endNearestVector = chrono::high_resolution_clock::now();

        auto vectorNearestTime =
            chrono::duration_cast<chrono::nanoseconds>(
                endNearestVector - startNearestVector
            ).count();

        auto startNearestSortedVector = chrono::high_resolution_clock::now();
        Task sortedNearest = sortedVector.front();
        auto endNearestSortedVector = chrono::high_resolution_clock::now();

        auto sortedVectorNearestTime =
            chrono::duration_cast<chrono::nanoseconds>(
                endNearestSortedVector - startNearestSortedVector
            ).count();

        cout << "\n=== 查詢最近 Deadline ===\n";
        cout << "普通陣列查詢時間: " << vectorNearestTime << " nanoseconds\n";
        cout << "排序陣列查詢時間: " << sortedVectorNearestTime << " nanoseconds\n";
        cout << "Red-Black Tree 查詢時間: " << treeNearestTime << " nanoseconds\n";

        if (nearestIt != normalVector.end()) {
            cout << "\n普通陣列最近任務: " << nearestIt->name
                 << " / " << nearestIt->deadline << '\n';
        }

        cout << "排序陣列最近任務: " << sortedNearest.name
             << " / " << sortedNearest.deadline << '\n';

        cout << "Red-Black Tree 最近任務: " << nearest.name
             << " / " << nearest.deadline << '\n';
    }

    void run() {
        while (true) {
            cout << "\n====== 作業忘記寫了 ======\n";
            cout << "1. 新增任務\n";
            cout << "2. 刪除任務\n";
            cout << "3. 修改任務\n";
            cout << "4. 查詢最近到期任務\n";
            cout << "5. 列出所有任務\n";
            cout << "6. 標記任務完成\n";
            cout << "7. 執行效能測試\n";
            cout << "0. 離開\n";

            int choice = readInt("請選擇功能: ");

            switch (choice) {
                case 1:
                    addTask();
                    break;
                case 2:
                    removeTask();
                    break;
                case 3:
                    editTask();
                    break;
                case 4:
                    showNearest();
                    break;
                case 5:
                    listTasks();
                    break;
                case 6:
                    markDone();
                    break;
                case 7:
                    benchmark();
                    break;
                case 0:
                    cout << "掰掰，記得寫作業。\n";
                    return;
                default:
                    cout << "沒有這個選項。\n";
                    break;
            }
        }
    }
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    TaskManager manager;
    manager.run();
    return 0;
}