#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <stack>
#include <algorithm>
#include <fstream>

using namespace std;

#define PRODUCT_X_ID "A8_11"
#define PRODUCT_Y_ID "A8_50"

struct Node
{
    string id;
    string name;
    map<int, int> demand; // <depth, count>

    Node(const string &id, const string &name) : id(id), name(name), demand({{0, 0}}) {}

    bool operator<(const Node &other) const
    {
        return name < other.name;
    }
};

std::ostream &operator<<(std::ostream &os, const Node &node)
{
    os << "Node ID: " << node.id << ", Name: " << node.name << endl;
    return os;
}

struct ExcelRow
{
    // Node *node;
    string position;
    string position_name;
    int depth;
    int period;
    int pb;
    int magazine_before;
    int pn;
    int magazine_after;
    set<Node *> whereItCameFrom;

    int pieces;

    ExcelRow() {}
    ExcelRow(const string &pos, const string &pos_name, int dp, int pr, int pb_val, int mag_before, int pn_val, int mag_after, set<Node *> &source_nodes)
        : position(pos), position_name(pos_name), depth(dp), period(pr), pb(pb_val), magazine_before(mag_before), pn(pn_val), magazine_after(mag_after), whereItCameFrom(source_nodes)
    {
    }

    bool operator<(const ExcelRow &other) const
    {
        if (position != other.position)
            return position < other.position;

        return pb < other.pb;
    }
};

std::ostream &operator<<(std::ostream &os, const ExcelRow &row)
{
    os << row.position;
    os << "," << row.depth;
    os << "," << row.period;
    os << "," << row.pb;
    os << "," << row.magazine_before;
    os << "," << 0; // you have to modify it directly in the excel (In real excel not in code)
    os << "," << row.pn;
    os << "," << row.magazine_after << ",";
    for (auto it : row.whereItCameFrom)
    {
        os << it->id << " ";
    }

    return os;
}

map<Node *, vector<pair<Node *, int>>> graph;
map<Node *, set<Node *>> graph_of_parents;
map<int, vector<ExcelRow>> excel_Z1;
map<int, vector<ExcelRow>> excel_Z2;
map<int, vector<ExcelRow>> excel_Z3;
map<int, vector<ExcelRow>> result;

// map<int, vector<ExcelRow>> &excel,
void create_excel(map<int, vector<ExcelRow>> &excel, int start_period, int quantity)
{
    int depth = 0;
    int count = 0;
    string id;
    string name;

    for (auto it = graph.begin(); it != graph.end(); ++it)
    {
        Node *node = it->first;
        id = it->first->id;
        name = it->first->name;

        for (const auto &tmp : it->first->demand)
        {
            depth = tmp.first;
            count = tmp.second;

            if (depth != 0 && count != 0)
            {

                ExcelRow row(id, name, depth, start_period - depth, count * quantity, 0, count * quantity, 0, graph_of_parents[node]);
                row.pieces = count;
                excel[depth].push_back(row);

                // cout << "   depth: " << tmp.first << " count: " << tmp.second << endl;
            }
        }
    }
}

// it find excel row with given product id and on given depth
ExcelRow *find_row(map<int, vector<ExcelRow>> &excel, string id, int depth)
{

    if (PRODUCT_X_ID == id || PRODUCT_Y_ID == id)
        return NULL;

    for (ExcelRow &row : excel[depth])
    {
        if (row.position == id)
        {
            return &row;
        }
    }
    return NULL;
}

// clears whereItCameFrom vector from products that are not in previous period

void remove_item_from_wifc(ExcelRow *row, Node *to_remove)
{

    if (PRODUCT_X_ID == to_remove->id || PRODUCT_Y_ID == to_remove->id)
        return;

    auto it_parent = row->whereItCameFrom.begin();
    for (auto &node : row->whereItCameFrom)
    {
        if (node->id == to_remove->id)
        {
            row->whereItCameFrom.erase(it_parent);
            return;
        }
        it_parent++;
    }
}

// remove unecessary id from variable "where it came from"
// clears whereItCameFrom vector from products that are not in previous period
void clear_wicf(map<int, vector<ExcelRow>> &excel)
{
    for (auto &it : excel)
    {
        for (ExcelRow &row : it.second)
        {
            for (auto &parent : row.whereItCameFrom)
            {

                ExcelRow *parent_row = find_row(excel, parent->id, (it.first - 1));

                if (parent_row == NULL)
                    remove_item_from_wifc(&row, parent);
            }
        }
    }
}

