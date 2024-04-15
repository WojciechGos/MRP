// EDYTOWAC LINIE 987


#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <stack>
#include <algorithm>
#include <fstream>

using namespace std;

#define PRODUCT_X_ID "A5_51"
#define PRODUCT_Y_ID "A5_52"

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
        // if (position != other.position)
        //     return position < other.position;

        return period < other.period;
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
map<int, vector<ExcelRow>> tmp;
map<int, vector<ExcelRow>> result_b;
map<int, vector<ExcelRow>> result_final;
map<int, vector<ExcelRow>> result_sorted;

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

map<int, vector<ExcelRow>> merge_excels_implementation_v2(map<int, vector<ExcelRow>> &a, map<int, vector<ExcelRow>> &b)
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
                cout << "comparing: " << *row_b << " || " << row << endl;
                if(row_b->period == row.period){
                    cout << "merging" << endl;
                    ExcelRow new_row = merge_rows(row, *row_b);
                    result[depth].push_back(new_row);
                }else{
                    cout << "pushiing" << endl;
                    result[depth].push_back(row);
                    result[depth].push_back(*row_b);
                }
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

map<int, vector<ExcelRow>> merge_for_solution(map<int, vector<ExcelRow>> a)
{
    map<int, vector<ExcelRow>> result;
   map<string, int> unique_ids;

    int depth = 0;
    for (auto it = a.rbegin(); it != a.rend(); ++it)
    {
        depth = it->first;

        // operation on sets B \ A. It means get all elements from A and common from B
        for (auto &row : a[depth])
        {
            if(unique_ids.find(row.position) == unique_ids.end()){
                result[depth].push_back(row);
                unique_ids[row.position] = depth;
                // row.depth = depth;
            }else{
                row.depth = unique_ids[row.position];
                result[unique_ids[row.position]].push_back(row);
            }
        }
       
    }

    return result;
}

