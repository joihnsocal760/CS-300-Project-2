#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>

using namespace std;

class Course {
    string id;
    string title;
    vector<string> prereqs;
    vector<Course> prereqs2;

public:
    Course() = default;
    Course(string i, string t) : id(move(i)), title(move(t)) {}

    string Id() const { return id; }
    string Title() const { return title; }
    const vector<string>& Prereqs() const { return prereqs; }

    void AddPrereq(const string& title) { prereqs.push_back(title); }
    void AddPrereq2(const Course& course) { prereqs2.push_back(course); }

    void PrintCourseOnly() const {
        cout << id << ": " << title << "\n";
    }

    void PrintCourseInfo() const {
        cout << id << ": " << title << "\n";
        if (prereqs.empty()) {
            cout << "\t---> Prereqs: NONE\n";
        }
        else {
            cout << "\t---> Prereqs: ";
            for (size_t i = 0; i < prereqs.size(); ++i) {
                cout << prereqs[i] << (i == prereqs.size() - 1 ? "\n" : ", ");
            }
        }
        cout << "\n";
    }
};

struct Node {
    Course course;
    unique_ptr<Node> left;
    unique_ptr<Node> right;

    Node(Course c) : course(move(c)), left(nullptr), right(nullptr) {}

    void PrintCourseOnly() const { course.PrintCourseOnly(); }
    void PrintCourseInfo() const { course.PrintCourseInfo(); }
};

class BinarySearchTree {
    unique_ptr<Node> root;

    void InOrderRecursive(const Node* node) const {
        if (!node) return;
        InOrderRecursive(node->left.get());
        node->PrintCourseInfo();
        InOrderRecursive(node->right.get());
    }

    Node* GetParentRecursive(Node* subtree, Node* node) const {
        if (!subtree) return nullptr;
        if (subtree->left.get() == node || subtree->right.get() == node) return subtree;
        return node->course.Id() < subtree->course.Id() ? GetParentRecursive(subtree->left.get(), node) : GetParentRecursive(subtree->right.get(), node);
    }

    void InsertRecursive(Node* node, Course course) {
        if (course.Id() < node->course.Id()) {
            if (!node->left) {
                node->left = make_unique<Node>(move(course));
            }
            else {
                InsertRecursive(node->left.get(), move(course));
            }
        }
        else {
            if (!node->right) {
                node->right = make_unique<Node>(move(course));
            }
            else {
                InsertRecursive(node->right.get(), move(course));
            }
        }
    }

    Node* SearchRecursiveHelper(Node* node, const string& courseId) const {
        if (!node) return nullptr;
        if (courseId == node->course.Id()) return node;
        return courseId < node->course.Id() ? SearchRecursiveHelper(node->left.get(), courseId) : SearchRecursiveHelper(node->right.get(), courseId);
    }

    size_t GetSizeHelper(Node* node) const {
        return node ? 1 + GetSizeHelper(node->left.get()) + GetSizeHelper(node->right.get()) : 0;
    }

public:
    BinarySearchTree() = default;

    size_t GetSize() const { return GetSizeHelper(root.get()); }

    void Insert(Course course) {
        if (!root) {
            root = make_unique<Node>(move(course));
        }
        else {
            InsertRecursive(root.get(), move(course));
        }
    }

    Node* GetParent(Node* node) const { return GetParentRecursive(root.get(), node); }
    Node* SearchRecursive(const string& courseId) const { return SearchRecursiveHelper(root.get(), courseId); }

    void InOrder() const {
        if (!root) {
            cout << "\n\t* No courses to display *\n";
            return;
        }
        InOrderRecursive(root.get());
    }
};

void PrintIntroduction() { cout << "Hello, please choose an option\n\n"; }
void PrintMenu() {
    cout << "\n\n\tMain Menu\n";
    cout << "1. Load Courses from a file\n";
    cout << "2. Display all courses\n";
    cout << "3. Display course information\n";
    cout << "9. Exit application\n";
}

int GetChoice() {
    int choice = 0;
    cout << "\nPlease enter your selection: ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

string GetFileName() {
    string filename;
    cout << "\nPlease enter the filename: ";
    getline(cin, filename);
    return filename;
}

string GetInputCourse() {
    string userInput;
    cout << "\nPlease enter the course id: ";
    getline(cin, userInput);
    return userInput;
}

string trimWord(string str) {
    const char* removeChar = " \t\n\r\f\v";
    str.erase(str.find_last_not_of(removeChar) + 1);
    str.erase(0, str.find_first_not_of(removeChar));
    return str;
}

void CreateMasterCourseList(string filepath, vector<string>& courses) {
    ifstream infile(filepath);
    if (!infile) return;
    string line, word;
    while (getline(infile, line)) {
        stringstream ss(line);
        while (getline(ss, word, ',')) {
            courses.push_back(trimWord(word));
        }
    }
}

void LoadCourses(const string& filepath, BinarySearchTree& bst) {
    vector<string> masterCourseList;
    CreateMasterCourseList(filepath, masterCourseList);

    ifstream infile(filepath);
    if (!infile) {
        cerr << filepath << " does not exist\n";
        return;
    }
    cout << "\n\t* Success " << filepath << " *\n\n";

    string line, word;
    while (getline(infile, line)) {
        stringstream ss(line);
        vector<string> courseLine;
        while (getline(ss, word, ',')) {
            courseLine.push_back(trimWord(word));
        }
        if (courseLine.size() < 2) {
            cerr << "Incomplete record in file\n";
            return;
        }
        Course c(courseLine[0], courseLine[1]);
        for (size_t i = 2; i < courseLine.size(); ++i) {
            if (find(masterCourseList.begin(), masterCourseList.end(), courseLine[i]) != masterCourseList.end()) {
                c.AddPrereq(courseLine[i]);
            }
        }
        if (bst.SearchRecursive(c.Id()) == nullptr) {
            bst.Insert(move(c));
            cout << c.Id() << " " << c.Title() << " has been read\n";
        }
        else {
            cout << c.Id() << " already exists\n";
        }
    }
}

int main(int argc, char* argv[]) {
    string filename = (argc == 2) ? argv[1] : "ABCU_test.txt";
    BinarySearchTree bst;
    PrintIntroduction();

    int choice = 0;
    while (choice != 9) {
        PrintMenu();
        choice = GetChoice();
        switch (choice) {
        case 1:
            filename = GetFileName();
            LoadCourses(filename, bst);
            cin.get();
            break;
        case 2:
            cout << "\n\nCourse Listing\n";
            bst.InOrder();
            cout << bst.GetSize() << " courses stored\n";
            cin.get();
            break;
        case 3:
        {
            string inputCourse = GetInputCourse();
            if (bst.SearchRecursive(inputCourse) == nullptr) {
                cout << "\n\n\t* " << inputCourse << " not found *\n";
            }
            else {
                cout << "\n\nCourse found: \n";
                bst.SearchRecursive(inputCourse)->PrintCourseInfo();
            }
        }
        cin.get();
        break;
        case 9:
            break;
        default:
            cerr << "\n\n\t* Error, try again * \n\n";
            break;
        }
    }
    cout << "\n\nThank You, Goodbye\n";
}