// calculate demand for excel that has some product in magazine
void calculate_excel_demand(map<int, vector<ExcelRow>> &excel)
{
    for (auto &it : excel)
    {
        for (ExcelRow &row : it.second)
        {

            int counter = 0;
            int real_demand = 0;
            for (auto &wicf : row.whereItCameFrom)
            {
                ExcelRow *parent_row = find_row(excel, wicf->id, it.first - 1);
                if (parent_row != NULL)
                {
                    counter++;
                    real_demand += parent_row->pn;
                }
            }

            // here is calculating real demand
            // counter > 0 mean that demand is appriopriate to assign
            if (counter > 0)
            {
                // cout << row << " depth: " << it.first << endl;
                row.pn = real_demand;
                // cout << row.pn << endl;
            }
        }
    }
}

void print_excel(map<int, vector<ExcelRow>> &excel)
{
    for (auto it = excel.begin(); it != excel.end(); ++it)
    {
        for (auto row : it->second)
        {
            std::cout << row << endl;
        }
    }
}

void dfs(Node *node, int depth)
{
    node->demand[depth]++; // count pb

    for (pair<Node *, int> &neighbor : graph[node])
    {
        graph_of_parents[neighbor.first].insert(node); // for wicf
        dfs(neighbor.first, depth + 1);
    }
}

// reset demand for each node in graph
void clear_dfs(Node *node)
{

    for (auto &it : node->demand)
    {
        it.second = 0;
    }

    for (pair<Node *, int> &neighbor : graph[node])
    {
        clear_dfs(neighbor.first);
    }
}

void set_stock_on_level_1(map<int, vector<ExcelRow>> &excel, string id, int stock)
{
    ExcelRow *row = find_row(excel, id, 1);
    row->magazine_before = stock;
    row->pn = row->pb - row->magazine_before;
}