map<int, vector<ExcelRow>> sort_rows_by_period(map<int, vector<ExcelRow>> a)
{
    map<int, vector<ExcelRow>> tmp;
for(auto &pair : a) {
    std::sort(pair.second.begin(), pair.second.end());
    tmp[pair.first] = pair.second;
}
return tmp;
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
    Node* metal = new Node("A5_1", "metal");
    Node* guma = new Node("A5_2", "guma");
    Node* przerzutki_przednie_tylnie = new Node("A5_3", "przerzutki przednie i tylnie");
    Node* gabka = new Node("A5_4", "gabka");
    Node* hamulce = new Node("A5_5", "hamulce");
    Node* swiatla_odblaski = new Node("A5_6", "swiatla_odblaski");
    Node* lancuch = new Node("A5_7", "lancuch");
    Node* dzwonek = new Node("A5_8", "dzwonek");
    Node* plastik = new Node("A5_9", "plastik");
    Node* bagaznik = new Node("A5_10", "bagaznik");
    Node* rurki_do_kierownicy = new Node("A5_11", "rurki do kierownicy");
    Node* rurki_do_ramy = new Node("A5_12", "rurki do ramy");
    Node* resory = new Node("A5_13", "resory");
    Node* sruby_duze = new Node("A5_14", "sruby duze");
    Node* sruby_male = new Node("A5_15", "sruby male");
    Node* kierownica_gorska = new Node("A5_16", "kierownica gorska");
    Node* detki = new Node("A5_17", "detki");
    Node* opony = new Node("A5_18", "opony");
    Node* blotniki = new Node("A5_19", "blotniki");
    Node* pedaly = new Node("A5_20", "pedaly");
    Node* koszyk_na_bagaz = new Node("A5_21", "koszyk na bagaz");
    Node* uchwyt_na_bidon = new Node("A5_22", "uchwyt na bidon");
    Node* stopka_rowerowa = new Node("A5_23", "stopka rowerowa");
    Node* kola = new Node("A5_24", "kola");
    Node* piasta = new Node("A5_25", "piasta");
    Node* obrecze_kol = new Node("A5_26", "obrecze kol");
    Node* mostki_kierownicy = new Node("A5_27", "mostki kierownicy");
    Node* oslony_przerzutek = new Node("A5_28", "oslony przerzutek");
    Node* prowadnice_lancucha = new Node("A5_29", "prowadnice lancucha");
    Node* klamki_do_hamulcow = new Node("A5_30", "klamki do hamulcow");
    Node* tarcze_hamulcowe = new Node("A5_31", "tarcze hamulcowe");
    Node* ochronniki_lancuchowe = new Node("A5_32", "ochronniki lancuchowe");
    Node* korba_do_pedalow = new Node("A5_33", "korba do pedalow");
    Node* zaslepki_do_rurek_kierownicy = new Node("A5_34", "zaslepki do rurek kierownicy");
    Node* zebatki = new Node("A5_35", "zebatki");
    Node* szprychy = new Node("A5_36", "szprychy");
    Node* uchwyt_na_zapiecie_rowerowe = new Node("A5_37", "uchwyt na zapiecie rowerowe");
    Node* rama_gorska = new Node("A5_38", "rama gorska");
    Node* linki_do_hamowania_w_gumowym_oplocie = new Node("A5_39", "linki do hamowania w gumowym oplocie");
    Node* gripy_i_owijki_kierownicy = new Node("A5_40", "gripy i owijki kierownicy");
    Node* siodelko = new Node("A5_41", "siodelko");
    Node* siodelko_na_rurce = new Node("A5_42", "siodelko na rurce");
    Node* pioro_resorowe = new Node("A5_43", "pioro resorowe");
    Node* rama_miejska = new Node("A5_44", "rama miejska");
    Node* kierownica_miejska = new Node("A5_45", "kierownica miejska");
    Node* rowerowe_systemy_zabezpieczen = new Node("A5_46", "rowerowe systemy zabezpieczen");
    Node* stery_rowerowe = new Node("A5_47", "stery rowerowe");
    Node* mocowanie_do_smartfonow = new Node("A5_48", "mocowanie do smartfonow");
    Node* wsporniki_i_mocowania_akcesorii = new Node("A5_49", "wsporniki i mocowania akcesoriow");
    Node* zatyczki_do_detki = new Node("A5_50", "zatyczki do detki");
    Node* rower_gorski = new Node("A5_51", "rower gorski");
    Node* rower_miejski = new Node("A5_52", "rower miejski");

    // 7
    graph[rower_gorski].push_back({ sruby_duze , 1 }); // ACHTUNG!!!!!!!
    graph[rower_gorski].push_back({ rama_gorska , 1}); 
    graph[rower_gorski].push_back({ kierownica_gorska , 1 });
    graph[rower_gorski].push_back({ kola , 1}); // ACHTUNG!!!!!!!
    graph[rower_gorski].push_back({ siodelko_na_rurce , 1 });
    graph[rower_gorski].push_back({ hamulce , 1}); // ACHTUNG!!!!!!!
    graph[rower_gorski].push_back({ wsporniki_i_mocowania_akcesorii , 1 }); // ACHTUNG!!!!!!!
    // KONIEC GALEZI


    // sruby
    graph[sruby_duze].push_back({ metal , 1 }); // w pliku z docs bylo: 0,02
    //KONIEC GALEZI

    // rama
    graph[rama_gorska].push_back({ uchwyt_na_zapiecie_rowerowe , 1 });
    graph[rama_gorska].push_back({ uchwyt_na_bidon , 1 });
    graph[rama_gorska].push_back({ rurki_do_ramy , 1}); // ACHTUNG!!!!!!!
    graph[rama_gorska].push_back({ resory , 1 }); // ACHTUNG!!!!!!!
    graph[rama_gorska].push_back({ mocowanie_do_smartfonow , 1 });
    graph[rama_gorska].push_back({ pedaly , 1 });
    graph[rama_gorska].push_back({ korba_do_pedalow , 1 });
    graph[rama_gorska].push_back({ stopka_rowerowa , 1});
    graph[rama_gorska].push_back({ stery_rowerowe , 1});
    graph[rama_gorska].push_back({ lancuch , 1});
    graph[rama_gorska].push_back({ prowadnice_lancucha , 1}); // ACHTUNG!!!!!!!
    graph[rama_gorska].push_back({ ochronniki_lancuchowe , 1 });



    // kierownica
    graph[kierownica_gorska].push_back({ mostki_kierownicy , 1 });
    graph[kierownica_gorska].push_back({ sruby_male , 1 }); // ACHTUNG!!!!!!!
    graph[kierownica_gorska].push_back({ oslony_przerzutek , 1});
    graph[kierownica_gorska].push_back({ przerzutki_przednie_tylnie , 1 }); // ACHTUNG!!!!!!!
    graph[kierownica_gorska].push_back({ rurki_do_kierownicy , 1 }); // ACHTUNG!!!!!!!
    graph[kierownica_gorska].push_back({ swiatla_odblaski , 1 }); //swiatla
    graph[kierownica_gorska].push_back({ gripy_i_owijki_kierownicy , 1 }); // ACHTUNG!!!!!!!
    graph[kierownica_gorska].push_back({ klamki_do_hamulcow , 1 }); // ACHTUNG!!!!!!!
    graph[kierownica_gorska].push_back({ dzwonek , 1 });


    // kola
    graph[kola].push_back({ opony, 1});
    graph[kola].push_back({ detki, 1});
    graph[kola].push_back({szprychy, 1}); // ACHTUNG!!!!!!!
    graph[kola].push_back({ piasta, 1});
    graph[kola].push_back({ blotniki, 1});
    graph[kola].push_back({ swiatla_odblaski, 1}); // ACHTUNG!!!!!!! odblaski
    graph[kola].push_back({ zatyczki_do_detki, 1}); // ACHTUNG!!!!!!!

    // siodelko
    graph[siodelko_na_rurce].push_back({ siodelko,1 });
    graph[siodelko_na_rurce].push_back({ rurki_do_kierownicy , 1 });

    //hamulce
    /* KONIEC */

    //wsporniki
    graph[siodelko_na_rurce].push_back({ wsporniki_i_mocowania_akcesorii , 1 }); // w pliku 0.05


    /*--------------------------------------------------------------------------- DZIECI RAMY GORSKIEJ ------------------------------*/


    // uchwyt_na_zapiecie_rowerowe
    graph[uchwyt_na_zapiecie_rowerowe].push_back({ metal , 1 }); // w pliku 0.05
   //KONIEC GALEZI

    // uchwyt_na_bidon
    graph[uchwyt_na_bidon].push_back({ metal , 1 }); // w pliku 0.05
    //KONIEC GALEZI

    // rurki_do_ramy
    graph[rurki_do_ramy].push_back({ metal , 1 }); // w pliku 1.2
    //KONIEC GALEZI

    // resory
    graph[resory].push_back({ guma , 1 }); // w pliku 0,01
    graph[resory].push_back({ metal , 1 }); // w pliku 0.02
    graph[resory].push_back({ pioro_resorowe , 1 });
    //KONIEC GALEZI

    // mocowanie_do_smartfonow
    graph[mocowanie_do_smartfonow].push_back({ metal , 1 }); // w pliku 0.03
    //KONIEC GALEZI

    // pedaly :)
    graph[pedaly].push_back({ metal , 1 }); // w pliku 0.04
    graph[pedaly].push_back({ sruby_male , 1 }); // ACHTUNG!!!
    graph[pedaly].push_back({ swiatla_odblaski , 1 }); //  odblaski
    graph[pedaly].push_back({ zebatki , 1 }); // ACHTUNG!!! 
    //KONIEC


    // korba_do_pedalow
    graph[korba_do_pedalow].push_back({ metal, 1 }); // w pliku 0.07
    //KONIEC


    // stopka_rowerowa
    graph[stopka_rowerowa].push_back({ metal , 1 }); // w pliku 0.07
    //KONIEC

    // stery_rowerowe
    graph[stery_rowerowe].push_back({ metal , 1 }); // w pliku 0.15
    //KONIEC

    // lancuch - KONIEC

    // prowadnice_lancucha
    graph[prowadnice_lancucha].push_back({ metal , 1 });
    //KONIEC

    // ochronniki_lancuchowe
    graph[ochronniki_lancuchowe].push_back({ metal , 1 });
    //KONIEC

    

    /*--------------------------------------------------------------------------- DZIECI KIEROWNICY GORSKIEJ ------------------------------*/

    //mostki

    graph[mostki_kierownicy].push_back({ sruby_duze , 1 }); // ACHTUNG!!! 
    graph[mostki_kierownicy].push_back({ rurki_do_kierownicy , 1 }); // ACHTUNG!!! 
   
    // 
    //sruby
    graph[sruby_male].push_back({ metal , 1 }); // w pliku 0.01

    /* -------------------------------------------------------------------------------------------BRAK------------------------------------------------------------------------------- */
    //oslony_przerzutek
    graph[oslony_przerzutek].push_back({ plastik , 1 });
    
    //przerzutki - NIC
 
    //rurki
    graph[rurki_do_kierownicy].push_back({ metal , 1 }); // w plku 0.4
    graph[rurki_do_kierownicy].push_back({ zaslepki_do_rurek_kierownicy , 1}); // ACHTUNG!!! 
   
    //swiatla i odblaski - NIC
    
    //gripy i owijki 
    graph[gripy_i_owijki_kierownicy].push_back({ metal , 1 }); // w pliku 0.02
    graph[gripy_i_owijki_kierownicy].push_back({ guma , 1 }); // w pliku 0.02

    //klamki
    graph[klamki_do_hamulcow].push_back({ metal , 1 }); // w pliku 0.03
    graph[klamki_do_hamulcow].push_back({ linki_do_hamowania_w_gumowym_oplocie , 1 });

    //dzwonek - NIC


    /*--------------------------------------------------------------------------- DZIECI  KOL ------------------------------*/

    /* opony */
    graph[opony].push_back({ guma , 1 }); // w pliku 0.2
    /* detki */
    graph[detki].push_back({ guma , 1 }); // w pliku 0.1


    /* -------------------------------------------------------------------------------------------BRAK TU I NIZEJ NA RAZIE------------------------------------------------------------------------------- */
    /* obrecze */
    graph[obrecze_kol].push_back({ metal , 1});

    /* szprychy */
    graph[szprychy].push_back({ metal , 1 });

    /* piasta */
    graph[piasta].push_back({ tarcze_hamulcowe , 1 });
    graph[piasta].push_back({ metal , 1});

    /* blotniki */
    graph[blotniki].push_back({ metal , 1 });

    /* swiatla i odblaski - NIC */

    /* zatyczki */
    graph[zatyczki_do_detki].push_back({ plastik , 1 });


    /*--------------------------------------------------------------------------- DZIECI DZIECI siodelko na rurce  ------------------------------*/

    graph[siodelko].push_back({ guma, 1});
    graph[siodelko].push_back({ gabka , 1 });

     //ramagorska->pedaly->sruby_male
    graph[sruby_male].push_back({ metal , 1 });

    //ramagorska->pedaly->zebatki
    graph[zebatki].push_back({ metal , 1 });

    //kierownica->mostek->sruby duze
    graph[sruby_duze].push_back({ metal , 1 });

    //kierownica->mostek->rurki do kierownicy
    graph[rurki_do_kierownicy].push_back({ metal , 1 });

    //kierownica->rurki do kierownicy->zaslepki
    graph[zaslepki_do_rurek_kierownicy].push_back({ guma , 1 });


    //kierownica->klamki->linki
    graph[linki_do_hamowania_w_gumowym_oplocie].push_back({ guma , 1 }); // w pliku 0.05
    graph[linki_do_hamowania_w_gumowym_oplocie].push_back({ metal , 1 }); // w pliku 0.05

    //kola->piasta->tarcza hamulcowa
    graph[tarcze_hamulcowe].push_back({ metal , 1});

    /* MIEJSKI */

    // TAKIE SAMO JAK DLA GORSKIEGO
    graph[rower_miejski].push_back({ sruby_duze , 1 });
    graph[rower_miejski].push_back({ kola , 1 });
    graph[rower_miejski].push_back({ siodelko_na_rurce , 1 });
    graph[rower_miejski].push_back({ hamulce , 1 });
    graph[rower_miejski].push_back({ wsporniki_i_mocowania_akcesorii , 1 });

    // INNE - koszyk i bagaznik, ktorego nie ma w gorskim
    graph[rower_miejski].push_back({ koszyk_na_bagaz , 1 });
    graph[rower_miejski].push_back({ bagaznik , 1 });
    //KONIECCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC




    // INNE NIZ DLA GORSKIEGO -- NOWE!!!!!
    graph[rower_miejski].push_back({ kierownica_miejska , 1 });
    graph[rower_miejski].push_back({ rowerowe_systemy_zabezpieczen , 1 });
    graph[rower_miejski].push_back({ rama_miejska , 1 });



    //kierownica miejska
    graph[kierownica_miejska].push_back({ mostki_kierownicy , 1 });
    graph[kierownica_miejska].push_back({ sruby_male , 1 });
    graph[kierownica_miejska].push_back({ oslony_przerzutek , 1 });
    graph[kierownica_miejska].push_back({ przerzutki_przednie_tylnie , 1 });
    graph[kierownica_miejska].push_back({ rurki_do_kierownicy , 1 });
    graph[kierownica_miejska].push_back({ swiatla_odblaski , 1 });
    graph[kierownica_miejska].push_back({ gripy_i_owijki_kierownicy , 1 });
    graph[kierownica_miejska].push_back({ klamki_do_hamulcow , 1 });
    graph[kierownica_miejska].push_back({ dzwonek , 1 });


    //rowerowe systemy zabezpieczen
    graph[rowerowe_systemy_zabezpieczen].push_back({ metal , 1 });


    //rama
    graph[rama_miejska].push_back({ uchwyt_na_zapiecie_rowerowe , 1 });
    graph[rama_miejska].push_back({ uchwyt_na_bidon , 1 });
    graph[rama_miejska].push_back({ rurki_do_ramy , 1 });
    graph[rama_miejska].push_back({ resory , 1 });
    graph[rama_miejska].push_back({ mocowanie_do_smartfonow , 1 });
    graph[rama_miejska].push_back({ pedaly , 1 });
    graph[rama_miejska].push_back({ korba_do_pedalow , 1 });
    graph[rama_miejska].push_back({ stopka_rowerowa , 1 });
    graph[rama_miejska].push_back({ stery_rowerowe , 1});
    graph[rama_miejska].push_back({ lancuch , 1 });
    graph[rama_miejska].push_back({ prowadnice_lancucha , 1 });
    graph[rama_miejska].push_back({ ochronniki_lancuchowe , 1 });


    // koszyk - metal i gabka
    graph[koszyk_na_bagaz].push_back({ metal , 1 });
    graph[koszyk_na_bagaz].push_back({ gabka , 1 });


    // bagaznik - metal i odlbaski
    graph[bagaznik].push_back({ metal , 1 });
    graph[bagaznik].push_back({ swiatla_odblaski , 1 });




    dfs(rower_gorski, 0); 
    
    create_excel(excel_Z1, 20, 2); //okres 20 i 2 sztuki
        // print_excel(excel_Z1);
    create_excel(excel_Z2, 22, 3); //okres 22 i 3 sztuki
        // print_excel(excel_Z2);
    clear_wicf(excel_Z2);
    clear_dfs(rower_gorski);
    set_stock_on_level_1(excel_Z1, "A5_14", 2); //EDYTOWAC!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    print_excel(excel_Z1);
    calculate_excel_demand(excel_Z1);

    save_to_txt(excel_Z1, "z1.txt");
    save_to_txt(excel_Z2, "z2.txt");

    // create second excel
    graph_of_parents.clear();
    dfs(rower_miejski, 0);
    create_excel(excel_Z3, 20, 4); // drugi wyrob okres 20 i 4 sztuki
    clear_wicf(excel_Z3);
    save_to_txt(excel_Z3, "z3.txt");
    // print_excel(excel_Z3);

    // calculate_excel_demand()

    tmp = merge_excels_implementation(excel_Z1, excel_Z3);
    save_to_txt(tmp, "tmp.txt");
    cout << "i m here \n\n\n asdasd" << endl;
    result_b = merge_excels_implementation_v2(tmp, excel_Z2);
    
    result_final = merge_for_solution( result_b);

    // print_excel(result_final);
    result_sorted = sort_rows_by_period(result_final);
    save_to_txt(result_final, "result_final.txt");
    save_to_txt(result_sorted, "result_sorted.txt");
    print_excel( result_b);
    save_to_txt( result_b, "result_b.txt");

    cout << "program finished successfully" << endl;

    cout << "HEJA BVB";

    return 0;
}