void save_to_txt(map<int, vector<ExcelRow>> &excel, const string &filename)
{
    std::ofstream outputFile(filename); // Open file for writing

    if (outputFile.is_open())
    { // Check if file is opened successfully
        for (auto it = excel.begin(); it != excel.end(); ++it)
        {
            for (const auto &row : it->second)
            {
                outputFile << row << std::endl; // Write row to file
            }
        }
        outputFile.close(); // Close file
        std::cout << "Excel data has been saved to " << filename << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

ExcelRow merge_rows(ExcelRow &a, ExcelRow &b)
{

    ExcelRow result;

    result.position = a.position;
    result.position_name = a.position_name;
    result.depth = a.depth;
    result.period = a.period;
    result.pb = a.pb + b.pb;
    result.magazine_before = a.magazine_before + b.magazine_before;
    result.pn = a.pn + b.pn;
    result.magazine_after = a.magazine_after + a.magazine_after;
    result.whereItCameFrom.insert(a.whereItCameFrom.begin(), a.whereItCameFrom.end());
    result.whereItCameFrom.insert(b.whereItCameFrom.begin(), b.whereItCameFrom.end());

    return result;
}

map<int, vector<ExcelRow>> merge_excels_implementation(map<int, vector<ExcelRow>> &a, map<int, vector<ExcelRow>> &b)
{
    map<int, vector<ExcelRow>> result;

    int depth = 0;
    for (auto &level : a)
    {
        depth = level.first;

        // operation on sets B \ A. It means get all elements from A and common from B
        for (auto &row : a[depth])
        {
            ExcelRow *row_b = find_row(b, row.position, depth);
            if (row_b != NULL)
            {
                ExcelRow new_row = merge_rows(row, *row_b);
                result[depth].push_back(new_row);
            }
            else
            {
                result[depth].push_back(row);
            }
        }

        // similar us above, but get only unique from B. B\A 
        for(auto &row: b[depth]){
            ExcelRow * exist = find_row(result, row.position, depth);
            if(exist == NULL)
                result[depth].push_back(row); 
        }
    }

    return result;
}

map<int, vector<ExcelRow>> merge_excels(map<int, vector<ExcelRow>> &a, map<int, vector<ExcelRow>> &b)
{

    map<int, vector<ExcelRow>> result;
    if (a.size() > b.size())
    {
        result = merge_excels_implementation(b, a);
    }
    result = merge_excels_implementation(a, b);

    return result;
}

int main()
{
    Node *guma = new Node("A8_1", "guma");
    Node *aluminium = new Node("A8_2", "aluminium");
    Node *zelazo = new Node("A8_3", "zelazo");
    Node *wkret_dociskowy = new Node("A8_4", "wkret_dociskowy");
    Node *sprezyna = new Node("A8_5", "sprezyna");
    Node *zarowka = new Node("A8_6", "zarowka");
    Node *kabel_z_przelacznikiem = new Node("A8_7", "kabel_z_przelacznikiem");
    Node *tkanina_lnu = new Node("A8_8", "tkanina lnu");
    Node *drewno = new Node("A8_9", "drewno");
    Node *klej = new Node("A8_10", "klej");
    Node *lampa_A = new Node("A8_11", "lampa A");
    Node *czapka = new Node("A8_12", "czapka");
    Node *ramie = new Node("A8_13", "ramie");
    Node *podstawa_typu_A = new Node("A8_14", "podstawa typu A");
    Node *abazur = new Node("A8_15", "abazur");
    Node *element_dekoracyjny = new Node("A8_16", "element dekoracyjny");
    Node *gniazdo_na_zarowke = new Node("A8_17", "gniazdo na zarowke");
    Node *zaslepka = new Node("A8_18", "zaslepka");
    Node *oslona = new Node("A8_19", "oslona");
    Node *gorne_ramie = new Node("A8_20", "gorne ramie");
    Node *dolne_ramie = new Node("A8_21", "dolne ramie");
    Node *blok_widel = new Node("A8_22", "blok widel");
    Node *arkusz_lnu = new Node("A8_23", "arkusz lnu");
    Node *element_drewniany = new Node("A8_24", "element drewniany");
    Node *zaslepka_typu_RING = new Node("A8_25", "zaslepka typu RING");
    Node *forma_gniazda_na_zarowke = new Node("A8_26", "forma gniazda na zarowke");
    Node *pasek_gumy = new Node("A8_27", "pasek gumy");
    Node *rulon_aluminium = new Node("A8_28", "rulon aluminium");
    Node *tylne_ramie = new Node("A8_29", "tylne ramie");
    Node *pasek_laczacy = new Node("A8_30", "pasek laczacy");
    Node *mostek = new Node("A8_31", "mostek");
    Node *duzy_przegub_kulowy = new Node("A8_32", "duzy przegub kulowy");
    Node *maly_przegub_kulowy = new Node("A8_33", "maly przegub kulowy");
    Node *ramie_boczne = new Node("A8_34", "ramie boczne");
    Node *pokretlo_regulacyjne = new Node("A8_35", "pokretlo regulacyjne");
    Node *oczyszczona_guma = new Node("A8_36", "oczyszczona guma");
    Node *masa_aluminiowa = new Node("A8_37", "masa aluminiowa");
    Node *pasek_aluminium = new Node("A8_38", "pasek aluminium");
    Node *rurka_aluminiowa_typu_A = new Node("A8_39", "rurka aluminiowa typu A");
    Node *rurka_aluminiowa_typu_B = new Node("A8_40", "rurka aluminiowa typu B");
    Node *pret_zelaza = new Node("A8_41", "pret zelaza");
    Node *pret_aluminium = new Node("A8_42", "pret aluminium");
    Node *plyta_aluminiowa = new Node("A8_43", "plyta aluminiowa");
    Node *podstawa_typu_B = new Node("A8_44", "podstawa typu B");
    Node *drewniany_kolek = new Node("A8_45", "drewniany kolek");
    Node *material_antyposlizgowy = new Node("A8_46", "material antyposlizgowy");
    Node *drewniany_kloc = new Node("A8_47", "drewniany kloc");
    Node *drewniana_podstawka = new Node("A8_48", "drewniana podstawka");
    Node *gwintowana_rurka = new Node("A8_49", "gwintowana rurka");
    Node *lampa_B = new Node("A8_50", "Lampa B");

    graph[lampa_A].push_back({czapka, 1});
    graph[lampa_A].push_back({ramie, 1});
    graph[lampa_A].push_back({podstawa_typu_A, 1});
    graph[lampa_A].push_back({kabel_z_przelacznikiem, 1});
    // ok
    graph[czapka].push_back({abazur, 1});
    graph[czapka].push_back({element_dekoracyjny, 1});
    graph[czapka].push_back({gniazdo_na_zarowke, 1});
    graph[czapka].push_back({zaslepka, 1});
    graph[czapka].push_back({oslona, 1});
    graph[czapka].push_back({zarowka, 1});
    // ok
    graph[ramie].push_back({gorne_ramie, 1});
    graph[ramie].push_back({dolne_ramie, 1});
    // ok
    graph[podstawa_typu_A].push_back({sprezyna, 2});
    graph[podstawa_typu_A].push_back({wkret_dociskowy, 2});
    graph[podstawa_typu_A].push_back({blok_widel, 1});
    // ok
    graph[abazur].push_back({arkusz_lnu, 1});
    graph[abazur].push_back({klej, 1});
    // ok
    graph[element_dekoracyjny].push_back({element_drewniany, 1});
    graph[element_dekoracyjny].push_back({klej, 1});
    // ok
    graph[gniazdo_na_zarowke].push_back({zaslepka_typu_RING, 1});
    graph[gniazdo_na_zarowke].push_back({forma_gniazda_na_zarowke, 1});
    // ok
    graph[zaslepka].push_back({pasek_gumy, 1});
    // ok
    graph[oslona].push_back({rulon_aluminium, 1});
    // ok
    graph[gorne_ramie].push_back({tylne_ramie, 1});
    graph[gorne_ramie].push_back({pasek_laczacy, 1});
    graph[gorne_ramie].push_back({mostek, 1});
    graph[gorne_ramie].push_back({wkret_dociskowy, 1});
    graph[gorne_ramie].push_back({duzy_przegub_kulowy, 1});
    // ok

    graph[dolne_ramie].push_back({ramie_boczne, 1});
    graph[dolne_ramie].push_back({pokretlo_regulacyjne, 1});
    graph[dolne_ramie].push_back({maly_przegub_kulowy, 1});
    // ok

    graph[arkusz_lnu].push_back({tkanina_lnu, 1});
    // ok
    graph[element_drewniany].push_back({drewno, 1});
    // ok
    graph[zaslepka_typu_RING].push_back({oczyszczona_guma, 1});
    // ok
    graph[forma_gniazda_na_zarowke].push_back({masa_aluminiowa, 1});
    // ok
    graph[pasek_gumy].push_back({oczyszczona_guma, 1});
    // ok
    graph[rulon_aluminium].push_back({pasek_aluminium, 1});
    // ok
    graph[tylne_ramie].push_back({rurka_aluminiowa_typu_A, 1});
    // ok

    graph[pasek_laczacy].push_back({rurka_aluminiowa_typu_B, 1});
    // ok
    graph[mostek].push_back({pret_zelaza, 1});
    // ok
    graph[duzy_przegub_kulowy].push_back({pret_aluminium, 1});
    // ok
    graph[maly_przegub_kulowy].push_back({pret_aluminium, 1});
    // ok
    graph[ramie_boczne].push_back({rurka_aluminiowa_typu_A, 1});
    // ok
    graph[pokretlo_regulacyjne].push_back({pret_aluminium, 1});
    // ok
    graph[pret_aluminium].push_back({aluminium, 1});
    // ok
    graph[oczyszczona_guma].push_back({guma, 1});
    //
    graph[masa_aluminiowa].push_back({aluminium, 1});
    // ok
    graph[pasek_aluminium].push_back({plyta_aluminiowa, 1});
    // ok
    graph[rurka_aluminiowa_typu_A].push_back({pasek_aluminium, 1});
    graph[rurka_aluminiowa_typu_B].push_back({pasek_aluminium, 1});
    // ok
    graph[plyta_aluminiowa].push_back({aluminium, 1});
    // ok
    graph[lampa_B].push_back({czapka, 1});
    graph[lampa_B].push_back({ramie, 1});
    graph[lampa_B].push_back({kabel_z_przelacznikiem, 1});
    graph[lampa_B].push_back({podstawa_typu_B, 1});
    // ok
    graph[podstawa_typu_B].push_back({drewniana_podstawka, 1});
    graph[podstawa_typu_B].push_back({material_antyposlizgowy, 1});
    graph[podstawa_typu_B].push_back({gwintowana_rurka, 1});
    // ok
    graph[drewniana_podstawka].push_back({drewniany_kolek, 1});
    graph[drewniany_kolek].push_back({drewniany_kloc, 1});
    graph[drewniany_kloc].push_back({drewno, 1});
    // ok
    graph[material_antyposlizgowy].push_back({oczyszczona_guma, 1});
    // ok
    graph[gwintowana_rurka].push_back({rurka_aluminiowa_typu_A, 1});

    dfs(lampa_A, 0);

    create_excel(excel_Z1, 20, 2);
    create_excel(excel_Z2, 22, 3);
    clear_wicf(excel_Z2);
    clear_dfs(lampa_A);
    set_stock_on_level_1(excel_Z1, "A8_12", 2);
    calculate_excel_demand(excel_Z1);
    // print_excel(excel_Z1);
    save_to_txt(excel_Z1, "z1.txt");
    save_to_txt(excel_Z2, "z2.txt");

    // create second excel
    graph_of_parents.clear();
    dfs(lampa_B, 0);
    create_excel(excel_Z3, 20, 4);
    clear_wicf(excel_Z3);
    save_to_txt(excel_Z3, "z3.txt");
    // print_excel(excel_Z3);

    // calculate_excel_demand()

    result = merge_excels_implementation(excel_Z1, excel_Z3);
    

    print_excel(result);
    save_to_txt(result, "result.txt");

    cout << "program finished successfully" << endl;
    return 0;
}
